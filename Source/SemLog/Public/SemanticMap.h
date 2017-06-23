// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "OwlUtils.h"
#include "UObject/NoExportTypes.h"
#include "SemanticMap.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USemanticMap : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USemanticMap();

	// Destructor
	~USemanticMap();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FString Filename;

	// Semantic logging directory path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FString LogDirectoryPath;

	// Semantic map object // TODO see which version to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FOwlObject SemanticMapObject;

	// Class name of the semantic map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FString Class;

	// Unique ID of the semantic map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FString Id;

	// Namespace of the semantic map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FString Ns;

	// Semantic map document as owl representation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SemanticMap)
	FOwlDocument OwlDocument;

	// Shows if the map already exists
	UPROPERTY(BlueprintReadOnly, Category = SemanticMap)
	bool bExists;

	// Shows if the default values have been set
	UPROPERTY(BlueprintReadOnly, Category = SemanticMap)
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
