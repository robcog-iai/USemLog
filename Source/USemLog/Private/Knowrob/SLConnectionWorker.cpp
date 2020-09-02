//// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
//
//#include "SLKRWSClient.h"
//#include "WebSocketsModule.h"
//#include "HAL/RunnableThread.h"
//
//int32 FSLKRWSClient::ThreadInstanceIdx = 0;
//
///* Begin FRunnable Interface */
//// Initializes the runnable object.
//bool FSLKRWSClient::FWSCommRunnable::Init()
//{
//	TArray<FString> Protocols;
//	Protocols.Add(KRWSClient->Protocol);
//
//	// Create websocket connection
//	KRWSClient->Client = FWebSocketsModule::Get().CreateWebSocket(
//		TEXT("ws://") + KRWSClient->Host + TEXT(":") + FString::FromInt(KRWSClient->Port), Protocols);
//
//	// Bind websocket events
//	KRWSClient->Client->OnConnected().AddRaw(KRWSClient, &FSLKRWSClient::OnConnected);
//	KRWSClient->Client->OnConnectionError().AddRaw(KRWSClient, &FSLKRWSClient::OnConnectionError);
//	KRWSClient->Client->OnClosed().AddRaw(KRWSClient, &FSLKRWSClient::OnClosed);
//	KRWSClient->Client->OnRawMessage().AddRaw(KRWSClient, &FSLKRWSClient::HandleWebSocketData);
//	KRWSClient->Client->Connect();
//
//	return true;
//}
//
//uint32 FSLKRWSClient::FWSCommRunnable::Run()
//{
//	//Initial wait before starting
//	FPlatformProcess::Sleep(0.01);
//
//	// Counter for re-trying an initially unsuccessful connection
//	uint32 ConnectionTrialCounter = 0;
//
//	while (StopCounter.GetValue() == 0)
//	{
//		if (!KRWSClient->Client->IsConnected())
//		{
//			if (++ConnectionTrialCounter > 100)
//			{
//				Stop();
//
//				UE_LOG(LogTemp, Error, TEXT("[%s] Could not connect to the server (IP %s, port %d)!"),
//					*FString(__FUNCTION__),
//					*(Handler->GetHost()),
//					Handler->GetPort());
//
//				continue;
//			}
//		}
//		else
//		{
//			// runing and connected
//		}
//
//		// Sleep the main loop
//		FPlatformProcess::Sleep(KRWSClient->GetClientInterval());
//	}
//	return 0;
//}
//
//void FSLKRWSClient::FWSCommRunnable::Stop()
//{
//	StopCounter.Increment();
//}
//
//// Exits the runnable object
//void FSLKRWSClient::FWSCommRunnable::Exit()
//{
//	UE_LOG(LogTemp, Warning, TEXT("thread exit"));
//}
///* End FRunnable Interface */
//
///**
// * Knowrob websocket client communication
// */
//
//// Ctor
//FSLKRWSClient::FSLKRWSClient()
//{
//	bConnected = false;
//	ThreadSleepInterval = 0.1f;
//}
//
//// Dtor
//FSLKRWSClient::~FSLKRWSClient()
//{
//	// Thread and runnable are killed before deconstr
//	ThreadCleanup();
//}
//
//void FSLKRWSClient::OnConnected()
//{
//	UE_LOG(LogTemp, Warning, TEXT("Connected"));
//	SetClientConnected(true);
//}
//
//void FSLKRWSClient::HandleWebSocketData(const void* Data, SIZE_T Length, SIZE_T BytesRemaining)
//{
//	if (BytesRemaining == 0 && ReceiveBuffer.Num() == 0)
//	{
//		// Skip the temporary buffer when the entire frame arrives in a single message. (common case)
//		HandleIncomingMessage((const uint8*)Data, Length);
//	}
//	else
//	{
//		ReceiveBuffer.Append((const uint8*)Data, Length);
//		if (BytesRemaining == 0)
//		{
//			HandleIncomingMessage(ReceiveBuffer.GetData(), ReceiveBuffer.Num());
//			ReceiveBuffer.Empty();
//		}
//	}
//}
//
//void FSLKRWSClient::HandleIncomingMessage(const uint8* Data, SIZE_T Length)
//{
//	std::string ProtoStr(Data, Data + Length);
//	QueueTask.Enqueue(ProtoStr);
//	UE_LOG(LogTemp, Warning, TEXT("receive the protobuf"));
//	//SendMsg(TEXT("Message recieved!"));
//}
//
//void FSLKRWSClient::Connect()
//{
//	Runnable = new FWSCommRunnable(this);
//	Thread = FRunnableThread::Create(Runnable, *FString::Printf(TEXT("PLBridgeHandlerRunnable_%d"), ThreadInstanceIdx++), 0, TPri_Normal);
//
//}
//
//// Stop runnable / thread / client
//void FSLKRWSClient::ThreadCleanup()
//{
//	// Kill the thread and the Runnable
//	if (Runnable)
//	{
//		Runnable->Stop();
//	}
//
//	if (Thread)
//	{
//		Thread->WaitForCompletion();
//		Thread->Kill(true);
//		//Thread->Kill(false);
//		delete Thread;
//		Thread = NULL;
//	}
//
//	if (Runnable)
//	{
//		Runnable->Exit();
//		delete Runnable;
//		Runnable = NULL;
//	}
//
//	if (Client.IsValid())
//	{
//		Client->Close();
//		Client = NULL;
//	}
//}
//
//void FSLKRWSClient::Disconnect()
//{
//	if (Client->IsConnected())
//		ThreadCleanup();
//}
//
//void FSLKRWSClient::SendMsg(const FString& StrToSend)
//{
//	FTCHARToUTF8 UTStr(*StrToSend);
//	TArray<uint8> BinaryMsg;
//	AppendArray(BinaryMsg, (uint8*)UTStr.Get(), UTStr.Length(), true);
//	BinaryMsg.Add('\0');
//	Client->Send(BinaryMsg.GetData(), BinaryMsg.Num(), false);
//}
//
//void FSLKRWSClient::AppendArray(TArray<uint8>& Out, uint8* In, SIZE_T Length, bool bShouldEscape)
//{
//	if (bShouldEscape)
//	{
//		for (SIZE_T I = 0; I < Length; I++)
//		{
//			if (In[I] == ':' || In[I] == '\\' || In[I] == '\n' || In[I] == '\r')
//			{
//				Out.Add('\\');
//			}
//			Out.Add(In[I]);
//		}
//	}
//	else
//	{
//		Out.Append(In, Length);
//	}
//}
//
//void FSLKRWSClient::OnConnectionError(const FString& Error)
//{
//	UE_LOG(LogTemp, Error, TEXT("[%s] Error in Websocket."), *FString(__FUNCTION__));
//}
//
//void FSLKRWSClient::OnClosed(int32 Status, const FString& Reason, bool bWasClean)
//{
//	UE_LOG(LogTemp, Error, TEXT("Websocket close"));
//}