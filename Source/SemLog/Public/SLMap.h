// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLOwl.h"
#include "UObject/NoExportTypes.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FString Filename;

	// Semantic logging directory path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FString LogDirectoryPath;

	// Semantic map object // TODO see which version to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FOwlObject SLMapObject;

	// Class name of the semantic map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FString Class;

	// Unique ID of the semantic map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FString Id;

	// Namespace of the semantic map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FString Ns;

	// Semantic map document as owl representation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SLMap)
	FOwlDocument OwlDocument;

	// Shows if the map already exists
	UPROPERTY(BlueprintReadOnly, Category = SLMap)
	bool bExists;

	// Shows if the default values have been set
	UPROPERTY(BlueprintReadOnly, Category = SLMap)
	bool bDefaultValuesSet;

private:
	// Full name of the semantic map, Class + Id 
	FString Name;

	// Full name of the semantic map, Ns + Name
	FString FullName;

	// Set document default values
	FORCEINLINE bool SetDefaultValues();

	// Remove the default values
	FORCEINLINE bool RemoveDefaultValues();

	// Insert individual to the map with its 3D transform
	FORCEINLINE bool InsertIndividual(const TPair<AActor*, TMap<FString, FString>>& ActorWithProperties);
};
