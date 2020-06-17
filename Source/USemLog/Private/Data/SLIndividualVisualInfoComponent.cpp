// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfoComponent.h"
#include "Data/SLIndividualComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Materials/MaterialInterface.h"


// Sets default values for this component's properties
USLIndividualVisualInfoComponent::USLIndividualVisualInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITOR
	//PrimaryComponentTick.bStartWithTickEnabled = true;
#endif // WITH_EDITOR

	bIsInit = false;
	bIsLoaded = false;
	OwnerIndividualComponent = nullptr;
	OwnerIndividual = nullptr;

	FirstLineSize = 5.f;
	SecondLineSize = 2.5f;
	ThirdLineSize = 2.5f;

	UMaterialInterface* MI = Cast<UMaterialInterface>(StaticLoadObject(
		UMaterialInterface::StaticClass(), NULL, TEXT("Material'/USemLog/Individual/M_InfoTextTranslucent.M_InfoTextTranslucent'"),
		NULL, LOAD_None, NULL));
	
	FirstLine = CreateDefaultTextSubobject(FirstLineSize, 0, FString("FirstLine"), FColor::White, MI);
	SecondLine = CreateDefaultTextSubobject(SecondLineSize, FirstLineSize, FString("SecondLine"), FColor::White, MI);
	ThirdLine = CreateDefaultTextSubobject(ThirdLineSize, (FirstLineSize + SecondLineSize), FString("ThirdLine"), FColor::White, MI);
}

// Called when the game starts
void USLIndividualVisualInfoComponent::BeginPlay()
{
	Super::BeginPlay();	
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualVisualInfoComponent::OnRegister()
{
	Super::OnRegister();

	//ClassText = NewObject<UTextRenderComponent>(this, TEXT("ClassTxt"));
	//ClassText->SetHorizontalAlignment(EHTA_Center);
	//ClassText->SetWorldSize(ClassTextSize);
	//ClassText->SetText(FText::FromString(TEXT("UnknownClass")));	
	//ClassText->SetupAttachment(this);
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualVisualInfoComponent::PostInitProperties()
{
	Super::PostInitProperties();
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

	ResetText();
	SetColors();
}

// Called before destroying the object.
void USLIndividualVisualInfoComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	OnDestroyed.Broadcast(this);

	if (FirstLine && FirstLine->IsValidLowLevel())
	{
		FirstLine->ConditionalBeginDestroy();
	}

	if (SecondLine && SecondLine->IsValidLowLevel())
	{
		SecondLine->ConditionalBeginDestroy();
	}

	if (ThirdLine && ThirdLine->IsValidLowLevel())
	{
		ThirdLine->ConditionalBeginDestroy();
	}	

	Super::BeginDestroy();
}

// Called every frame
void USLIndividualVisualInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Log message"), *FString(__FUNCTION__), __LINE__);
}

// Connect to sibling individual component
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
	SetColors();
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
	SetColors();
	return IsLoaded();
}

// Hide/show component
bool USLIndividualVisualInfoComponent::ToggleVisibility()
{
	if (IsVisible())
	{
		//ClassText->SetVisibility(false);
		//IdText->SetVisibility(false);
		SetVisibility(false, true);
	}
	else
	{
		//ClassText->SetVisibility(true);
		//IdText->SetVisibility(true);
		SetVisibility(true, true);
	}
	return true;
}

// Point text towards the camera
bool USLIndividualVisualInfoComponent::PointToCamera()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		CameraManager = PC->PlayerCameraManager;
		UE_LOG(LogTemp, Log, TEXT("%s::%d CameraLoc=%s;"), *FString(__FUNCTION__), __LINE__, *CameraManager->GetTargetLocation().ToString());
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
	}
}

