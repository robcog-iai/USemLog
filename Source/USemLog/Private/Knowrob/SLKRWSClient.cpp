// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKRWSClient.h"
#include "WebSocketsModule.h"
#if SL_WITH_PROTO
#include "Knowrob/Proto/SLProtoMsgType.h"
#endif // SL_WITH_PROTO	


// Ctor
FSLKRWSClient::FSLKRWSClient()
{
}

// Dtor
FSLKRWSClient::~FSLKRWSClient()
{
}

// Set websocket conection parameters
void FSLKRWSClient::Init(const FString& InHost, int32 InPort, const FString& InProtocol)
{
	// TODO, why is this needed
	// Make sure the webscokets module is loaded
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	// Create the url and protocol entries
	TArray<FString> Protocols{ InProtocol };
	const FString Url = TEXT("ws://") + InHost + TEXT(":") + FString::FromInt(InPort);

	// Clear any previous connection
	if (WebSocket.IsValid())
	{
		if (WebSocket->IsConnected())
		{
			Disconnect();
		}
		Clear();
		WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url, Protocols);
	}
	else
	{
		WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url, Protocols);
	}

	// Bind WS delegates
	WebSocket->OnConnected().AddRaw(this, &FSLKRWSClient::HandleWebSocketConnected);
	WebSocket->OnConnectionError().AddRaw(this, &FSLKRWSClient::HandleWebSocketConnectionError);
	WebSocket->OnClosed().AddRaw(this, &FSLKRWSClient::HandleWebSocketConnectionClosed);
	WebSocket->OnRawMessage().AddRaw(this, &FSLKRWSClient::HandleWebSocketData);
}

// Start connection
void FSLKRWSClient::Connect()
{
	if (WebSocket.IsValid())
	{
		if (!WebSocket->IsConnected())
		{
			WebSocket->Connect();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d WebSocket is already connected.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d WebSocket is not valid, call init first.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Disconnect from the server.
void FSLKRWSClient::Disconnect()
{
	if (WebSocket.IsValid())
	{
		// Close connection
		WebSocket->Close();
	}

	// Clear any remaining messages
	ReceiveBuffer.Empty();
	MessageQueue.Empty();
}

// Clear the webscosket 
void FSLKRWSClient::Clear()
{
	if (WebSocket.IsValid())
	{
		// Close connection
		WebSocket->Close();

		// Unbind delgates
		WebSocket->OnConnected().RemoveAll(this);
		WebSocket->OnConnectionError().RemoveAll(this);
		WebSocket->OnClosed().RemoveAll(this);
		WebSocket->OnRawMessage().RemoveAll(this);

		// Reset shared pointer
		WebSocket.Reset();
	}

	// Clear any remaining messages
	ReceiveBuffer.Empty();
	MessageQueue.Empty();
}

// Called on connection
void FSLKRWSClient::HandleWebSocketConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f KR websocket client is connected.. "),
		*FString(__FUNCTION__), __LINE__, FPlatformTime::Seconds());

	// Trigger delegate
	OnConnection.ExecuteIfBound(true);
}

// Called on connection error
void FSLKRWSClient::HandleWebSocketConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f KR websocket client connection error: %s"),
		*FString(__FUNCTION__), __LINE__, FPlatformTime::Seconds(), *Error);

	// Trigger delegate
	OnConnection.ExecuteIfBound(false);
}

// Called on connection closed
void FSLKRWSClient::HandleWebSocketConnectionClosed(int32 Status, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f KR websocket client connection closed: Status=%d; Reason=%s; bWasClean=%d;"),
		*FString(__FUNCTION__), __LINE__, FPlatformTime::Seconds(), Status, *Reason, bWasClean);

	// Trigger delegate
	OnConnection.ExecuteIfBound(false);
}

// Called on new data (can be partial)
void FSLKRWSClient::HandleWebSocketData(const void* Data, SIZE_T Length, SIZE_T BytesRemaining)
{
	if (BytesRemaining == 0 && ReceiveBuffer.Num() == 0)
	{
		// Skip the temporary buffer when the entire frame arrives in a single message. (common case)
		HandleWebSocketFullData((const uint8*)Data, Length);
	}
	else
	{
		ReceiveBuffer.Append((const uint8*)Data, Length);
		if (BytesRemaining == 0)
		{
			HandleWebSocketFullData(ReceiveBuffer.GetData(), ReceiveBuffer.Num());
			ReceiveBuffer.Empty();
		}
	}
}

// Called when the full data has been received
void FSLKRWSClient::HandleWebSocketFullData(const uint8* Data, SIZE_T Length)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f KR websocket client new message enqueued.."),
		*FString(__FUNCTION__), __LINE__, FPlatformTime::Seconds());

	MessageQueue.Enqueue(std::string(Data, Data + Length));

	// Trigger delegate
	OnNewProcessedMsg.ExecuteIfBound();
}

// Send message via websocket
void FSLKRWSClient::SendResponse(const FSLKRResponse& Response)
{
#if SL_WITH_PROTO
	if (Response.Type == ResponseType::TEXT)
	{
		std::string TextStr(TCHAR_TO_UTF8(*Response.Text));
		sl_pb::KRAmevaResponse AmevaResponse;
		AmevaResponse.set_type(sl_pb::KRAmevaResponse::Text);
		AmevaResponse.set_text(TextStr);
		std::string ProtoStr = AmevaResponse.SerializeAsString();
		WebSocket->Send(ProtoStr.data(), ProtoStr.size(), true);
	}
	else if (Response.Type == ResponseType::FILE)
	{
		// Notify knowrob to create a file
		std::string FLNameStr(TCHAR_TO_UTF8(*Response.FileName));
		sl_pb::KRAmevaResponse CreationResponse;
		CreationResponse.set_type(sl_pb::KRAmevaResponse::FileCreation);
		CreationResponse.set_filename(FLNameStr);
		std::string ProtoStr = CreationResponse.SerializeAsString();
		WebSocket->Send(ProtoStr.data(), ProtoStr.size(), true);

		// Slice the file data into frames and send them to knowrob
		const int DefaultFrameSize = 64;
		for (int Offset = 0; Offset < Response.FileData.Num(); Offset += DefaultFrameSize)
		{
			sl_pb::KRAmevaResponse DataResponse;
			DataResponse.set_type(sl_pb::KRAmevaResponse::FileData);
			int32 FrameSize = Response.FileData.Num() - Offset < DefaultFrameSize ? Response.FileData.Num() - Offset : DefaultFrameSize;
			const uint8* FileData = Response.FileData.GetData();
			uint8 DataFrame[DefaultFrameSize];
			memcpy(DataFrame, FileData + Offset, FrameSize);
			DataResponse.set_datalength(FrameSize);
			DataResponse.set_filedata((char*)DataFrame);
			ProtoStr = DataResponse.SerializeAsString();
			WebSocket->Send(ProtoStr.data(), ProtoStr.size(), true);
		}

		// Notify that all the data is sent
		sl_pb::KRAmevaResponse FinishResponse;
		FinishResponse.set_type(sl_pb::KRAmevaResponse::FileFinish);
		FinishResponse.set_filename(FLNameStr);
		ProtoStr = FinishResponse.SerializeAsString();
		WebSocket->Send(ProtoStr.data(), ProtoStr.size(), true);
	}
#endif // SL_WITH_PROTO	
}