// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Knowrob/SLKRWSClient.h"
#include "SLKnowrobManager.generated.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLIndividualManager;

/**
*
**/
UCLASS()
class USEMLOG_API ASLKnowrobManager : public AInfo
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ASLKnowrobManager();

	// Dtor
	~ASLKnowrobManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set up any required references and connect to server
	void Init();

	// Start processing any incomming messages
	void Start();

	// Stop processing the messages, and disconnect from server
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	/* Knowrob websocket client delegate triggers */
	// Called when a new message is received from knowrob
	void OnKRMsg();

	// Called when connected or disconnecetd with knowrob
	void OnKRConnection(bool bConnectionValue);

private:
	// Get the mongo query manager from the world (or spawn a new one)
	bool SetMongoQueryManager();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

protected:
	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done logging
	uint8 bIsFinished : 1;

private:
	// Knowrob server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	FString KRServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	int32 KRServerPort = 8080;
	
	// Websocket protocal
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	FString KRWSProtocol = TEXT("prolog_websocket");


	// Knowrob server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	int32 MongoServerPort = 27017;


	// Websocket connection to knowrob
	TSharedPtr<FSLKRWSClient> KRWSClient;

	// Keeps access to all the individuals in the world
	ASLIndividualManager* IndividualManager;

	// Manages the mongo connection
	ASLMongoQueryManager* MongoQueryManager;
	

	/* Editor button hacks */
	// Triggers a call to init or reset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bInitAndStartButtonHack = false;

	// Triggers a call to init or reset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bFinishButtonHack = false;
};
