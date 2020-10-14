// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLReachMonitor.h"
#include "Monitors/SLMonitorStructs.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"

#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"


// Set default values
USLReachMonitor::USLReachMonitor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	InitSphereRadius(30.f);

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bCallbacksAreBound = false;

	CurrGraspedObj = nullptr;
	
	ShapeColor = FColor::Orange.WithAlpha(64);
}

// Dtor
USLReachMonitor::~USLReachMonitor()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
bool USLReachMonitor::Init()
{
	if (!bIsInit)
	{

		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			IndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!IndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return false;
		}

		// Set the individual object
		IndividualObject = IndividualComponent->GetIndividualObject();
		
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
void USLReachMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Bind to the update callback function
		GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &USLReachMonitor::ReachUpdate, UpdateRate, true);
		GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle);

		SetGenerateOverlapEvents(true);

		TriggerInitialOverlaps();
		
		if(!bCallbacksAreBound)
		{
			OnComponentBeginOverlap.AddDynamic(this, &USLReachMonitor::OnOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLReachMonitor::OnOverlapEnd);
			bCallbacksAreBound = true;
		}
		
		// Mark as started
		bIsStarted = true;
	}
}

// Stop publishing grasp events
void USLReachMonitor::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if(bCallbacksAreBound)
		{
			OnComponentBeginOverlap.RemoveDynamic(this, &USLReachMonitor::OnOverlapBegin);
			OnComponentEndOverlap.RemoveDynamic(this, &USLReachMonitor::OnOverlapEnd);
			bCallbacksAreBound = false;
		}
		
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called after the C++ constructor and after the properties have been initialized
void USLReachMonitor::PostInitProperties()
{
	Super::PostInitProperties();
	RelocateSphere();
}

// Called when a property is changed in the editor
void USLReachMonitor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLReachMonitor, SphereRadius))
	{
		RelocateSphere();
	}
}

// Move the sphere location so that its surface overlaps with the end of the manipulator
void USLReachMonitor::RelocateSphere()
{
	if (GetOwner())
	{
		USceneComponent* RootComp = GetOwner()->GetRootComponent();
		//FVector Center = GetOwner()->CalculateComponentsBoundingBoxInLocalSpace().GetCenter();
		const float BoundsCenterOffsetDist = FVector::Distance(RootComp->Bounds.Origin, RootComp->GetComponentLocation());
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
bool USLReachMonitor::SubscribeForManipulatorEvents()
{
	if(USLManipulatorMonitor* Sibling = CastChecked<USLManipulatorMonitor>(
		GetOwner()->GetComponentByClass(USLManipulatorMonitor::StaticClass())))
	{
		// Timeline reaching,      positioning
		// [-----------contact][contact--------grasp]
		Sibling->OnBeginManipulatorContact.AddUObject(this, &USLReachMonitor::OnSLManipulatorContactBegin);
		Sibling->OnEndManipulatorContact.AddUObject(this, &USLReachMonitor::OnSLManipulatorContactEnd);
		Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLReachMonitor::OnSLGraspBegin);
		Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLReachMonitor::OnSLGraspEnd);

		return true;
	}
	return false;
}

// Update callback, checks distance to hand, if it increases it resets the start time
void USLReachMonitor::ReachUpdate()
{
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for(auto& C : CandidatesWithTimeAndDistance)
	{
		const float CurrDist = FVector::Distance(GetOwner()->GetActorLocation(), C.Key->GetActorLocation());
		const float PrevDist = C.Value.Get<1>();
		const float DiffDist = PrevDist - CurrDist;

		// Ignore small difference changes (MinDist)
		if(DiffDist > MinDist)
		{
			// Positive difference makes the hand closer to the object, update the distance
			C.Value.Get<ESLTimeAndDist::SLDist>() = CurrDist;

			//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s is moving closer to %s; PrevDist=%f; CurrDist=%f; DiffDist=%f; [%s - %s];"),
			//	*FString(__func__), __LINE__, 
			//	CurrTime, 
			//	*SemanticOwner.Obj->GetName(), 
			//	*C.Key->GetName(),
			//	PrevDist, CurrDist, DiffDist,
			//	*GetOwner()->GetActorLocation().ToString(),
			//	*C.Key->GetActorLocation().ToString());
		}
		else if(DiffDist < -MinDist)
		{
			// Negative difference makes the hand further away from the object, update distance, reset the start time
			C.Value.Get<ESLTimeAndDist::SLTime>() = CurrTime;
			C.Value.Get<ESLTimeAndDist::SLDist>() = CurrDist;

			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s is moving further to %s; PrevDist=%f; CurrDist=%f; DiffDist=%f; [%s - %s];"),
			//	*FString(__func__), __LINE__, 
			//	CurrTime, 
			//	*SemanticOwner.Obj->GetName(), 
			//	*C.Key->GetName(),
			//	PrevDist, CurrDist, DiffDist,
			//	*GetOwner()->GetActorLocation().ToString(),
			//	*C.Key->GetActorLocation().ToString());
		}
		else
		{
			// TODO reset time when idling for a longer period
			//UE_LOG(LogTemp, Log, TEXT("%s::%d [%f] %s is idling relative to %s; PrevDist=%f; CurrDist=%f; DiffDist=%f; [%s - %s];"),
			//	*FString(__func__), __LINE__, 
			//	CurrTime, 
			//	*SemanticOwner.Obj->GetName(), 
			//	*C.Key->GetName(),
			//	PrevDist, CurrDist, DiffDist,
			//	*GetOwner()->GetActorLocation().ToString(),
			//	*C.Key->GetActorLocation().ToString());
		}

	}
}

