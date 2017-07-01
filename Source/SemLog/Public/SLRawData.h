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
	
	// Initialise the logger and log dynamic and static entities
	void InitAndFirstLog(UWorld* InWorld);

	// Log dynamic entities
	void LogDynamicEntities();

private:
	// Create Json object with a 3d location
	FORCEINLINE TSharedPtr<FJsonObject> CreateLocationJsonObject(const FVector& Location);

	// Create Json object with a 3d rotation as quaternion 
	FORCEINLINE TSharedPtr<FJsonObject> CreateRotationJsonObject(const FQuat& Rotation);

	// Create Json object with name location and rotation
	FORCEINLINE TSharedPtr<FJsonObject> CreateNameLocRotJsonObject(
	const FString& Name, const FVector& Location, const FQuat& Rotation);

	// Add the entity's raw data to the json array
	void AddActorToJsonArray(
		TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
		AActor* Actor,
		const FString& UniqueName,
		FVector& PreviousLocation);

	// Filename
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Filename;

	// Filename
	UPROPERTY(EditAnywhere, Category = "SL")
	FString LogDirectoryPath;

	// Log entities which moved at least with the given distance
	UPROPERTY(EditAnywhere, Category = "SL")
	float DistanceThreshold;
	
	// Distance threshold (squared for faster comparisons)
	float SquaredDistanceThreshold;

	// Pointer to the world
	UWorld* World;

	// File handle to append the raw data
	IFileHandle* FileHandle;

	// Dynamic actors with their unique name and previous location
	TMap<AActor*, FUniqueNameAndLocation> DynamicActorsWithData;
};
