// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonObject.h"
//#include "GenericPlatform/GenericPlatformFile.h"
#include "SLRawData.generated.h"


/**
*
*/
USTRUCT()
struct FUniqueNameAndLocation
{
	GENERATED_USTRUCT_BODY()

	// Default constructor
	FUniqueNameAndLocation() 
	{};
	
	// Constructor with Id
	FUniqueNameAndLocation(FString InUniqueName) 
		: UniqueName(InUniqueName)
	{};

	// Constructor with Id and Location
	FUniqueNameAndLocation(FString InUniqueName, FVector InLocation)
		: UniqueName(InUniqueName), Location(InLocation)
	{};

	// Id of the entity
	FString UniqueName;

	// Previous location 
	FVector Location;
};

/**
 * 
 */
UCLASS()
class SEMLOG_API USLRawData : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLRawData();

	// Destructor
	~USLRawData();

	// Init logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool Init(UWorld* InWorld, const float DistanceThreshold = 0.1f);

	// Set file handle for appending log data to file every update
	UFUNCTION(BlueprintCallable, Category = SL)
	void SetLogToFile(const FString EpisodeId, const FString LogDirectoryPath);
	
	// Log dynamic and static entities to file
	UFUNCTION(BlueprintCallable, Category = SL)
	void FirstLog();

	// Get the dynamic and static entities as json string
	UFUNCTION(BlueprintCallable, Category = SL)
	bool GetFirstJsonEntry(FString& FirstJsonEntry);

	// Log dynamic entities
	UFUNCTION(BlueprintCallable, Category = SL)
	void LogDynamic();

	// Log dynamic entities and return them as json string
	UFUNCTION(BlueprintCallable, Category = SL)
	bool GetDynamicJsonEntry(FString& DynamicJsonEntry);

	// See if logger initialised
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsInit() const { return bIsInit; }

private:
	// Add string to file
	bool AddToFile(FString& JsonString);

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
		const FString& UniqueName,
		FVector& PreviousLocation);

	// Add component's data to the json array
	void AddComponentToJsonArray(
		TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
		USceneComponent* Component,
		const FString& UniqueName,
		FVector& PreviousLocation);

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
};