// Publish currently overlapping components
void USLReachMonitor::TriggerInitialOverlaps()
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
bool USLReachMonitor::CanBeACandidate(AStaticMeshActor* InObject) const
{
	UActorComponent* AC = InObject->GetComponentByClass(USLIndividualComponent::StaticClass());
	return AC != nullptr;
	
	//// Check if the object is movable
	//if (!InObject->IsRootComponentMovable())
	//{
	//	return false;
	//}
	
	//// Check if actor has a valid static mesh component
	//if (UStaticMeshComponent* SMC = InObject->GetStaticMeshComponent())
	//{
	//	// Commented out since handles can be grasped and have no physics on
	//	//// Check if component has physics on
	//	//if (!SMC->IsSimulatingPhysics())
	//	//{
	//	//	return false;
	//	//}

	//	// Check that object is not too heavy/large
	//	if (SMC->GetMass() < ObjectWeightLimit &&
	//		InObject->GetComponentsBoundingBox().GetVolume() < ObjectVolumeLimit)
	//	{
	//		return true;
	//	}
	//}

	//return false;
}

// Checks for candidates in the overlap area
void USLReachMonitor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
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
			const float Dist = FVector::Distance(GetOwner()->GetActorLocation(), AsSMA->GetActorLocation());
			CandidatesWithTimeAndDistance.Emplace(AsSMA, MakeTuple(GetWorld()->GetTimeSeconds(), Dist));

			//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s added as candidate.."),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
			
			// New candidate added, make sure update callback timer is running
			GetWorld()->GetTimerManager().UnPauseTimer(UpdateTimerHandle);
		}
	}
}

// Checks for candidates in the overlap area
void USLReachMonitor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherActor))
	{
		// Remove candidate
		if (CandidatesWithTimeAndDistance.Remove(AsSMA) > 0)
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s removed as candidate.."),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());

			// If it was the last element, pause timer
			if(CandidatesWithTimeAndDistance.Num() == 0)
			{
				GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle);
			}
		}
	}
}


// Called when sibling detects a grasp, used for ending the manipulator positioning event
void USLReachMonitor::OnSLGraspBegin(USLBaseIndividual* Self, AActor* OtherActor, float Time, const FString& GraspType)
{
	if(CurrGraspedObj)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Cannot set %s as grasped object.. manipulator is already grasping %s;"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *OtherActor->GetName(), *CurrGraspedObj->GetName());
		return;
	}

	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherActor))
	{
		// Check if the grasped object is a candidate and is in contact with the hand
		if(FSLTimeAndDist* CandidateTimeAndDist = CandidatesWithTimeAndDistance.Find(AsSMA))
		{
			// TODO this could be an outdated time due to the delay, it however makes sense to keep it this way
			// since if there is a grasp with the object, it should also be in contact with
			if(float* ContactTime = ObjectsInContactWithManipulator.Find(AsSMA))
			{
				// Grasp is active, ignore future contact/grasp events
				CurrGraspedObj = OtherActor;

				//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s set as grasped object.."),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());

				// Cancel delay callback if active
				GetWorld()->GetTimerManager().ClearTimer(ManipulatorContactDelayTimerHandle);

				// Broadcast reach and pre grasp events
				const float ReachStartTime = CandidateTimeAndDist->Get<ESLTimeAndDist::SLTime>();
				const float ReachEndTime = *ContactTime;
				OnPreGraspAndReachEvent.Broadcast(IndividualObject, OtherActor, ReachStartTime, ReachEndTime, Time);

				// Remove existing candidates and pause the update callback while the hand is grasping
				CandidatesWithTimeAndDistance.Empty();
				ObjectsInContactWithManipulator.Empty();
				GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle);


				// Remove overlap callbacks while grasp is active
				if(bCallbacksAreBound)
				{
					OnComponentBeginOverlap.RemoveDynamic(this, &USLReachMonitor::OnOverlapBegin);
					OnComponentEndOverlap.RemoveDynamic(this, &USLReachMonitor::OnOverlapEnd);
					bCallbacksAreBound = false;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Grasped %s is not in the objects in contact with the manipulator list, this should not happen.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Grasped %s is not in the candidates list, this should not happen.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
		}
	}
}

