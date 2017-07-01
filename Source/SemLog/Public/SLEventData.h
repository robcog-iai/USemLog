// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLOwl.h"
#include "SLEventData.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USLEventData : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLEventData();

	// Destructor
	~USLEventData();

	//// Write document to file
	//UFUNCTION(BlueprintCallable)
	//bool WriteToFile(bool bOverwrite = false);

	//// Filename
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FString Filename;

	//// Semantic logging directory path
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FString LogDirectoryPath;

	//// Semantic map object // TODO see which version to use
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FOwlObject OwlObject;

	//// Class name of the event logs
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FString Class;

	//// Unique ID of the event logs
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FString Id;

	//// Namespace of the event logs
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FString Ns;

	//// Event logs document as owl representation
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	//FOwlDocument OwlDocument;
};
