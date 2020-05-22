// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfo.h"
#include "Data/SLIndividualComponent.h"
#include "Components/TextRenderComponent.h"

// Sets default values for this component's properties
USLIndividualVisualInfo::USLIndividualVisualInfo()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bIsInit = false;
	ClassTextSize = 50.f;
	IdTextSize = 25.f;
	//TypeTextSize = 20.f;

	ClassText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ClassTxt") /*,true*/);
	//ClassText->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	ClassText->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	ClassText->SetHorizontalAlignment(EHTA_Center);
	ClassText->SetWorldSize(ClassTextSize);
	ClassText->SetText(FText::FromString(TEXT("UnknownClass")));
	ClassText->SetVisibility(true);
	////ClassText->SetupAttachment(this);

	//ClassText->SetFlags(RF_Transactional);

	IdText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IdTxt"));
	IdText->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	IdText->SetHorizontalAlignment(EHTA_Center);
	IdText->SetWorldSize(IdTextSize);
	IdText->SetText(FText::FromString(TEXT("UnknownId")));
	////IdText->SetupAttachment(this);
	IdText->SetRelativeLocation(FVector(0.f, 0.f, -ClassTextSize));
}


// Called when the game starts
void USLIndividualVisualInfo::BeginPlay()
{
	Super::BeginPlay();	
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualVisualInfo::OnRegister()
{
	Super::OnRegister();

	//ClassText = NewObject<UTextRenderComponent>(this, TEXT("ClassTxt"));
	//ClassText->SetHorizontalAlignment(EHTA_Center);
	//ClassText->SetWorldSize(ClassTextSize);
	//ClassText->SetText(FText::FromString(TEXT("UnknownClass")));	
	//ClassText->SetupAttachment(this);

	//IdText = NewObject<UTextRenderComponent>(this, TEXT("IdTxt"));
	//IdText->SetHorizontalAlignment(EHTA_Center);
	//IdText->SetWorldSize(IdTextSize);
	//IdText->SetText(FText::FromString(TEXT("UnknownId")));
	//IdText->SetupAttachment(this);
	//IdText->SetRelativeLocation(FVector(0.f, 0.f, - ClassTextSize));

	//TypeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TypeTxt"));
	//TypeText->SetHorizontalAlignment(EHTA_Center);
	//TypeText->SetWorldSize(TypeTextSize);
	//TypeText->SetText(FText::FromString(TEXT("UnknownType")));
	//TypeText->SetupAttachment(this);
	//TypeText->SetRelativeLocation(FVector(0.f, 0.f, - ClassTextSize - IdTextSize));

	//Init();
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualVisualInfo::PostInitProperties()
{
	Super::PostInitProperties();

	//Init();
}

// Called every frame
void USLIndividualVisualInfo::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Connect to sibling individual component
bool USLIndividualVisualInfo::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
	}

	// Check if the owner has an individual component
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		Sibling = CastChecked<USLIndividualComponent>(AC);
		bIsInit = true;
		return true;
	}

	return false;
}

// Refresh values from parent (returns false if component not init)
bool USLIndividualVisualInfo::Refresh()
{
	if(!bIsInit)
	{
		if (Init())
		{
			return Refresh();
		}
		else
		{
			return false;
		}
	}

	if (USLIndividual* SLI = Sibling->GetCastedIndividualObject<USLIndividual>())
	{
		ClassText->SetText(FText::FromString(SLI->GetClass()));
		IdText->SetText(FText::FromString(SLI->GetId()));
		//TypeText->SetText(FText::FromString(TEXT("Visible Individual")));
	}

	return true;
}

// Hide/show component
void USLIndividualVisualInfo::ToggleVisibility()
{
	if (IsVisible())
	{
		ClassText->SetVisibility(false);
		IdText->SetVisibility(false);
		SetVisibility(false);
	}
	else
	{
		ClassText->SetVisibility(true);
		IdText->SetVisibility(true);
		SetVisibility(true);
	}
}
