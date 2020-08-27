// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLKnowrobConnectHandler.h"
#include "knowrob_ameva.pb.h"
#include "SLKnowrobConnectClient.generated.h"

// Forward declarations
class ASLIndividualManager;
class ASLMongoManager;
class ASLVizManager;
enum class ESLVizMeshType : uint8;
/**
*
**/
UCLASS()
class USEMLOG_API ASLKnowrobConnectClient : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ASLKnowrobConnectClient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set any references
	bool Init();

	// Clear any references
	void Reset();

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

private:
	// Set the individual manager
	bool SetIndividualManager();

	// Set the mongo db manager
	bool SetMongoManager();

	// Set the visualization manager
	bool SetVizManager();

	// Initialize the Mongodb
	bool InitMongo();

	// Parse the proto sequence and trigger function
	void ProcessProtobuf(std::string ProtoStr);

	void ViewEntityPoseAt(const FString& Id, float Timestamp, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit = false);

	void ViewEntityTraj(const FString& Id, float start, float end, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit = false);

	ESLVizMeshType GetVizMeshType(knowrob_ameva::MarkerType Marker);

	FLinearColor GetMarkerColor(FString Color);

private:
	// True if the manager is init
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the individuals manager is set and initialized
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIndividualManagerSet : 1;

	// True if the mongo manager is set and initialized
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bMongoManagerSet : 1;

	// True if the viz manager is set and initialized
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bVizManagerSet : 1;

	// Used to get access to the semantic individuals
	ASLIndividualManager* IndividualManager;

	// Used to query the subsymbolic data from mongo
	ASLMongoManager* MongoManager;

	// Used to visualize the world using various markers
	ASLVizManager* VizManager;

	// Used to handle websocket connect to knowrob 
	TSharedPtr<FSLKnowrobConnectHandler> Handler;
};