// Set the sibling
bool USLIndividualVisualInfoComponent::SetOwnerIndividualComponent()
{
	if (HasOwnerIndividualComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual component sibling already exists, this should not happen.."),
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

bool USLIndividualVisualInfoComponent::SetOwnerIndividual()
{
	if (HasOwnerIndividual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual sibling already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	if (HasOwnerIndividualComponent() || SetOwnerIndividualComponent())
	{
		if (USLBaseIndividual* SLI = OwnerIndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
		{
			OwnerIndividual = SLI;
			return true;
		}
	}

	return false;
}

// Private init implementation
bool USLIndividualVisualInfoComponent::InitImpl()
{
	bool bIndividualCompSet = HasOwnerIndividualComponent() || SetOwnerIndividualComponent();
	bool bIndividualSet = HasOwnerIndividual() || SetOwnerIndividual();
	if (bIndividualCompSet && bIndividualSet)
	{
		if (OwnerIndividualComponent->IsInit())
		{
			if (BindDelegates())
			{
				return true;
			}
		}
		else
		{
			ResetText();
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not init info component because the individual component is not init.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}
	}
	return false;
}

// Private load implementation
bool USLIndividualVisualInfoComponent::LoadImpl()
{
	if (HasOwnerIndividualComponent() && HasOwnerIndividual())
	{		
		if (OwnerIndividualComponent->IsInit() || OwnerIndividualComponent->Init())
		{
			// Read any available data
			if (OwnerIndividual->HasClass())
			{
				FirstLine->SetText(FText::FromString("Class : " + OwnerIndividual->GetClass()));
			}
			if (OwnerIndividual->HasId())
			{
				SecondLine->SetText(FText::FromString("Id : " + OwnerIndividual->GetId()));
			}
			if (OwnerIndividual->HasId())
			{
				ThirdLine->SetText(FText::FromString("Type : " + OwnerIndividual->GetTypeName()));
			}

			if (OwnerIndividualComponent->IsLoaded() || OwnerIndividualComponent->Load())
			{
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info comp load failed because sibling is not loaded.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info comp load failed because sibling is not init.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d This should not happen, the sibling component and individual should be set here.."),
			*FString(__FUNCTION__), __LINE__);
	}
	SetColors();
	return false;
}

// Update info as soon as the individual changes their data
bool USLIndividualVisualInfoComponent::BindDelegates()
{
	bool bRetVal = true;
	if (HasOwnerIndividualComponent())
	{
		/* Sibling init change */
		if (!OwnerIndividualComponent->OnInitChanged.IsAlreadyBound(
			this, &USLIndividualVisualInfoComponent::OnSiblingInitChanged))
		{
			OwnerIndividualComponent->OnInitChanged.AddDynamic(
				this, &USLIndividualVisualInfoComponent::OnSiblingInitChanged);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component init changed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Sibling load change */
		if (!OwnerIndividualComponent->OnLoadedChanged.IsAlreadyBound(
			this, &USLIndividualVisualInfoComponent::OnSiblingLoadedChanged))
		{
			OwnerIndividualComponent->OnLoadedChanged.AddDynamic(
				this, &USLIndividualVisualInfoComponent::OnSiblingLoadedChanged);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component loaded changed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		/* Sibling destroyed */
		if (!OwnerIndividualComponent->OnDestroyed.IsAlreadyBound(
			this, &USLIndividualVisualInfoComponent::OnSiblingDestroyed))
		{
			OwnerIndividualComponent->OnDestroyed.AddDynamic(
				this, &USLIndividualVisualInfoComponent::OnSiblingDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component destroyed delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		// Bind data changes
		if (USLBaseIndividual* SLI = OwnerIndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
		{
			if (!SLI->OnNewClassValue.IsAlreadyBound(this, &USLIndividualVisualInfoComponent::OnIndividualClassChanged))
			{
				SLI->OnNewClassValue.AddDynamic(this, &USLIndividualVisualInfoComponent::OnIndividualClassChanged);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component on new class delegate is already bound, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}

			if (!SLI->OnNewIdValue.IsAlreadyBound(this, &USLIndividualVisualInfoComponent::OnIndividualIdChanged))
			{
				SLI->OnNewIdValue.AddDynamic(this, &USLIndividualVisualInfoComponent::OnIndividualIdChanged);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info component on new id delegate is already bound, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's sibling individual cannot be reached, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			bRetVal = false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's cannot bind delegates, sibling component is not set.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		bRetVal = false;
	}
	return bRetVal;
}

// Set the color of the text depending on the sibling state;
void USLIndividualVisualInfoComponent::SetColors()
{
	if (IsLoaded())
	{
		FirstLine->SetTextRenderColor(FColor::Green);
	}
	else if (IsInit())
	{
		FirstLine->SetTextRenderColor(FColor::Yellow);
	}
	else
	{
		FirstLine->SetTextRenderColor(FColor::Red);
	}	
}

// Set the text values to default
void USLIndividualVisualInfoComponent::ResetText()
{
	FirstLine->SetText(FText::FromString("Class :"));
	SecondLine->SetText(FText::FromString("Id :"));
	ThirdLine->SetText(FText::FromString("Type :"));
}

// Render text subobject creation helper
UTextRenderComponent* USLIndividualVisualInfoComponent::CreateDefaultTextSubobject(float Size, float Offset, const FString& DefaultName, FColor Color, UMaterialInterface* MaterialInterface)
{
	UTextRenderComponent* TRC = CreateDefaultSubobject<UTextRenderComponent>(*DefaultName);
	TRC->SetHorizontalAlignment(EHTA_Left);
	TRC->SetVerticalAlignment(EVRTA_TextTop);
	TRC->SetWorldSize(Size);
	TRC->SetText(FText::FromString(DefaultName));
	TRC->SetTextRenderColor(Color);
	TRC->SetRelativeLocation(FVector(0.f, 0.f, -Offset));
	TRC->SetupAttachment(this);
	if (MaterialInterface)
	{
		TRC->SetTextMaterial(MaterialInterface);
	}
	return TRC;
}


/* Delegate functions */
// Called when siblings init value has changed
void USLIndividualVisualInfoComponent::OnSiblingInitChanged(USLIndividualComponent* Component, bool bNewVal)
{
	if (!bNewVal)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s's individual component is not init anymore, vis info will need to be re initialized.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName());
		SetIsInit(false);
	}
	SetColors();
}

// Called when siblings load value has changed
void USLIndividualVisualInfoComponent::OnSiblingLoadedChanged(USLIndividualComponent* Component, bool bNewVal)
{
	if (!bNewVal)
	{
		SetIsLoaded(false);
	}
	SetColors();
}

// Called when sibling is being destroyed
void USLIndividualVisualInfoComponent::OnSiblingDestroyed(USLIndividualComponent* Component)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d Sibling %s destroyed, self destroying"),
		*FString(__FUNCTION__), __LINE__, *Component->GetName());
	// Trigger self destruct
	ConditionalBeginDestroy();
}

// Called when the individual class value has changed
void USLIndividualVisualInfoComponent::OnIndividualClassChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		FirstLine->SetText(FText::FromString("Class :"));
	}
	else
	{
		FirstLine->SetText(FText::FromString("Class : " + NewVal));
	}
}

// Called when the individual id value has changed
void USLIndividualVisualInfoComponent::OnIndividualIdChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		SecondLine->SetText(FText::FromString("Id :"));
	}
	else
	{
		SecondLine->SetText(FText::FromString("Id : " + NewVal));
	}
}

