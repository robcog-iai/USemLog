// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLReachListener.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "SLManipulatorListener.h"
#include "SLEntitiesManager.h"

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
	bCallbacksAreBound = false;
	UpdateRate = 0.24f;
	WeightLimit = 15.0f;
	VolumeLimit = 30000.0f; // 1000cm^3 = 1 Liter

	CurrGraspedObj = nullptr;
	
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
		// Init the semantic entities manager
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(GetWorld());
		}

		// Check that the owner is part of the semantic entities
		SemanticOwner = FSLEntitiesManager::GetInstance()->GetEntity(GetOwner());
		if (!SemanticOwner.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not semantically annotated.."), *FString(__func__), __LINE__);
			return false;
		}
		
		// Subscribe for grasp notifications from sibling component
		if(SubscribeForManipulatorEvents())
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
		// Bind to the update callback function
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLReachListener::Update, UpdateRate, true);
		GetWorld()->GetTimerManager().PauseTimer(TimerHandle);

		SetGenerateOverlapEvents(true);

		TriggerInitialOverlaps();
		
		if(!bCallbacksAreBound)
		{
			OnComponentBeginOverlap.AddDynamic(this, &USLReachListener::OnOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLReachListener::OnOverlapEnd);
			bCallbacksAreBound = true;
		}
		
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
bool USLReachListener::SubscribeForManipulatorEvents()
{
	if(USLManipulatorListener* Sibling = CastChecked<USLManipulatorListener>(
		GetOwner()->GetComponentByClass(USLManipulatorListener::StaticClass())))
	{
		// Timeline reaching,      positioning
		// [-----------contact][contact--------grasp]
		Sibling->OnBeginManipulatorContact.AddUObject(this, &USLReachListener::OnSLManipulatorContactBegin);
		Sibling->OnEndManipulatorContact.AddUObject(this, &USLReachListener::OnSLManipulatorContactEnd);
		Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLReachListener::OnSLGraspBegin);
		Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLReachListener::OnSLGraspEnd);
		return true;
	}
	return false;
}

