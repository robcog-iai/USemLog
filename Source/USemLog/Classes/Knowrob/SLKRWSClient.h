// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Containers/Queue.h"
#include "SLKRResponseStruct.h"
#include <string>

// Triggered when a new messages is added to the queue
DECLARE_DELEGATE(FSLKRWSClientNewMsg);

// Triggered when connected / disconnected
DECLARE_DELEGATE_OneParam(FSLKRWSClientConnection, bool /*bConnected*/);

/**
 * Knowrob websocket client communication
 */
class USEMLOG_API FSLKRWSClient
{
public:
	// Default ctor
	FSLKRWSClient();

	// Dtor
	~FSLKRWSClient();

	// Set websocket connection parameters
	void Init(const FString& InHost, int32 InPort, const FString& InProtocol);

	// Initiate a client connection to the server and bind event handlers
	void Connect();
	
	// Check if the client is connected to the server
	bool IsConnected() const { return WebSocket.IsValid() ? WebSocket->IsConnected() : false; }

	// Disconnect from the server.
	void Disconnect();

	// Clear the webscosket 
	void Clear();

	// Send message via websocket
	void SendResponse(const FSLKRResponse& Response);

protected:
	/* IWebSocket delegate handlers */
	// Called on connection
	void HandleWebSocketConnected();

	// Called on connection error
	void HandleWebSocketConnectionError(const FString& Error);

	// Called on connection closed
	void HandleWebSocketConnectionClosed(int32 Status, const FString& Reason, bool bWasClean);

	// Called on new data (can be partial)
	void HandleWebSocketData(const void* Data, SIZE_T Length, SIZE_T BytesRemaining);

	// Called when the full data has been received
	void HandleWebSocketFullData(const uint8* Data, SIZE_T Length);

public:
	// Triggered when a new processed message is added to the queue
	FSLKRWSClientNewMsg OnNewProcessedMsg;

	// Triggered when connected / disconnected
	FSLKRWSClientConnection OnConnection;

	// Received message as binary string
	TQueue<std::string> MessageQueue;

private:
	// Websocket interface
	TSharedPtr<IWebSocket> WebSocket;

	// Received message binary
	TArray<uint8> ReceiveBuffer;
};