// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualTextInfoComponent.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualVisualAssets.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h" // FindLookAtRotation
#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif //WITH_EDITOR


// Sets default values for this component's properties
USLIndividualTextInfoComponent::USLIndividualTextInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;
	OwnerIndividualComponent = nullptr;
	OwnerIndividualObj = nullptr;

	bDelegatesBound = false;

	TextScreenSize = 5.f;
	ScaleText1 = 1.f;
	ScaleText2 = 0.8f;
	ScaleText3 = 0.8f;

	static USLIndividualVisualAssets* AssetsContainer = Cast<USLIndividualVisualAssets>(StaticLoadObject(
		USLIndividualVisualAssets::StaticClass(), NULL, AssetContainerPath,
		NULL, LOAD_None, NULL));

	Text1 = CreateTextComponentSubobject("FirstText", AssetsContainer);
	Text2 = CreateTextComponentSubobject("SecondText", AssetsContainer);
	Text2->SetTextRenderColor(FColor::Red);
	Text3 = CreateTextComponentSubobject("ThirdText", AssetsContainer);
	Text3->SetTextRenderColor(FColor::White);
}
	
// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualTextInfoComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualTextInfoComponent::OnRegister()
{
	Super::OnRegister();

	// Delegates need to be re-bound after a level load
	if (IsInit() && !bDelegatesBound)
	{
		BindDelegates();
	}
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualTextInfoComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	// Check if actor already has a semantic data component
	for (const auto AC : GetOwner()->GetComponentsByClass(USLIndividualTextInfoComponent::StaticClass()))
	{
		if (AC != this)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has a visual info component (%s), self-destruction commenced.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
			//DestroyComponent();
			ConditionalBeginDestroy();
			return;
		}
	}

	ResizeText();
	ResetTextContent();
	SetTextColors();
	PointToCamera();
}

// Called when the game starts
void USLIndividualTextInfoComponent::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void USLIndividualTextInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GetOwner()->WasRecentlyRendered())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s WasRecentlyRendered"), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d NOT %s WasRecentlyRendered"), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	}
}

// Called before destroying the object.
void USLIndividualTextInfoComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	OnDestroyed.Broadcast(this);

	if (Text1 && Text1->IsValidLowLevel())
	{
		Text1->ConditionalBeginDestroy();
	}

	if (Text2 && Text2->IsValidLowLevel())
	{
		Text2->ConditionalBeginDestroy();
	}

	if (Text3 && Text3->IsValidLowLevel())
	{
		Text3->ConditionalBeginDestroy();
	}	

	Super::BeginDestroy();
}

// Connect to owner individual component
bool USLIndividualTextInfoComponent::Init(bool bReset)
{
	if (bReset)
	{
		SetIsInit(false, false);
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl());
	return IsInit();
}

// Refresh values from parent (returns false if component not init)
bool USLIndividualTextInfoComponent::Load(bool bReset)
{
	if (bReset)
	{
		SetIsLoaded(false, false);
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			return false;
		}
	}

	SetIsLoaded(LoadImpl());
	return IsLoaded();
}

// Hide/show component
void USLIndividualTextInfoComponent::ToggleVisibility()
{
	if (IsVisible())
	{
		HideVisualInfo();
	}
	else
	{
		ShowVisualInfo();
	}
}

// Hide the visual info in the world
void USLIndividualTextInfoComponent::HideVisualInfo()
{
	SetVisibility(false, true);
}

// Show the visual info
void USLIndividualTextInfoComponent::ShowVisualInfo()
{
	SetVisibility(true, true);
}

// Point text towards the camera
bool USLIndividualTextInfoComponent::PointToCamera()
{
	// True if we are in the editor (this is still true when using Play In Editor). You may want to use GWorld->HasBegunPlay in that case)	
	if (GIsEditor)
	{
		// TODO check if standalone e.g. 
		//if (GIsPlayInEditorWorld) 

#if WITH_EDITOR
		for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
		{
			if (LevelVC && LevelVC->IsPerspective())
			{
				SetWorldRotation(LevelVC->GetViewRotation() + FRotator(180.f, 0.f, 180.f));
				break;
			}
		}
#endif //WITH_EDITOR
	}
	else if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		//PC->PlayerCameraManager; // This will not call or yield anything
	}

	return false;
}

// Set the init flag, return true if the state change
void USLIndividualTextInfoComponent::SetIsInit(bool bNewValue, bool bBroadcast)
{
	if (bIsInit != bNewValue)
	{
		if (!bNewValue)
		{
			SetIsLoaded(false);
		}

		bIsInit = bNewValue;
		if (bBroadcast)
		{
			// todo see if broadcast is required
		}
		SetTextColors();
	}
}

// Set the loaded flag
void USLIndividualTextInfoComponent::SetIsLoaded(bool bNewValue, bool bBroadcast)
{
	if (bIsLoaded != bNewValue)
	{
		bIsLoaded = bNewValue;
		if (bBroadcast)
		{
			// todo see if broadcast is required
		}
		SetTextColors();
	}
}