// Update callback, checks distance to hand, if it increases it resets the start time
void USLReachListener::Update()
{
	AActor* Owner = GetOwner();
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for(auto& C : CandidatesWithTimeAndDistance)
	{
		const float CurrDistSq = FVector::DistSquared(Owner->GetActorLocation(), C.Key->GetActorLocation());
		const float PrevDistSq = C.Value.Get<1>();
		const float DiffDistSq = PrevDistSq - CurrDistSq;

		// Ignore small squared difference changes (MinDistSq)
		if(DiffDistSq > MinDistSq)
		{
			// Positive difference makes the hand closer to the object, update the distance
			C.Value.Get<ESLTimeAndDistance::Dist>() = CurrDistSq;
		}
		else if(DiffDistSq < - MinDistSq)
		{
			// Negative difference makes the hand further away from the object, update distance, reset the start time
			C.Value.Get<ESLTimeAndDistance::Time>() = CurrTime;
			C.Value.Get<ESLTimeAndDistance::Dist>() = CurrDistSq;
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Hand=%s moving away from candidate %s resetting time! DistSq=%f"),
			//	*FString(__func__), __LINE__, CurrTime, *SemanticOwner.Obj->GetName(), *C.Key->GetName(), DiffDistSq);
		}

		// TODO reset idling times, e.g the hand stays still for a longer period, the reaching then only happens if it starts moving
	}
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
	// Make sure the object is semantically annotated
	if (!FSLEntitiesManager::GetInstance()->IsObjectEntitySet(InObject))
	{
		return false;
	}
	
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

// Called when the sibling is in contact with an object, used for ending the reaching event and starting the manipulator positioning event
void USLReachListener::OnSLManipulatorContactBegin(const FSLContactResult& ContactResult)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %f ** Hand START contact with %s"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *ContactResult.Other.Obj->GetName());

	if(CurrGraspedObj)
	{
		// Ignore while in grasp mode
		return;
	}
	
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(ContactResult.Other.Obj))
	{
		// Check if the object in contact with is one of the candidates (should be)
		if (CandidatesWithTimeAndDistance.Contains(AsSMA))
		{
			ObjectsInContactWithManipulator.Emplace(AsSMA, ContactResult.Time);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %f Hand in contact with %s"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}

// Cancel started events
void USLReachListener::OnSLManipulatorContactEnd(const FSLEntity& Self, const FSLEntity& Other, float Time)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d %f ** Hand END in contact with %s"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other.Obj->GetName());
	
	if(CurrGraspedObj)
	{
		// Ignore while in grasp mode
		return;
	}
	
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Other.Obj))
	{
		// Check if the object in contact with is one of the candidates (should be)
		if (ObjectsInContactWithManipulator.Remove(AsSMA) > 0)
		{
			// Reset reach start in the candidate
			if(FSLTimeAndDistance* TimeAndDist = CandidatesWithTimeAndDistance.Find(AsSMA))
			{
				TimeAndDist->Get<ESLTimeAndDistance::Time>() = GetWorld()->GetTimeSeconds();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
			}
			
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Contact ended between the hand and %s"), *FString(__func__), __LINE__, *AsSMA->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}

// Called when sibling detects a grasp, used for ending the manipulator positioning event
void USLReachListener::OnSLGraspBegin(const FSLEntity& Self, UObject* Other, float Time, const FString& GraspType)
{
	if(CurrGraspedObj)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should happen.. "), *FString(__func__), __LINE__);
		return;
	}
	
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Other))
	{
		// Check if the grasped object is a candidate and is in contact with the hand
		if(FSLTimeAndDistance* CandidateTimeAndDist = CandidatesWithTimeAndDistance.Find(AsSMA))
		{
			if(float* ContactTime = ObjectsInContactWithManipulator.Find(AsSMA))
			{
				// Grasp is active, ignore multiple event inputs
				CurrGraspedObj = Other;

				// Broadcast reach and pre grasp events
				const float ReachStartTime = CandidateTimeAndDist->Get<ESLTimeAndDistance::Time>();
				const float ReachEndTime = *ContactTime;
				OnPreAndReachEvent.Broadcast(SemanticOwner, Other, ReachStartTime, ReachEndTime, Time);

				// Remove existing candidates and pause the update callback while the hand is grasping
				CandidatesWithTimeAndDistance.Empty();
				ObjectsInContactWithManipulator.Empty();
				GetWorld()->GetTimerManager().PauseTimer(TimerHandle);


				// Remove overlap callbacks while grasp is active
				if(bCallbacksAreBound)
				{
					OnComponentBeginOverlap.RemoveDynamic(this, &USLReachListener::OnOverlapBegin);
					OnComponentEndOverlap.RemoveDynamic(this, &USLReachListener::OnOverlapEnd);
					bCallbacksAreBound = false;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d This should happen.."), *FString(__func__), __LINE__);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should happen.."), *FString(__func__), __LINE__);
		}
	}
}

// Reset looking for the events
void USLReachListener::OnSLGraspEnd(const FSLEntity& Self, UObject* Other, float Time)
{	
	if(CurrGraspedObj == Other)
	{
		CurrGraspedObj = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.. possible if multiple objects were grasped.."), *FString(__func__), __LINE__);
		return;
	}
	
	// Start looking for new candidates
	TriggerInitialOverlaps();
	
	// Start the overlap callbacks
	if(!bCallbacksAreBound)
	{
		OnComponentBeginOverlap.AddDynamic(this, &USLReachListener::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLReachListener::OnOverlapEnd);
		bCallbacksAreBound = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not have happened.. the callbacks should have been UN-BOUND.."), *FString(__func__), __LINE__);
	}
}

// Called on overlap begin events
void USLReachListener::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore skeletal meshes
	if(AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherActor))
	{
		if(CanBeACandidate(AsSMA))
		{
			const float Distance = FVector::Distance(GetOwner()->GetActorLocation(), AsSMA->GetActorLocation());
			FSLTimeAndDistance TimeAndDistance = MakeTuple(GetWorld()->GetTimeSeconds(), Distance);
			CandidatesWithTimeAndDistance.Emplace(AsSMA, TimeAndDistance);
			
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d %f \t Added %s as candidate."),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
			
			// New candidate added, make sure update callback timer is running
			GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle);
		}
	}
}

// Called on overlap end events
void USLReachListener::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherActor))
	{
		if (CandidatesWithTimeAndDistance.Remove(AsSMA) > 0)
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d %f \t Removed %s as candidate"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());

			if(CandidatesWithTimeAndDistance.Num() == 0)
			{
				//UE_LOG(LogTemp, Error, TEXT("%s::%d %f \t\t No more candidates, pausing the update timer"),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
				GetWorld()->GetTimerManager().PauseTimer(TimerHandle);
			}
		}
	}
}
