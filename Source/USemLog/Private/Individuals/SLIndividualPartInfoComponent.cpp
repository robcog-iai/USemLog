// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualPartInfoComponent.h"

// Sets default values for this component's properties
USLIndividualPartInfoComponent::USLIndividualPartInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;

	//PartOfSplineMesh = CreateDefaultSubobject<USplineMeshComponent>("SplineMesh");
	//PartOfSplineMesh->SetupAttachment(this);
	//PartOfSplineMesh->SetMobility(EComponentMobility::Movable);
	//PartOfSplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//PartOfSplineMesh->SetSplineUpDir(FVector::UpVector, false);
	//PartOfSplineMesh->PrimaryComponentTick.bCanEverTick = false;
	//PartOfSplineMesh->PostPhysicsComponentTick.bCanEverTick = false;
	//PartOfSplineMesh->bCastDynamicShadow = false;
	//PartOfSplineMesh->CastShadow = false;

	//if (AssetsContainer)
	//{
	//	if (AssetsContainer->SplineMesh)
	//	{
	//		PartOfSplineMesh->SetStaticMesh(AssetsContainer->SplineMesh);
	//	}

	//	if (AssetsContainer->SplineMaterial)
	//	{
	//		PartOfSplineMesh->SetMaterial(0, AssetsContainer->SplineMaterial);
	//		PartOfSplineMesh->SetMaterial(1, AssetsContainer->SplineMaterial);
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's info compoonent asset container is no set.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	//}

	//PartOfSplineMesh->SetVisibility(true);
}


// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualPartInfoComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualPartInfoComponent::OnRegister()
{
	Super::OnRegister();

	// Delegates need to be re-bound after a level load
	if (IsInit())
	{
		BindDelegates();
	}
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualPartInfoComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

}

// Called when the game starts
void USLIndividualPartInfoComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void USLIndividualPartInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// Called before destroying the object.
void USLIndividualPartInfoComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	Super::BeginDestroy();
}

// Connect to owner individual component
bool USLIndividualPartInfoComponent::Init(bool bReset)
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
bool USLIndividualPartInfoComponent::Load(bool bReset)
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
void USLIndividualPartInfoComponent::ToggleVisibility()
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
void USLIndividualPartInfoComponent::HideVisualInfo()
{
	SetVisibility(false, true);
}

// Show the visual info
void USLIndividualPartInfoComponent::ShowVisualInfo()
{
	SetVisibility(true, true);
}

// Set the init flag, return true if the state change
void USLIndividualPartInfoComponent::SetIsInit(bool bNewValue, bool bBroadcast)
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
void USLIndividualPartInfoComponent::SetIsLoaded(bool bNewValue, bool bBroadcast)
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

// Private init implementation
bool USLIndividualPartInfoComponent::InitImpl()
{

	return false;
}

// Private load implementation
bool USLIndividualPartInfoComponent::LoadImpl()
{
	return false;
}

// Update info as soon as the individual changes their data
bool USLIndividualPartInfoComponent::BindDelegates()
{
	bool bRetVal = true;

	return bRetVal;
}