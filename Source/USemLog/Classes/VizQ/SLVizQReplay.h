// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "SLVizQReplay.generated.h"

// Forward declaration
class ASLKnowrobManager;

UENUM()
enum class ESLVizQReplayType : uint8
{
	Goto		UMETA(DisplayName = "Goto"),
	Replay		UMETA(DisplayName = "Replay"),
};

/**
 * 
 */
UCLASS()
class USLVizQReplay : public USLVizQBase
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
	/* Replay parameters */
	UPROPERTY(EditAnywhere, Category = "Replay")
	FString Task;

	UPROPERTY(EditAnywhere, Category = "Replay")
	FString Episode;

	UPROPERTY(EditAnywhere, Category = "Replay")
	ESLVizQReplayType Type = ESLVizQReplayType::Goto;

	UPROPERTY(EditAnywhere, Category = "Replay")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Replay", meta=(editcondition = "Type==ESLVizQReplayType::Replay"))
	float EndTime = -1.f;

	UPROPERTY(EditAnywhere, Category = "Replay", meta = (editcondition = "Type==ESLVizQReplayType::Replay"))
	bool bLoop = true;

	UPROPERTY(EditAnywhere, Category = "Replay", meta = (editcondition = "Type==ESLVizQReplayType::Replay"))
	float UpdateRate = -1.f;

	UPROPERTY(EditAnywhere, Category = "Replay", meta = (editcondition = "Type==ESLVizQReplayType::Replay"))
	int32 StepSize = 1;


	/* Manual interaction */
	UPROPERTY(EditAnywhere, Category = "Manual Interaction|Replay", meta = (editcondition = "Type==ESLVizQReplayType::Replay"))
	bool bPauseRadioButton = false;

	UPROPERTY(EditAnywhere, Category = "Manual Interaction|Replay", meta = (editcondition = "Type==ESLVizQReplayType::Replay"))
	bool bStopButton = false;

	UPROPERTY(EditAnywhere, Category = "Manual Interaction|Replay", meta = (editcondition = "Type==ESLVizQReplayType::Replay"))
	bool bLiveUpdate = false;
};
