// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLReachListener.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "SLManipulatorListener.h"

// Set default values
USLReachListener::USLReachListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	InitSphereRadius(30.f);

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	UpdateRate = 0.24f;
	WeightLimit = 15.0f;
	VolumeLimit = 30000.0f; // 1000cm^3 = 1 Liter

	ShapeColor = FColor::Orange.WithAlpha(64);
}

// Dtor
USLReachListener::~USLReachListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
bool USLReachListener::Init()
{
	if (!bIsInit)
	{
		// Subscribe for grasp notifications from sibling component
		if(SubscribeForGraspEvents())
		{
			bIsInit = true;
			return true;
		}		
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLReachListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		SetGenerateOverlapEvents(true);

		TriggerInitialOverlaps();
		OnComponentBeginOverlap.AddDynamic(this, &USLReachListener::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLReachListener::OnOverlapEnd);

		// Bind to the update callback function
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLReachListener::Update, UpdateRate, true);

		// Mark as started
		bIsStarted = true;
	}
}

// Stop publishing grasp events
void USLReachListener::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called after the C++ constructor and after the properties have been initialized
void USLReachListener::PostInitProperties()
{
	Super::PostInitProperties();
	RelocateSphere();
}

// Called when a property is changed in the editor
void USLReachListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLReachListener, SphereRadius))
	{
		RelocateSphere();
	}
}

// Move the sphere location so that its surface overlaps with the end of the manipulator
void USLReachListener::RelocateSphere()
{
	if (GetOwner())
	{
		USceneComponent* RootComp = GetOwner()->GetRootComponent();
		//FVector Center = GetOwner()->CalculateComponentsBoundingBoxInLocalSpace().GetCenter();
		float BoundsCenterOffsetDist = FVector::Distance(RootComp->Bounds.Origin, RootComp->GetComponentLocation());
		float OwnerRadius;
		float OwnerHalfHeight;
		GetOwner()->GetRootComponent()->CalcBoundingCylinder(OwnerRadius, OwnerHalfHeight);
		if (OwnerRadius < SphereRadius)
		{
			SetRelativeLocation(FVector(SphereRadius - OwnerRadius + BoundsCenterOffsetDist, 0.f, 0.f));
		}
	}
}
#endif // WITH_EDITOR

// Subscribe for grasp events from sibling component
bool USLReachListener::SubscribeForGraspEvents()
{
	if(USLManipulatorListener* Sibling = CastChecked<USLManipulatorListener>(
		GetOwner()->GetComponentByClass(USLManipulatorListener::StaticClass())))
	{
		Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLReachListener::OnSLGraspBegin);
		return true;
	}
	return false;
}



// Update callback, check for changes in the reach model
void USLReachListener::Update()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
}

// Publish currently overlapping components
void USLReachListener::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	const FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		OnOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Check if the object is can be a candidate for reaching
bool USLReachListener::CanBeACandidate(AStaticMeshActor* InObject) const
{
	// Check if the object is movable
	if (!InObject->IsRootComponentMovable())
	{
		return false;
	}

	// Check if actor has a valid static mesh component
	if (UStaticMeshComponent* SMC = InObject->GetStaticMeshComponent())
	{
		// Commented out since handles can be grasped and have no physics on
		//// Check if component has physics on
		//if (!SMC->IsSimulatingPhysics())
		//{
		//	return false;
		//}

		// Check that object is not too heavy/large
		if (SMC->GetMass() < WeightLimit &&
			InObject->GetComponentsBoundingBox().GetVolume() < VolumeLimit)
		{
			return true;
		}
	}

	return false;
}

// Called when sibling detects a grasp
void USLReachListener::OnSLGraspBegin(const FSLEntity& Self, UObject* Other, float Time, const FString& GraspType)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
		FString::Printf(TEXT(" *  Finish reach here! %s T:%f"),
			*Other->GetName(), GetWorld()->GetTimeSeconds()), false, FVector2D(1.5f, 1.5f));
}

// Called on overlap begin events
void USLReachListener::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
	//// Ignore self overlaps 
	//if (OtherActor == GetOwner())
	//{
	//	return;
	//}
}

// Called on overlap end events
void USLReachListener::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
	//// Ignore self overlaps 
	//if (OtherActor == GetOwner())
	//{
	//	return;
	//}

	//if (OtherActor->IsA(AStaticMeshActor::StaticClass()))
	//{
	//	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
	//	//	FString::Printf(TEXT(" *  *  *END* *BCAST* *CONTACT* %s<-->%s T:%f C:%d"),
	//	//		*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds(), OvCounter), false, FVector2D(1.5f, 1.5f));
	//	//OnEndSLContactOverlap.Broadcast(OtherActor);
	//}
}
