// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLiveViewManager.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "SLVisLoggerSpectatorPC.h"

// Sets default values
ASLVisLiveViewManager::ASLVisLiveViewManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	InputNextRenderType = "NextRenderType";
	InputPrevRenderType = "PrevRenderType";

	RenderTypes.Emplace(ESLVisRenderType::Color);
	RenderTypes.Emplace(ESLVisRenderType::Depth);
	RenderTypes.Emplace(ESLVisRenderType::Normal);
	RenderTypes.Emplace(ESLVisRenderType::Specular);
	RenderTypes.Emplace(ESLVisRenderType::Mask);

	ActiveRenderTypeIdx = 0;
}

// Called when the game starts or when spawned
void ASLVisLiveViewManager::BeginPlay()
{
	Super::BeginPlay();

	Init();
	Start();
}

// Called when actor removed from game or game ended
void ASLVisLiveViewManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// Finish
	Finish();
}

// Init the manager
void ASLVisLiveViewManager::Init()
{
	if (!bIsInit)
	{
		ViewportClient = GetWorld()->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%dCould not access UGameViewportClient.."), *FString(__func__), __LINE__);
			return;
		}
		
		// Make sure there is at least one render type
		if(RenderTypes.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No render types added.."), *FString(__func__), __LINE__);
			return;
		}

		// Create mask handler if required
		if(RenderTypes.Contains(ESLVisRenderType::Mask))
		{
			MaskHandler = NewObject<USLVisLiveViewMaskHandler>(this);
			MaskHandler->Init();
			if(!MaskHandler->IsInit())
			{
				RenderTypes.Remove(ESLVisRenderType::Mask);
			}
		}
		

		bIsInit = true;
	}
}

// Start the manager, bind inputs
void ASLVisLiveViewManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// By default is color rendered
		if(RenderTypes[0] != ESLVisRenderType::Color)
		{
			// Goto the first render type
			ApplyRenderType(RenderTypes[0]);
		}

		SetupInputBindings();

		bIsStarted = true;
	}
}

// Finish the manager
void ASLVisLiveViewManager::Finish()
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Bind user inputs
void ASLVisLiveViewManager::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(InputNextRenderType, IE_Released, this, &ASLVisLiveViewManager::GotoNextRender);
			IC->BindAction(InputPrevRenderType, IE_Released, this, &ASLVisLiveViewManager::GotoPrevRender);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No player controller found.."), *FString(__func__), __LINE__);
	}
}

// Apply render type
void ASLVisLiveViewManager::ApplyRenderType(ESLVisRenderType Type)
{
	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if(Type == ESLVisRenderType::Color)
	{
		if (MaskHandler && MaskHandler->AreMasksOn())
		{
			MaskHandler->ApplyOriginalMaterials();
			ViewportClient->GetEngineShowFlags()->SetPostProcessing(true);

			// TODO Kept here in case one needs to test with/without these params
			//ViewportClient->GetEngineShowFlags()->SetLighting(true);
			//ViewportClient->GetEngineShowFlags()->SetColorGrading(true);
			//ViewportClient->GetEngineShowFlags()->SetTonemapper(true);
			//ViewportClient->GetEngineShowFlags()->SetMaterials(true);
			//ViewportClient->GetEngineShowFlags()->SetAntiAliasing(true);
		}

		// Disable any previous buffer vis
		ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
	}
	else if(Type == ESLVisRenderType::Depth)
	{
		if (MaskHandler && MaskHandler->AreMasksOn())
		{
			MaskHandler->ApplyOriginalMaterials();
			ViewportClient->GetEngineShowFlags()->SetPostProcessing(true);
		}

		// Select the buffer to vis
		ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		BufferVisTargetCV->Set(*FString("SLSceneDepthWorldUnits"));
		
	}
	else if(Type == ESLVisRenderType::Normal)
	{
		if (MaskHandler && MaskHandler->AreMasksOn())
		{
			MaskHandler->ApplyOriginalMaterials();
			ViewportClient->GetEngineShowFlags()->SetPostProcessing(true);
		}

		// Select the buffer to vis
		ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		BufferVisTargetCV->Set(*FString("WorldNormal"));
	}
	else if(Type == ESLVisRenderType::Specular)
	{
		if (MaskHandler && MaskHandler->AreMasksOn())
		{
			MaskHandler->ApplyOriginalMaterials();
			ViewportClient->GetEngineShowFlags()->SetPostProcessing(true);
		}

		// Select the buffer to vis
		ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		BufferVisTargetCV->Set(*FString("Specular"));
	}
	else if(Type == ESLVisRenderType::Mask)
	{
		if (MaskHandler && MaskHandler->IsInit())
		{
			MaskHandler->ApplyMaskMaterials();
			ViewportClient->GetEngineShowFlags()->SetPostProcessing(false);

			// TODO Kept here in case one needs to test with/without these params
			//ViewportClient->GetEngineShowFlags()->SetLighting(false);
			//ViewportClient->GetEngineShowFlags()->SetColorGrading(false);
			//ViewportClient->GetEngineShowFlags()->SetTonemapper(false);
			//ViewportClient->GetEngineShowFlags()->SetMaterials(false);
			//ViewportClient->GetEngineShowFlags()->SetAntiAliasing(false);

			//ViewportClient->GetEngineShowFlags()->SetSceneColorFringe(false);
			//ViewportClient->GetEngineShowFlags()->SetAtmosphericFog(false);
			//ViewportClient->GetEngineShowFlags()->SetGlobalIllumination(true);

			////ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			////BufferVisTargetCV->Set(TEXT("BaseColor"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Unknown render type.."), *FString(__func__), __LINE__);
	}
}

// Goto next render type
void ASLVisLiveViewManager::GotoNextRender()
{
	// Increase the index, if it is the last in the array, set it to the first entry
	ActiveRenderTypeIdx = ActiveRenderTypeIdx >= RenderTypes.Num() - 1 ? 0 : ActiveRenderTypeIdx + 1;
	ApplyRenderType(RenderTypes[ActiveRenderTypeIdx]);
}

// Goto previous render type
void ASLVisLiveViewManager::GotoPrevRender()
{
	// Decrease the index, if it becomes smaller than 0, set it to the last entry 
	ActiveRenderTypeIdx = ActiveRenderTypeIdx <= 0 ? RenderTypes.Num() - 1 : ActiveRenderTypeIdx - 1;
	ApplyRenderType(RenderTypes[ActiveRenderTypeIdx]);
}
