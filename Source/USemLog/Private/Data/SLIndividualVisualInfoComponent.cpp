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

	FirstLineSize = 10.f;
	SecondLineSize = 3.f;
	ThirdLineSize = 4.f;

	UMaterialInterface* MI = Cast<UMaterialInterface>(StaticLoadObject(
		UMaterialInterface::StaticClass(), NULL, TEXT("Material'/USemLog/Individual/M_InfoTextTranslucent.M_InfoTextTranslucent'"),
		NULL, LOAD_None, NULL));
	
	FirstLine = CreateDefaultTextSubobject(FirstLineSize, 0, FString("FirstLine"), FColor::White, MI);
	SecondLine = CreateDefaultTextSubobject(SecondLineSize, FirstLineSize, FString("SecondLine"), FColor::Blue, MI);
	ThirdLine = CreateDefaultTextSubobject(ThirdLineSize, FirstLineSize+SecondLineSize, FString("ThirdLine"), FColor::Red, MI);
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
}

// Called before destroying the object.
void USLIndividualVisualInfoComponent::BeginDestroy()
{
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

// Called when sibling is being destroyed
void USLIndividualVisualInfoComponent::OnSiblingIndividualComponentDestroyed(USLIndividualComponent* Component)
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
		FirstLine->SetText(FText::FromString("---"));
	}
	else
	{
		FirstLine->SetText(FText::FromString(NewVal));
	}
	SetStateColor();
}

// Called when the individual id value has changed
void USLIndividualVisualInfoComponent::OnIndividualIdChanged(USLBaseIndividual* BI, const FString& NewVal)
{
	if (NewVal.IsEmpty())
	{
		SecondLine->SetText(FText::FromString("---"));
	}
	else
	{
		SecondLine->SetText(FText::FromString(NewVal));
	}
	SetStateColor();
}

// Set the color of the text depending on the sibling state;
void USLIndividualVisualInfoComponent::SetStateColor()
{
	if (Sibling->IsLoaded())
	{
		FirstLine->SetTextRenderColor(FColor::Green);
	}
	else if (Sibling->IsInit())
	{
		FirstLine->SetTextRenderColor(FColor::Yellow);
	}
	else
	{
		FirstLine->SetTextRenderColor(FColor::Red);
	}
}

// Render text subobject creation helper
UTextRenderComponent* USLIndividualVisualInfoComponent::CreateDefaultTextSubobject(float Size, float Offset, const FString& DefaultName, FColor Color, UMaterialInterface* MaterialInterface)
{
	UTextRenderComponent* TRC = CreateDefaultSubobject<UTextRenderComponent>(*DefaultName);
	TRC->SetHorizontalAlignment(EHTA_Center);
	TRC->SetVerticalAlignment(EVRTA_TextCenter);
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

// Connect to sibling individual component
bool USLIndividualVisualInfoComponent::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
	}

	if (!bIsInit)
	{
		// Check if the owner has an individual component
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			Sibling = CastChecked<USLIndividualComponent>(AC);
			Sibling->OnDestroyed.AddDynamic(this, &USLIndividualVisualInfoComponent::OnSiblingIndividualComponentDestroyed);
			bIsInit = true;
			return true;
		}
	}

	return false;
}

// Refresh values from parent (returns false if component not init)
bool USLIndividualVisualInfoComponent::Load(bool bReset)
{
	if (bReset)
	{
		bIsLoaded = false;
	}

	if (bIsLoaded)
	{
		return true;
	}

	if(!bIsInit)
	{
		if (!Init(bReset))
		{
			return false;
		}
	}

	if (Sibling && Sibling->IsValidLowLevel() && Sibling->Init())
	{
		if (USLBaseIndividual* SLI = Sibling->GetCastedIndividualObject<USLBaseIndividual>())
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

			const FString ClassVal = SLI->HasClass() ?  SLI->GetClass() : "---";
			const FString IdVal = SLI->HasId() ? SLI->GetId() : "---";

			FirstLine->SetText(FText::FromString(ClassVal));
			SecondLine->SetText(FText::FromString(IdVal));
			ThirdLine->SetText(FText::FromString(SLI->GetTypeName()));

			bIsLoaded = true;
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d This should not happen, the sibling should be set here.."),
			*FString(__FUNCTION__), __LINE__);
	}
	return false;
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
