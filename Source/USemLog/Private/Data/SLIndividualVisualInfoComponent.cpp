// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfoComponent.h"
#include "Data/SLIndividualComponent.h"
#include "Data/SLIndividualVisualInfoAssets.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h" // FindLookAtRotation
#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif //WITH_EDITOR


// Sets default values for this component's properties
USLIndividualVisualInfoComponent::USLIndividualVisualInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;
	OwnerIndividualComponent = nullptr;
	OwnerIndividualObj = nullptr;

	TextSize = 5.f;
	FirstLineTextSizeRatio = 1.f;
	SecondLineTextSizeRatio = 0.8f;
	ThirdLineTextSizeRatio = 0.8f;


	static USLIndividualVisualInfoAssets* AssetsContainer = Cast<USLIndividualVisualInfoAssets>(StaticLoadObject(
		USLIndividualVisualInfoAssets::StaticClass(), NULL, AssetContainerPath,
		NULL, LOAD_None, NULL));

	FirstText = CreateDefaultTextSubobject("FirstLine", AssetsContainer);
	SecondText = CreateDefaultTextSubobject("SecondLine", AssetsContainer);
	SecondText->SetTextRenderColor(FColor::Red);
	ThirdText = CreateDefaultTextSubobject("ThirdLine", AssetsContainer);
	ThirdText->SetTextRenderColor(FColor::White);
}

	
// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualVisualInfoComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualVisualInfoComponent::OnRegister()
{
	Super::OnRegister();

	// Delegates need to be re-bound after a level load
	if (IsInit())
	{
		BindDelegates();
	}
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualVisualInfoComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	// Check if actor already has a semantic data component
	for (const auto AC : GetOwner()->GetComponentsByClass(USLIndividualVisualInfoComponent::StaticClass()))
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
void USLIndividualVisualInfoComponent::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void USLIndividualVisualInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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
void USLIndividualVisualInfoComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	OnDestroyed.Broadcast(this);

	if (FirstText && FirstText->IsValidLowLevel())
	{
		FirstText->ConditionalBeginDestroy();
	}

	if (SecondText && SecondText->IsValidLowLevel())
	{
		SecondText->ConditionalBeginDestroy();
	}

	if (ThirdText && ThirdText->IsValidLowLevel())
	{
		ThirdText->ConditionalBeginDestroy();
	}	

	Super::BeginDestroy();
}

// Connect to owner individual component
bool USLIndividualVisualInfoComponent::Init(bool bReset)
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
bool USLIndividualVisualInfoComponent::Load(bool bReset)
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
void USLIndividualVisualInfoComponent::ToggleVisibility()
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
void USLIndividualVisualInfoComponent::HideVisualInfo()
{
	SetVisibility(false, true);
}

// Show the visual info
void USLIndividualVisualInfoComponent::ShowVisualInfo()
{
	SetVisibility(true, true);
}