// Set the owner
bool USLIndividualTextInfoComponent::SetOwnerIndividualComponent()
{
	if (HasOwnerIndividualComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual component owner already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	// Check if the owner has an individual component
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		OwnerIndividualComponent = CastChecked<USLIndividualComponent>(AC);
		return true;
	}
	return false;
}

// Set the owner individual
bool USLIndividualTextInfoComponent::SetOwnerIndividualObj()
{
	if (HasOwnerIndividualObj())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual owner already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	if (HasOwnerIndividualComponent() || SetOwnerIndividualComponent())
	{
		if (USLBaseIndividual* SLI = OwnerIndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
		{
			OwnerIndividualObj = SLI;
			return true;
		}
	}

	return false;
}

// Private init implementation
bool USLIndividualTextInfoComponent::InitImpl()
{
	bool bIndividualCompSet = HasOwnerIndividualComponent() || SetOwnerIndividualComponent();
	bool bIndividualSet = HasOwnerIndividualObj() || SetOwnerIndividualObj();
	if (bIndividualCompSet && bIndividualSet)
	{
		BindDelegates();
		if (OwnerIndividualComponent->IsInit())
		{
			if (OwnerIndividualObj->IsAttachedToAnotherIndividual())
			{
				static USLIndividualVisualAssets* AssetsContainer = Cast<USLIndividualVisualAssets>(StaticLoadObject(
					USLIndividualVisualAssets::StaticClass(), NULL, AssetContainerPath,
					NULL, LOAD_None, NULL));
				//PartOfSplineMesh = CreateSplineMeshComponent(AssetsContainer);
				FVector StartLoc, StartTangent, EndTangent;
				//if (PartOfSplineMesh && PartOfSplineMesh->IsValidLowLevel())
				//{
				//	PartOfSplineMesh->SetStartAndEnd(StartLoc, StartTangent,
				//		OwnerIndividualObj->GetPartOfActor()->GetActorLocation(), EndTangent);

				//}
			}
			ResizeText();
			return true;
		}
		else
		{
			ResetTextContent();
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not init info component because the individual component is not init.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}
	}
	return false;
}

// Private load implementation
bool USLIndividualTextInfoComponent::LoadImpl()
{
	if (HasOwnerIndividualComponent() && HasOwnerIndividualObj())
	{		
		if (OwnerIndividualComponent->IsInit() || OwnerIndividualComponent->Init())
		{
			// Read any available data
			if (OwnerIndividualObj->IsClassValueSet())
			{
				Text1->SetText(FText::FromString("class:" + OwnerIndividualObj->GetClassValue()));
			}
			if (OwnerIndividualObj->IsIdValueSet())
			{
				Text2->SetText(FText::FromString("id:" + OwnerIndividualObj->GetIdValue()));
			}
			
			Text3->SetText(FText::FromString("type:" + OwnerIndividualObj->GetTypeName()));


			if (OwnerIndividualComponent->IsLoaded() || OwnerIndividualComponent->Load())
			{
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info comp load failed because individual component is not loaded.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info comp load failed because individual component  is not init.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d This should not happen, the individual component component and individual should be set here.."),
			*FString(__FUNCTION__), __LINE__);
	}
	return false;
}

// Update info as soon as the individual changes their data
bool USLIndividualTextInfoComponent::BindDelegates()
{
	bool bRetVal = true;
	if (HasOwnerIndividualComponent())
	{
		/* Init */
		if (!OwnerIndividualComponent->OnInitChanged.IsAlreadyBound(
			this, &USLIndividualTextInfoComponent::OnOwnerIndividualComponentInitChanged))
		{
			OwnerIndividualComponent->OnInitChanged.AddDynamic(
				this, &USLIndividualTextInfoComponent::OnOwnerIndividualComponentInitChanged);
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component init changed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Load */
		if (!OwnerIndividualComponent->OnLoadedChanged.IsAlreadyBound(
			this, &USLIndividualTextInfoComponent::OnOwnerIndividualComponentLoadedChanged))
		{
			OwnerIndividualComponent->OnLoadedChanged.AddDynamic(
				this, &USLIndividualTextInfoComponent::OnOwnerIndividualComponentLoadedChanged);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component loaded changed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Destroyed */
		if (!OwnerIndividualComponent->OnDestroyed.IsAlreadyBound(
			this, &USLIndividualTextInfoComponent::OnOwnerIndividualComponentDestroyed))
		{
			OwnerIndividualComponent->OnDestroyed.AddDynamic(
				this, &USLIndividualTextInfoComponent::OnOwnerIndividualComponentDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component destroyed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Individual Obj */
		if (HasOwnerIndividualObj())
		{
			/* Class */
			if (!OwnerIndividualObj->OnNewClassValue.IsAlreadyBound(this, &USLIndividualTextInfoComponent::OnOwnerIndividualClassChanged))
			{
				OwnerIndividualObj->OnNewClassValue.AddDynamic(this, &USLIndividualTextInfoComponent::OnOwnerIndividualClassChanged);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component on new class delegate is already bound, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}

			/* Id */
			if (!OwnerIndividualObj->OnNewIdValue.IsAlreadyBound(this, &USLIndividualTextInfoComponent::OnOwnerIndividualIdChanged))
			{
				OwnerIndividualObj->OnNewIdValue.AddDynamic(this, &USLIndividualTextInfoComponent::OnOwnerIndividualIdChanged);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component on new id delegate is already bound, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's owner individual cannot be reached, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			bRetVal = false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's cannot bind delegates, owner component is not set.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		bRetVal = false;
	}
	bDelegatesBound = bRetVal;
	return bRetVal;
}

// Set the color of the text depending on the owner state;
void USLIndividualTextInfoComponent::SetTextColors()
{
	if (IsLoaded())
	{
		Text1->SetTextRenderColor(FColor::Green);
	}
	else if (IsInit())
	{
		Text1->SetTextRenderColor(FColor::Yellow);
	}
	else
	{
		Text1->SetTextRenderColor(FColor::Red);
	}	
}

// Recalculate the size of the text
void USLIndividualTextInfoComponent::ResizeText()
{
	if (GetOwner())
	{
		FVector BoundsOrigin;
		FVector BoxExtent;
		GetOwner()->GetActorBounds(false, BoundsOrigin, BoxExtent);
		TextScreenSize = FMath::Clamp(BoxExtent.Size() / 30.f, MinClampTextSize, MaxClampTextSize);
	}

	const float FirstSize = TextScreenSize * ScaleText1;
	const float SecondSize = TextScreenSize * ScaleText2;
	const float ThirdSize = TextScreenSize * ScaleText3;

	const float FirstRelLoc = (FirstSize + SecondSize + ThirdSize);
	const float SecondRelLoc = (SecondSize + ThirdSize);
	const float ThirdRelLoc = ThirdSize;

	if (Text1 && Text1->IsValidLowLevel() && !Text1->IsPendingKill())
	{
		Text1->SetWorldSize(FirstSize);
		Text1->SetRelativeLocation(FVector(0.f, 0.f, FirstRelLoc));
	}

	if (Text2 && Text2->IsValidLowLevel() && !Text2->IsPendingKill())
	{
		Text2->SetWorldSize(SecondSize);
		Text2->SetRelativeLocation(FVector(0.f, 0.f, SecondRelLoc));
	}

	if (Text3 && Text3->IsValidLowLevel() && !Text3->IsPendingKill())
	{
		Text3->SetWorldSize(ThirdSize);
		Text3->SetRelativeLocation(FVector(0.f, 0.f, ThirdRelLoc));
	}
}

// Set the text values to default
void USLIndividualTextInfoComponent::ResetTextContent()
{
	Text1->SetText(FText::FromString("class:"));
	Text2->SetText(FText::FromString("id:"));
	Text3->SetText(FText::FromString("type:"));
}

// Render text subobject creation helper
UTextRenderComponent* USLIndividualTextInfoComponent::CreateTextComponentSubobject(const FString& DefaultName, USLIndividualVisualAssets* Assets)
{
	UTextRenderComponent* TRC = CreateDefaultSubobject<UTextRenderComponent>(*DefaultName);
	TRC->SetHorizontalAlignment(EHTA_Center);
	TRC->SetVerticalAlignment(EVRTA_TextBottom);
	TRC->SetText(FText::FromString(DefaultName));
	TRC->SetupAttachment(this);
	TRC->PrimaryComponentTick.bCanEverTick = false;
	if (Assets)
	{
		if (Assets->TextMaterialTranslucent)
		{
			TRC->SetTextMaterial(Assets->TextMaterialTranslucent);
		}

		if (Assets->TextFont)
		{
			TRC->SetFont(Assets->TextFont);
		}
	}
	return TRC;
}


/* Delegate functions */
// Called when owners init value has changed
void USLIndividualTextInfoComponent::OnOwnerIndividualComponentInitChanged(USLIndividualComponent* Component, bool bNewVal)
{
	if (bNewVal != IsInit())
	{
		SetIsInit(bNewVal);
		SetTextColors();
		if (!bNewVal)
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d %s's individual component is not init anymore, vis info will need to be re initialized.."),
				*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName());
		}
	}	
}

// Called when owners load value has changed
void USLIndividualTextInfoComponent::OnOwnerIndividualComponentLoadedChanged(USLIndividualComponent* Component, bool bNewVal)
{
	if (bNewVal != IsLoaded())
	{
		SetIsLoaded(bNewVal);
		SetTextColors();
	}
}

// Called when owner is being destroyed
void USLIndividualTextInfoComponent::OnOwnerIndividualComponentDestroyed(USLIndividualComponent* Component)
{
	// Trigger self destruct
	ConditionalBeginDestroy();
}

// Called when the individual class value has changed
void USLIndividualTextInfoComponent::OnOwnerIndividualClassChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		Text1->SetText(FText::FromString("class:"));
	}
	else
	{
		Text1->SetText(FText::FromString("class:" + NewVal));
	}
}

// Called when the individual id value has changed
void USLIndividualTextInfoComponent::OnOwnerIndividualIdChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		Text2->SetText(FText::FromString("id:"));
	}
	else
	{
		Text2->SetText(FText::FromString("id:" + NewVal));
	}
}

