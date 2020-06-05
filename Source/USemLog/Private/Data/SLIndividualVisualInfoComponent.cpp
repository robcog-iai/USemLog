// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfoComponent.h"
#include "Data/SLIndividualComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"


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
	ClassTextSize = 50.f;
	IdTextSize = 25.f;
	//TypeTextSize = 20.f;

	ClassText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ClassTxt"));
	ClassText->SetHorizontalAlignment(EHTA_Center);
	ClassText->SetWorldSize(ClassTextSize);
	ClassText->SetText(FText::FromString(TEXT("UnknownClass")));
	ClassText->SetupAttachment(this);

	IdText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IdTxt"));
	IdText->SetHorizontalAlignment(EHTA_Center);
	IdText->SetWorldSize(IdTextSize);
	IdText->SetText(FText::FromString(TEXT("UnknownId")));
	IdText->SetupAttachment(this);
	IdText->SetRelativeLocation(FVector(0.f, 0.f, -ClassTextSize));
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

	//Init();
}

// Called before destroying the object.
void USLIndividualVisualInfoComponent::BeginDestroy()
{
	if (ClassText && ClassText->IsValidLowLevel())
	{
		ClassText->ConditionalBeginDestroy();
	}

	if (IdText && IdText->IsValidLowLevel())
	{
		IdText->ConditionalBeginDestroy();
	}

	OnSLComponentDestroyed.Broadcast(this);

	Super::BeginDestroy();
}

// Called every frame
void USLIndividualVisualInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Log message"), *FString(__FUNCTION__), __LINE__);
}

// Called when sibling is being destroyed
void USLIndividualVisualInfoComponent::OnSiblingDestroyed(USLIndividualComponent* Component)
{
	// Trigger self destruct
	ConditionalBeginDestroy();
}

// Connect to sibling individual component
bool USLIndividualVisualInfoComponent::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
	}

	// Check if the owner has an individual component
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		Sibling = CastChecked<USLIndividualComponent>(AC);
		Sibling->OnDestroyed.AddDynamic(this, &USLIndividualVisualInfoComponent::OnSiblingDestroyed);
		bIsInit = true;
		return true;
	}

	return false;
}

// Refresh values from parent (returns false if component not init)
bool USLIndividualVisualInfoComponent::RefreshComponents()
{
	if(!bIsInit)
	{
		if (Init())
		{
			return RefreshComponents();
		}
		else
		{
			return false;
		}
	}

	if (USLBaseIndividual* SLI = Sibling->GetCastedIndividualObject<USLBaseIndividual>())
	{
		ClassText->SetText(FText::FromString(SLI->GetClass()));
		IdText->SetText(FText::FromString(SLI->GetId()));
		//TypeText->SetText(FText::FromString(TEXT("Visible Individual")));
	}

	return true;
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
bool USLIndividualVisualInfoComponent::UpdateOrientation()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		CameraManager = PC->PlayerCameraManager;
		UE_LOG(LogTemp, Log, TEXT("%s::%d CameraLoc=%s;"), *FString(__FUNCTION__), __LINE__, *CameraManager->GetTargetLocation().ToString());
	}
	return false;
}
