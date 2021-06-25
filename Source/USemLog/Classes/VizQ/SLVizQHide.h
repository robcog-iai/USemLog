// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "SLVizQHide.generated.h"

// Forward declaration
class ASLKnowrobManager;

/**
 * 
 */
UCLASS()
class USLVizQHide : public USLVizQBase
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Virtual implementation of the execute function
	virtual void ExecuteImpl(ASLKnowrobManager* KRManager) override;

protected:
	/* Hide parameters */
	UPROPERTY(EditAnywhere, Category = "Hide")
	TArray<FString> Individuals;

	UPROPERTY(EditAnywhere, Category = "Hide")
	bool bHideValue = true;


	/* Editor interaction */
	UPROPERTY(EditAnywhere, Category = "Hide|Edit")
	bool bAddSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Hide|Edit")
	bool bRemoveSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Hide|Edit")
	bool bOverwrite = false;

	UPROPERTY(EditAnywhere, Category = "Hide|Edit")
	bool bEnsureUniqueness = true;
};