// Point text towards the camera
bool USLIndividualVisualInfoComponent::PointToCamera()
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
void USLIndividualVisualInfoComponent::SetIsInit(bool bNewValue, bool bBroadcast)
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
void USLIndividualVisualInfoComponent::SetIsLoaded(bool bNewValue, bool bBroadcast)
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
bool USLIndividualVisualInfoComponent::SetOwnerIndividualComponent()
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
bool USLIndividualVisualInfoComponent::SetOwnerIndividualObj()
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
bool USLIndividualVisualInfoComponent::InitImpl()
{
	bool bIndividualCompSet = HasOwnerIndividualComponent() || SetOwnerIndividualComponent();
	bool bIndividualSet = HasOwnerIndividualObj() || SetOwnerIndividualObj();
	if (bIndividualCompSet && bIndividualSet)
	{
		BindDelegates();
		if (OwnerIndividualComponent->IsInit())
		{
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
bool USLIndividualVisualInfoComponent::LoadImpl()
{
	if (HasOwnerIndividualComponent() && HasOwnerIndividualObj())
	{		
		if (OwnerIndividualComponent->IsInit() || OwnerIndividualComponent->Init())
		{
			// Read any available data
			if (OwnerIndividualObj->HasClass())
			{
				FirstText->SetText(FText::FromString("class:" + OwnerIndividualObj->GetClass()));
			}
			if (OwnerIndividualObj->HasId())
			{
				SecondText->SetText(FText::FromString("id:" + OwnerIndividualObj->GetId()));
			}
			
			ThirdText->SetText(FText::FromString("type:" + OwnerIndividualObj->GetTypeName()));


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
bool USLIndividualVisualInfoComponent::BindDelegates()
{
	bool bRetVal = true;
	if (HasOwnerIndividualComponent())
	{
		/* Init */
		if (!OwnerIndividualComponent->OnInitChanged.IsAlreadyBound(
			this, &USLIndividualVisualInfoComponent::OnOwnerIndividualComponentInitChanged))
		{
			OwnerIndividualComponent->OnInitChanged.AddDynamic(
				this, &USLIndividualVisualInfoComponent::OnOwnerIndividualComponentInitChanged);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component init changed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Load */
		if (!OwnerIndividualComponent->OnLoadedChanged.IsAlreadyBound(
			this, &USLIndividualVisualInfoComponent::OnOwnerIndividualComponentLoadedChanged))
		{
			OwnerIndividualComponent->OnLoadedChanged.AddDynamic(
				this, &USLIndividualVisualInfoComponent::OnOwnerIndividualComponentLoadedChanged);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component loaded changed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Destroyed */
		if (!OwnerIndividualComponent->OnDestroyed.IsAlreadyBound(
			this, &USLIndividualVisualInfoComponent::OnOwnerIndividualComponentDestroyed))
		{
			OwnerIndividualComponent->OnDestroyed.AddDynamic(
				this, &USLIndividualVisualInfoComponent::OnOwnerIndividualComponentDestroyed);
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
			if (!OwnerIndividualObj->OnNewClassValue.IsAlreadyBound(this, &USLIndividualVisualInfoComponent::OnOwnerIndividualClassChanged))
			{
				OwnerIndividualObj->OnNewClassValue.AddDynamic(this, &USLIndividualVisualInfoComponent::OnOwnerIndividualClassChanged);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component on new class delegate is already bound, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}

			/* Id */
			if (!OwnerIndividualObj->OnNewIdValue.IsAlreadyBound(this, &USLIndividualVisualInfoComponent::OnOwnerIndividualIdChanged))
			{
				OwnerIndividualObj->OnNewIdValue.AddDynamic(this, &USLIndividualVisualInfoComponent::OnOwnerIndividualIdChanged);
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
	return bRetVal;
}

// Set the color of the text depending on the owner state;
void USLIndividualVisualInfoComponent::SetTextColors()
{
	if (IsLoaded())
	{
		FirstText->SetTextRenderColor(FColor::Green);
	}
	else if (IsInit())
	{
		FirstText->SetTextRenderColor(FColor::Yellow);
	}
	else
	{
		FirstText->SetTextRenderColor(FColor::Red);
	}	
}

// Recalculate the size of the text
void USLIndividualVisualInfoComponent::ResizeText()
{
	if (GetOwner())
	{
		FVector BoundsOrigin;
		FVector BoxExtent;
		GetOwner()->GetActorBounds(false, BoundsOrigin, BoxExtent);
		TextSize = FMath::Clamp(BoxExtent.Size() / 30.f, MinClampTextSize, MaxClampTextSize);
	}

	const float FirstSize = TextSize * FirstLineTextSizeRatio;
	const float SecondSize = TextSize * SecondLineTextSizeRatio;
	const float ThirdSize = TextSize * ThirdLineTextSizeRatio;

	const float FirstRelLoc = (FirstSize + SecondSize + ThirdSize);
	const float SecondRelLoc = (SecondSize + ThirdSize);
	const float ThirdRelLoc = ThirdSize;

	if (FirstText && FirstText->IsValidLowLevel() && !FirstText->IsPendingKill())
	{
		FirstText->SetWorldSize(FirstSize);
		FirstText->SetRelativeLocation(FVector(0.f, 0.f, FirstRelLoc));
	}

	if (SecondText && SecondText->IsValidLowLevel() && !SecondText->IsPendingKill())
	{
		SecondText->SetWorldSize(SecondSize);
		SecondText->SetRelativeLocation(FVector(0.f, 0.f, SecondRelLoc));
	}

	if (ThirdText && ThirdText->IsValidLowLevel() && !ThirdText->IsPendingKill())
	{
		ThirdText->SetWorldSize(ThirdSize);
		ThirdText->SetRelativeLocation(FVector(0.f, 0.f, ThirdRelLoc));
	}
}

// Set the text values to default
void USLIndividualVisualInfoComponent::ResetTextContent()
{
	FirstText->SetText(FText::FromString("class:"));
	SecondText->SetText(FText::FromString("id:"));
	ThirdText->SetText(FText::FromString("type:"));
}

// Render text subobject creation helper
UTextRenderComponent* USLIndividualVisualInfoComponent::CreateDefaultTextSubobject(const FString& DefaultName, USLIndividualVisualInfoAssets* Assets)
{
	UTextRenderComponent* TRC = CreateDefaultSubobject<UTextRenderComponent>(*DefaultName);
	TRC->SetHorizontalAlignment(EHTA_Center);
	TRC->SetVerticalAlignment(EVRTA_TextBottom);
	TRC->SetText(FText::FromString(DefaultName));
	TRC->SetupAttachment(this);
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
void USLIndividualVisualInfoComponent::OnOwnerIndividualComponentInitChanged(USLIndividualComponent* Component, bool bNewVal)
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
void USLIndividualVisualInfoComponent::OnOwnerIndividualComponentLoadedChanged(USLIndividualComponent* Component, bool bNewVal)
{
	if (bNewVal != IsLoaded())
	{
		SetIsLoaded(bNewVal);
		SetTextColors();
	}
}

// Called when owner is being destroyed
void USLIndividualVisualInfoComponent::OnOwnerIndividualComponentDestroyed(USLIndividualComponent* Component)
{
	// Trigger self destruct
	ConditionalBeginDestroy();
}

// Called when the individual class value has changed
void USLIndividualVisualInfoComponent::OnOwnerIndividualClassChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		FirstText->SetText(FText::FromString("class:"));
	}
	else
	{
		FirstText->SetText(FText::FromString("class:" + NewVal));
	}
}

// Called when the individual id value has changed
void USLIndividualVisualInfoComponent::OnOwnerIndividualIdChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		SecondText->SetText(FText::FromString("id:"));
	}
	else
	{
		SecondText->SetText(FText::FromString("id:" + NewVal));
	}
}

