// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "Viz/SLVizStructs.h"
#include "SLVizQHighlight.generated.h"

// Forward declaration
class ASLKnowrobManager;

UENUM()
enum class ESLVizQHighlightAction : uint8
{
	Highlight			UMETA(DisplayName = "Highlight"),
	Remove				UMETA(DisplayName = "Remove"),
	RemoveAll			UMETA(DisplayName = "RemoveAll"),
};

/**
 * 
 */
UCLASS()
class USLVizQHighlight : public USLVizQBase
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
	/* Highlight parameters */
	UPROPERTY(EditAnywhere, Category = "Highlight")
	TArray<FString> Ids;

	UPROPERTY(EditAnywhere, Category = "Highlight")
	ESLVizQHighlightAction Action = ESLVizQHighlightAction::Highlight;

	UPROPERTY(EditAnywhere, Category = "Highlight", meta=(editcondition="Action==ESLVizQHighlightAction::Highlight"))
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Highlight", meta = (editcondition = "Action==ESLVizQHighlightAction::Highlight"))
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Translucent;


	/* Editor interaction */
	UPROPERTY(EditAnywhere, Category = "Highlight|Edit")
	bool bAddSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Highlight|Edit")
	bool bRemoveSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Highlight|Edit")
	bool bOverwrite = false;

	UPROPERTY(EditAnywhere, Category = "Highlight|Edit")
	bool bEnsureUniqueness = true;
};