// Reset looking for the events
void USLReachMonitor::OnSLGraspEnd(USLBaseIndividual* Self, AActor* Other, float Time)
{	
	if(CurrGraspedObj == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.. currently grasped object is nullptr while ending grasp with %s"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
		return;
	}

	if(CurrGraspedObj == Other)
	{
		CurrGraspedObj = nullptr;
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s removed as grasped object.."),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] End grasp with %s while %s is still grasped.. ignoring event.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
		return;
	}

	// Start looking for new candidates
	TriggerInitialOverlaps();

	// Start the overlap callbacks
	if(!bCallbacksAreBound)
	{
		OnComponentBeginOverlap.AddDynamic(this, &USLReachMonitor::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLReachMonitor::OnOverlapEnd);
		bCallbacksAreBound = true;
	}
}

// Called when the sibling is in contact with an object, used for ending the reaching event and starting the manipulator positioning event
void USLReachMonitor::OnSLManipulatorContactBegin(const FSLContactResult& ContactResult)
{
	if(CurrGraspedObj)
	{
		// Ignore any manipulator contacts while in grasp mode
		return;
	}
	
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(ContactResult.Other->GetParentActor()))
	{
		// Check if the object in contact with is one of the candidates (should be)
		if (CandidatesWithTimeAndDistance.Contains(AsSMA))
		{
			// Check if the contact should be concatenated 
			if(!SkipRecentManipulatorContactEndEventTime(AsSMA, ContactResult.Time))
			{
				// Overwrite previous time or create a new contact result
				ObjectsInContactWithManipulator.Emplace(AsSMA, ContactResult.Time);
				//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s added as object in contact with the manipulator.."),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s is in contact with the manipulator, but it is not in the candidates list, this should not happen.. "),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *AsSMA->GetName());
		}
	}
}

// Manipulator is not in contact with object anymore, check for possible concatenation, or reset the potential reach time
void USLReachMonitor::OnSLManipulatorContactEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	if(CurrGraspedObj)
	{
		// Ignore any manipulator contacts while in grasp mode
		return;
	}

	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Other->GetParentActor()))
	{
		// Check contact with manipulator (remove in delay callback, give concatenation a chance)
		if (ObjectsInContactWithManipulator.Contains(AsSMA))
		{
			// Cache the event
			RecentlyEndedManipulatorContactEvents.Emplace(AsSMA, Time);

			if (!GetWorld())
			{
				// The episode finished, going further is futile
				return;
			}

			// Delay reseting the reach time, it might be a small disconnection with the hand
			if(!GetWorld()->GetTimerManager().IsTimerActive(ManipulatorContactDelayTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(ManipulatorContactDelayTimerHandle, this, &USLReachMonitor::DelayedManipulatorContactEndEventCallback,
					MaxPreGraspEventTimeGap * 1.2f, false);
			}
		}
		else
		{
			// 
			// It can happen, during the grasp there is a contact with the manipulator
			// when the contact ends after the grasp, this gets called and there are no items in ObjectsInContactWithManipulator
			//UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}

// Delayed call of setting finished event to check for possible concatenation of jittering events of the same type
void USLReachMonitor::DelayedManipulatorContactEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedManipulatorContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, reset the reach time
		if(CurrTime - EvItr->Time > MaxPreGraspEventTimeGap)
		{
			// Reset reach start in the candidate
			if(FSLTimeAndDist* TimeAndDist = CandidatesWithTimeAndDistance.Find(EvItr->Other))
			{
				// No new contact happened, remove and reset reach time
				if(ObjectsInContactWithManipulator.Remove(EvItr->Other) > 0)
				{
					//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s removed as object in contact with the manipulator.. (after delay, contact end time=%f)"),
					//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *EvItr->Other->GetName(), EvItr->Time);
					TimeAndDist->Get<ESLTimeAndDist::SLTime>() = GetWorld()->GetTimeSeconds();
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s is not in the contact list.. this should not happen.."),
						*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *EvItr->Other->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s is not in the candidates list.. this should not happen.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *EvItr->Other->GetName());
			}
			
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedManipulatorContactEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(ManipulatorContactDelayTimerHandle, this, &USLReachMonitor::DelayedManipulatorContactEndEventCallback,
			MaxPreGraspEventTimeGap * 1.2f, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLReachMonitor::SkipRecentManipulatorContactEndEventTime(AStaticMeshActor* Other, float StartTime)
{
	for (auto EvItr(RecentlyEndedManipulatorContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == Other)
		{
			// Check time difference between the previous and current event
			const float TimeGap = StartTime - EvItr->Time;
			if(TimeGap < MaxPreGraspEventTimeGap)
			{
				// Event will be concatenated
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedManipulatorContactEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(ManipulatorContactDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
