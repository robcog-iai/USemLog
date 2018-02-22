// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonObject.h"
#include "SLRawDataLogger.generated.h"

/** Delegate type for new raw data */
DECLARE_MULTICAST_DELEGATE_OneParam(FSLOnNewRawDataSignature, const FString&);

/**
* Unique name and location of the entities to be logged by the raw data logger
* location is used for calculating the distance between then and now
*/
USTRUCT()
struct FUniqueNameAndLocation
{
	GENERATED_USTRUCT_BODY()
	
	// Default constructor
	FUniqueNameAndLocation()
	{};

	// Constructor with Id and Location
	FUniqueNameAndLocation(FString InUniqueName, FVector InLocation = FVector(-INFINITY))
		: UniqueName(InUniqueName), Location(InLocation)
	{};

	// Unique name of the entity
	FString UniqueName;

	// Previous location 
	FVector Location;
};

/**
 * Semantic logger of raw data 
 * (location, rotation of semantically annotated entities in the world)
 */
UCLASS()
class SEMLOG_API USLRawDataLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLRawDataLogger();

	// Destructor
	~USLRawDataLogger();

	// Init logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool Init(UWorld* InWorld, const float DistanceThreshold = 0.1f);

	// Set file handle for appending log data to file every update
	UFUNCTION(BlueprintCallable, Category = SL)
	void InitFileHandle(const FString EpisodeId, const FString LogDirectoryPath);

	// Allow broadcasting the data as events
	UFUNCTION(BlueprintCallable, Category = SL)
	void InitBroadcaster();
	
	// Log dynamic and static entities to file
	UFUNCTION(BlueprintCallable, Category = SL)
	void LogFirstEntry();

	// Log dynamic entities
	UFUNCTION(BlueprintCallable, Category = SL)
	void LogDynamicEntities();

	// Add new dynamic entity for logging
	UFUNCTION(BlueprintCallable, Category = SL)
	void AddNewDynamicEntity(AActor* Actor);

	// Remove dynamic entity from logging
	UFUNCTION(BlueprintCallable, Category = SL)
	void RemoveDynamicEntity(AActor* Actor);

	// See if logger initialized
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsInit() const { return bIsInit; }

	// Delegate to publish the data
	FSLOnNewRawDataSignature OnNewData;

private:
	// Get the dynamic and static entities as json string
	bool GetAllEntitiesAsJson(FString& FirstJsonEntry);

	// Log dynamic entities and return them as json string
	bool GetDynamicEntitiesAsJson(FString& DynamicJsonEntry);

	// Add json content to file
	bool InsertJsonContentToFile(const FString& JsonString);

	// Broadcast json content
	void BroadcastJsonContent(const FString& JsonString);

	// Create Json object with a 3d location
	FORCEINLINE TSharedPtr<FJsonObject> CreateLocationJsonObject(const FVector& Location);

	// Create Json object with a 3d rotation as quaternion 
	FORCEINLINE TSharedPtr<FJsonObject> CreateRotationJsonObject(const FQuat& Rotation);

	// Create Json object with name location and rotation
	FORCEINLINE TSharedPtr<FJsonObject> CreateNameLocRotJsonObject(
		const FString& Name, const FVector& Location, const FQuat& Rotation);

	// Add actors data to the json array
	void AddActorToJsonArray(
		TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
		AActor* Actor,
		FUniqueNameAndLocation &UniqueNameAndLocation);

	// Add component's data to the json array
	void AddComponentToJsonArray(
		TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
		USceneComponent* Component,
		FUniqueNameAndLocation &UniqueNameAndLocation);

	// Distance threshold (squared for faster comparisons)
	float SquaredDistanceThreshold;

	// Pointer to the world
	UWorld* World;

	// File handle to append the raw data
	IFileHandle* FileHandle;

	// Dynamic actors with their unique name and previous location
	TMap<AActor*, FUniqueNameAndLocation> DynamicActorsWithData;

	// Dynamic components with their unique name and previous location
	TMap<USceneComponent*, FUniqueNameAndLocation> DynamicComponentsWithData;

	// Logger initialized
	bool bIsInit;

	// Logging to file
	bool bLogToFile;

	// Broadcast data
	bool bBroadcastData;
};
