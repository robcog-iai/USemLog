// Fill out your copyright notice in the Description page of Project Settings.


#include "SLKnowrobConnectHandler.h"
#include "WebSocketsModule.h"
#include "HAL/RunnableThread.h"

int32 FSLKnowrobConnectHandler::ThreadInstanceIdx = 0;

bool FSLKnowrobConnectHandler::FPLBridgeHandlerRunnable::Init()
{
	TArray<FString> Protocols;
	Protocols.Add(Handler->Protocol);
	Handler->Client = FWebSocketsModule::Get().CreateWebSocket(TEXT("ws://") + Handler->Host + TEXT(":") + FString::FromInt(Handler->Port), Protocols);
	Handler->Client->OnConnected().AddRaw(this->Handler, &FSLKnowrobConnectHandler::HandleWebSocketConnected);
	Handler->Client->OnConnectionError().AddRaw(this->Handler, &FSLKnowrobConnectHandler::HandleWebSocketConnectionError);
	Handler->Client->OnClosed().AddRaw(this->Handler, &FSLKnowrobConnectHandler::HandleWebSocketConnectionClosed);
	Handler->Client->OnRawMessage().AddRaw(this->Handler, &FSLKnowrobConnectHandler::HandleWebSocketData);
	Handler->Client->Connect();

	return true;
}

uint32 FSLKnowrobConnectHandler::FPLBridgeHandlerRunnable::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.01);

	// Counter for re-trying an initially unsuccessful connection
	uint32 ConnectionTrialCounter = 0;

	while (StopCounter.GetValue() == 0)
	{
		if (!Handler->Client->IsConnected())
		{
			if (++ConnectionTrialCounter > 100)
			{
				Stop();

				UE_LOG(LogTemp, Error, TEXT("[%s] Could not connect to the server (IP %s, port %d)!"),
					*FString(__FUNCTION__),
					*(Handler->GetHost()),
					Handler->GetPort());

				continue;
			}
		}
		else
		{
			// runing and connected
		}

		// Sleep the main loop
		FPlatformProcess::Sleep(Handler->GetClientInterval());
	}
	return 0;
}

void FSLKnowrobConnectHandler::FPLBridgeHandlerRunnable::Stop()
{
	StopCounter.Increment();
}

// Exits the runnable object
void FSLKnowrobConnectHandler::FPLBridgeHandlerRunnable::Exit()
{
	UE_LOG(LogTemp, Warning, TEXT("thread exit"));
}


void FSLKnowrobConnectHandler::HandleWebSocketConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Connected"));
	SetClientConnected(true);
}

void FSLKnowrobConnectHandler::HandleWebSocketData(const void* Data, SIZE_T Length, SIZE_T BytesRemaining)
{
	if (BytesRemaining == 0 && ReceiveBuffer.Num() == 0)
	{
		// Skip the temporary buffer when the entire frame arrives in a single message. (common case)
		HandleIncomingMessage((const uint8*)Data, Length);
	}
	else
	{
		ReceiveBuffer.Append((const uint8*)Data, Length);
		if (BytesRemaining == 0)
		{
			HandleIncomingMessage(ReceiveBuffer.GetData(), ReceiveBuffer.Num());
			ReceiveBuffer.Empty();
		}
	}
}

void FSLKnowrobConnectHandler::HandleIncomingMessage(const uint8* Data, SIZE_T Length)
{
	std::string ProtoStr(Data, Data + Length);
	QueueTask.Enqueue(ProtoStr);
	//SendMsg();
}

void FSLKnowrobConnectHandler::Connect()
{
	Runnable = new FPLBridgeHandlerRunnable(this);
	Thread = FRunnableThread::Create(Runnable, *FString::Printf(TEXT("PLBridgeHandlerRunnable_%d"), ThreadInstanceIdx++), 0, TPri_Normal);

}

// Stop runnable / thread / client
void FSLKnowrobConnectHandler::ThreadCleanup()
{
	// Kill the thread and the Runnable
	if (Runnable)
	{
		Runnable->Stop();
	}

	if (Thread)
	{
		Thread->WaitForCompletion();
		Thread->Kill(true);
		//Thread->Kill(false);
		delete Thread;
		Thread = NULL;
	}

	if (Runnable)
	{
		Runnable->Exit();
		delete Runnable;
		Runnable = NULL;
	}

	if (Client.IsValid())
	{
		Client->Close();
		Client = NULL;
	}
}

void FSLKnowrobConnectHandler::Disconnect()
{
	if (Client->IsConnected())
		ThreadCleanup();
}

void FSLKnowrobConnectHandler::SendMsg()
{
	FString StrToSend = TEXT("Message recieved!");
	FTCHARToUTF8 UTStr(*StrToSend);
	TArray<uint8> BinaryMsg;
	AppendArray(BinaryMsg, (uint8*)UTStr.Get(), UTStr.Length(), true);
	BinaryMsg.Add('\0');
	Client->Send(BinaryMsg.GetData(), BinaryMsg.Num(), false);
}

void FSLKnowrobConnectHandler::AppendArray(TArray<uint8>& Out, uint8* In, SIZE_T Length, bool bShouldEscape)
{
	if (bShouldEscape)
	{
		for (SIZE_T I = 0; I < Length; I++)
		{
			if (In[I] == ':' || In[I] == '\\' || In[I] == '\n' || In[I] == '\r')
			{
				Out.Add('\\');
			}
			Out.Add(In[I]);
		}
	}
	else
	{
		Out.Append(In, Length);
	}
}

void FSLKnowrobConnectHandler::HandleWebSocketConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Error, TEXT("[%s] Error in Websocket."), *FString(__FUNCTION__));
}

void FSLKnowrobConnectHandler::HandleWebSocketConnectionClosed(int32 Status, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Error, TEXT("Websocket close"));
}