// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLOwl.h"
#include "SLMap.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USLMap : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLMap();

	// Destructor
	~USLMap();

	// Check if semantic map already exists
	UFUNCTION(BlueprintCallable)
	bool Exists();

	// Generate the semantic map
	UFUNCTION(BlueprintCallable)
	bool Generate(UWorld* World);

	// Write document to file
	UFUNCTION(BlueprintCallable)
	bool WriteToFile(bool bOverwrite = false);

	// Filename
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	FString Filename;

	// Semantic logging directory path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	FString LogDirectoryPath;

	// Semantic map individual (ns + class + id)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	FOwlIndividualName SemMapIndividual;

	// Semantic map document as owl representation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	FOwlDocument OwlDocument;

	// Shows if the map already exists
	uint8 bExists : 1;

	// Shows if the default values of the owl document have been set
	uint8 bOwlDefaultValuesSet : 1;

private:
	// Map of extra properties of the actors
	TMap<AActor*, TArray<FOwlTriple>> ActorToExtraProperties;

	// Map of extra properties of the components
	TMap<UActorComponent*, TArray<FOwlTriple>> ComponentToExtraProperties;

	// Check parent-child attachment properties
	void AddParentChildAttachmentProperties(const TMap<AActor*, TMap<FString, FString>>& ActorToTagProperties);

	// Check skeletal mesh properties
	void AddExtraProperties(const TMap<AActor*, TMap<FString, FString>>& ActorToTagProperties,
		const TMap<UActorComponent*, TMap<FString, FString>>& ComponentToTagProperties);

	// Set document default values
	void SetDefaultValues();

	// Remove document default values
	void RemoveDefaultValues();

	// Insert actor individual to the map with its 3D transform
	void InsertActorIndividual(const TPair<AActor*, TMap<FString, FString>>& ActorWithProperties);

	// Insert component individual to the map with its 3D transform
	void InsertComponentIndividual(const TPair<UActorComponent*, TMap<FString, FString>>& ComponentWithTagProperties);

	// Insert individual to the document
	void InsertIndividual(
		const FString IndividualClass,
		const FString IndividualId,
		const FVector Location,
		const FQuat Quat,
		const FVector BoundingBox,
		const TArray<FOwlTriple>& ExtraProperties = TArray<FOwlTriple>(),
		bool bSaveAsRightHandedCoordinate = true);
};
