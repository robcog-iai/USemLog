// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLogVision.h"
#include "GameFramework/Actor.h"
#include "SLVisHelpers.h" // ESLVisRenderType
#include "Engine/GameViewportClient.h"
#include "SLVisLiveViewMaskHandler.h"
#include "SLVisLiveViewManager.generated.h"

/**
* Changes the render type of the view on user input
*/
UCLASS()
class USEMLOGVISION_API ASLVisLiveViewManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisLiveViewManager();

	// Init the manager
	void Init();

	// Start the manager, bind inputs
	void Start();

	// Finish the manager
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Bind user inputs
	void SetupInputBindings();

	// Apply render type
	void ApplyRenderType(ESLVisRenderType Type);

	// Goto next render type
	void GotoNextRender();

	// Goto previous render type
	void GotoPrevRender();

	// Toggle the view mode (lit/unlit)
	void ViewModeToggle();

private:
	// Set when logger is initialized
	bool bIsInit;

	// Set when logger is started
	bool bIsStarted;

	// Set when logger is finished
	bool bIsFinished;

	// The input to select the next render type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName InputNextRenderType;

	// The input to select the previous grasp
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName InputPrevRenderType;

	// The input to select the view mode
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName InputViewMode;

	// Render types
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<ESLVisRenderType> RenderTypes;

	// Mask image visualizer helper
	UPROPERTY() // Avoid GC
	USLVisLiveViewMaskHandler* MaskHandler;

	// Index of the active render type
	uint8 ActiveRenderTypeIdx;

	// Used for switching default render types
	UGameViewportClient* ViewportClient;

	// Flag for the view mode
	bool bViewModeLit;
};
