// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include <string>
/**
 * 
 */
class USEMLOG_API FSLKRWSClient
{
private:
	// Thread to handle socket message
	class FPLBridgeHandlerRunnable : public FRunnable
	{
	public:
		FPLBridgeHandlerRunnable(
			FSLKRWSClient* KrowrobConnHandler) :
			StopCounter(0),
			Handler(KrowrobConnHandler)
		{
		}

		// Create connection, bind functions to WebSocket Client, and Connect.
		virtual bool Init() override;

		// Process subscribed messages using "Send"
		virtual uint32 Run() override;

		// Set the stop counter and disconnect
		virtual void Stop() override;

		// Exits the runnable object. Called in the context of the aggregating thread to perform any cleanup.
		virtual void Exit() override;
	
	private:
		// Increase the StopCounter to stop the Runnable thread.
		FThreadSafeCounter StopCounter;
		FSLKRWSClient* Handler;
	};

	FString Host;
	int32 Port;
	FString Protocol;

	FThreadSafeBool bIsClientConnected;
	float ClientInterval;

	TSharedPtr<IWebSocket> Client;

	FPLBridgeHandlerRunnable* Runnable;
	FRunnableThread* Thread;

	/** Index used to disambiguate thread instances for stats reasons */
	static int32 ThreadInstanceIdx;

	// Buffer for receiving message
	TArray<uint8> ReceiveBuffer;

	// WebSocket event handlers
	void HandleWebSocketConnected();
	void HandleWebSocketData(const void* Data, SIZE_T Length, SIZE_T BytesRemaining);
	void HandleIncomingMessage(const uint8* Data, SIZE_T Length);
	void HandleWebSocketConnectionError(const FString& Error);
	void HandleWebSocketConnectionClosed(int32 Status, const FString& Reason, bool bWasClean);

	// Friendship declaration
	friend class FPLBridgeHandlerRunnable;

public:
	FSLKRWSClient(const FString& InHost, int32 InPort, const FString& InProtocol) :
		Host(InHost), Port(InPort),
		Protocol(InProtocol),
		bIsClientConnected(false),
		ClientInterval(0.1)
	{
	}

	~FSLKRWSClient()
	{
		// Thread and runnable are killed before deconstr
		ThreadCleanup();
	}

	float GetClientInterval() const
	{
		return ClientInterval;
	}

	void SetClientInterval(float NewInterval)
	{
		ClientInterval = NewInterval;
	}

	bool IsClientConnected() const
	{
		return Client->IsConnected();
	}

	void SetClientConnected(bool bVal)
	{
		bIsClientConnected.AtomicSet(bVal);
	}

	FString GetHost() const
	{
		return Host;
	}

	int32 GetPort() const
	{
		return Port;
	}

	// Create runnable instance and run the thread
	void Connect();

	// Unsubscribe / Unadvertise all messages, stop the thread
	void Disconnect();

	// Stop runnable / thread / client
	void ThreadCleanup();

	// Send message to server
	void SendMsg(const FString& StrToSend);

	// Transform stirng to binary
	void AppendArray(TArray<uint8>& Out, uint8* In, SIZE_T Length, bool bShouldEscape);

	TQueue< std::string > QueueTask;
};
