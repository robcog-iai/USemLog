// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManipulatorListener.h"
#include "SLManipulatorOverlapSphere.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLEntitiesManager.h"
#include "GameFramework/PlayerController.h"
#if SL_WITH_MC_GRASP
#include "MCGraspAnimController.h"
#endif // SL_WITH_MC_GRASP


// Sets default values for this component's properties
USLManipulatorListener::USLManipulatorListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bGraspIsDirty = true;
	bIsPaused = false;
	InputAxisName = "LeftGrasp";
	bIsNotSkeletal = false;
	UnPauseTriggerVal = 0.5;
	
#if WITH_EDITOR
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITOR

	ActiveGraspType = "Default";
}

// Dtor
USLManipulatorListener::~USLManipulatorListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLManipulatorListener::Init(bool bInDetectGrasps, bool bInDetectContacts)
{
	if (!bIsInit)
	{
		bDetectGrasps = bInDetectGrasps;
		bDetectContacts = bInDetectContacts;
		
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

		// Remove any unset references in the array
		Fingers.Remove(nullptr);

#if SL_WITH_MC_GRASP
		// Subscribe to grasp type changes
		 SubscribeToGraspTypeChanges();
#endif // SL_WITH_MC_GRASP
		

		// True if each group has at least one bone overlap
		if (LoadOverlapGroups())
		{
			for (auto BoneOverlap : GroupA)
			{
				BoneOverlap->Init(bDetectGrasps, bDetectContacts);
			}
			for (auto BoneOverlap : GroupB)
			{
				BoneOverlap->Init(bDetectGrasps, bDetectContacts);
			}

			bIsInit = true;
			return true;
		}
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLManipulatorListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Bind grasp trigger input and update check functions
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (UInputComponent* IC = PC->InputComponent)
			{
				IC->BindAxis(InputAxisName, this, &USLManipulatorListener::GraspInputAxisCallback);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d No Input Component found.."), *FString(__func__), __LINE__);
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No Player controller found.."), *FString(__func__), __LINE__);
			return;
		}

		// Start listening on the bone overlaps
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->Start();
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnBeginOverlapContact);
				BoneOverlap->OnEndSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnEndOverlapContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnBeginOverlapGroupAGrasp);
				BoneOverlap->OnEndSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnEndOverlapGroupAGrasp);
			}
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Start();
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnBeginOverlapContact);
				BoneOverlap->OnEndSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnEndOverlapContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnBeginOverlapGroupBGrasp);
				BoneOverlap->OnEndSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnEndOverlapGroupBGrasp);
			}
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue grasp detection
void USLManipulatorListener::PauseGraspDetection(bool bInPause)
{
	if (bInPause != bIsPaused)
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->PauseGrasp(bInPause);
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->PauseGrasp(bInPause);
		}
		bIsPaused = bInPause;


		// Clear sets
		if (bInPause)
		{
			for (const auto& Obj : GraspedObjects)
			{
				OnEndManipulatorGrasp.Broadcast(SemanticOwner, Obj, GetWorld()->GetTimeSeconds());
			}
			GraspedObjects.Empty();
			SetA.Empty();
			SetB.Empty();
		}
	}
}

// Stop publishing grasp events
void USLManipulatorListener::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->Finish();
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Finish();
		}
		
		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedGraspEvents)
		{
			OnEndManipulatorGrasp.Broadcast(SemanticOwner, EvItr.OtherActor, EvItr.Time);
		}
		RecentlyEndedGraspEvents.Empty();

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedContactEvents)
		{
			OnEndManipulatorContact.Broadcast(SemanticOwner, EvItr.OtherItem, EvItr.Time);
		}
		RecentlyEndedContactEvents.Empty();

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLManipulatorListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorListener, HandType))
	{
		if (HandType == ESLGraspHandType::Left)
		{
			InputAxisName = "LeftGrasp";
		}
		else if (HandType == ESLGraspHandType::Right)
		{
			InputAxisName = "RightGrasp";
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorListener, bIsNotSkeletal))
	{
		Fingers.Empty();
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLManipulatorListener::LoadOverlapGroups()
{
	// Lambda to check grasp overlap components of owner and add them to their groups
	auto GetOverlapComponentsLambda = [this](AActor* Owner)
	{
		TArray<UActorComponent*> GraspOverlaps = Owner->GetComponentsByClass(USLManipulatorOverlapSphere::StaticClass());
		for (UActorComponent* GraspOverlapComp : GraspOverlaps)
		{
			USLManipulatorOverlapSphere* GraspOverlap = CastChecked<USLManipulatorOverlapSphere>(GraspOverlapComp);
			if (GraspOverlap->Group == ESLManipulatorOverlapGroup::A)
			{
				GroupA.Add(GraspOverlap);
			}
			else if (GraspOverlap->Group == ESLManipulatorOverlapGroup::B)
			{
				GroupB.Add(GraspOverlap);
			}
		}
	};

	if (bIsNotSkeletal)
	{
		for (const auto& F : Fingers)
		{
			GetOverlapComponentsLambda(F);
		}
	}
	else 
	{
		GetOverlapComponentsLambda(GetOwner());
	}

	// Check if at least one valid overlap shape is in each group
	if (GroupA.Num() == 0 || GroupB.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d One of the grasp groups is empty grasp detection disabled."), *FString(__func__), __LINE__);
		bDetectGrasps = false;
	}
	return true;
}

/* Begin grasp related */
#if SL_WITH_MC_GRASP
// Subscribe to grasp type changes
bool USLManipulatorListener::SubscribeToGraspTypeChanges()
{
	if (UMCGraspAnimController* Sibling = CastChecked<UMCGraspAnimController>(
		GetOwner()->GetComponentByClass(UMCGraspAnimController::StaticClass())))
	{
		Sibling->OnGraspType.AddUObject(this, &USLManipulatorListener::OnGraspType);
		return true;
	}
	return false;
}

// Callback on grasp type change
void USLManipulatorListener::OnGraspType(const FString& Type)
{
	ActiveGraspType = Type;
	ActiveGraspType.RemoveFromStart("GA_");
	ActiveGraspType.RemoveFromEnd("_Left");
	ActiveGraspType.RemoveFromEnd("_Right");
	ActiveGraspType.Append("Grasp");
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d ActiveGraspType=%s"), *FString(__func__), __LINE__, *ActiveGraspType);
}
#endif // SL_WITH_MC_GRASP

// Check if the grasp trigger is active
void USLManipulatorListener::GraspInputAxisCallback(float Value)
{
	if (Value >= UnPauseTriggerVal)
	{
		PauseGraspDetection(false);
	}
	else
	{	
		PauseGraspDetection(true);
	}
}

// Process beginning of grasp in group A
void USLManipulatorListener::OnBeginOverlapGroupAGrasp(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetA.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupB.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}
}

// Process beginning of grasp in group B
void USLManipulatorListener::OnBeginOverlapGroupBGrasp(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetB.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupA.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}
}

// Process ending of contact in group A
void USLManipulatorListener::OnEndOverlapGroupAGrasp(AActor* OtherActor)
{
	if (SetA.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			EndGrasp(OtherActor);
		}
	}
}

// Process ending of contact in group B
void USLManipulatorListener::OnEndOverlapGroupBGrasp(AActor* OtherActor)
{
	if (SetB.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			EndGrasp(OtherActor);
		}
	}
}

// Check for grasping state
void USLManipulatorListener::CheckGraspState()
{
	for (const auto& Obj : SetA.Intersect(SetB))
	{
		if (!GraspedObjects.Contains(Obj))
		{
			BeginGrasp(Obj);
		}
	}
}

// A grasp has started
void USLManipulatorListener::BeginGrasp(AActor* OtherActor)
{
	// Check if it is a new grasp event, or a concatenation with a previous one, either way, there is a new grasp
	GraspedObjects.AddUnique(OtherActor);
	if(!SkipRecentGraspEndEventBroadcast(OtherActor, GetWorld()->GetTimeSeconds()))
	{
		OnBeginManipulatorGrasp.Broadcast(SemanticOwner, OtherActor, GetWorld()->GetTimeSeconds(), ActiveGraspType);
	}
}

// A grasp has ended
void USLManipulatorListener::EndGrasp(AActor* OtherActor)
{
	//if (GraspedObjects.Contains(OtherActor))
	if (GraspedObjects.Remove(OtherActor) > 0)
	{
		// Grasp ended
		RecentlyEndedGraspEvents.Emplace(FSLGraspEndEvent(OtherActor, GetWorld()->GetTimeSeconds()));
		
		// Delay publishing for a while, in case the new event is of the same type and should be concatenated
		if(!GetWorld()->GetTimerManager().IsTimerActive(GraspDelayTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, &USLManipulatorListener::DelayedGraspEndEventCallback,
				MaxGraspEventTimeGap*2.f, false);
		}
	}
}

// End all grasps
void USLManipulatorListener::EndAllGrasps()
{
	for (const auto& Obj : GraspedObjects)
	{
		OnEndManipulatorGrasp.Broadcast(SemanticOwner, Obj, GetWorld()->GetTimeSeconds());
	}
	GraspedObjects.Empty();
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorListener::DelayedGraspEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > MaxGraspEventTimeGap)
		{
			//if(GraspedObjects.Remove(EvItr->OtherActor)>0)
			//{
			//	OnEndManipulatorGrasp.Broadcast(SemanticOwner, EvItr->OtherActor, EvItr->Time);
			//}
			//else
			//{
			//	UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
			//}

			// Broadcast delayed event
			OnEndManipulatorGrasp.Broadcast(SemanticOwner, EvItr->OtherActor, EvItr->Time);
			
			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedGraspEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, &USLManipulatorListener::DelayedGraspEndEventCallback,
			MaxGraspEventTimeGap*2.f, false);
	}
}

// Check if this begin event happened right after the previous one ended
// if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorListener::SkipRecentGraspEndEventBroadcast(AActor* OtherActor, float StartTime)
{
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->OtherActor == OtherActor)
		{
			// Check time difference
			if(StartTime - EvItr->Time < MaxGraspEventTimeGap)
			{
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedGraspEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(GraspDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
/* End grasp related */


/* Begin contact related */
// Process beginning of contact
void USLManipulatorListener::OnBeginOverlapContact(AActor* OtherActor)
{
	if (FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(OtherActor))
	{
		if (int32* NumContacts = ObjectsInContact.Find(OtherActor))
		{
			(*NumContacts)++;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d >>>>>>> [%f] INC Finger=%d; Other=%s;"),
				*FString(__func__), __LINE__,
				GetWorld()->GetTimeSeconds(),
				*NumContacts,
				*OtherActor->GetName());
		}
		else
		{
			// Check if it is a new contact event, or a concatenation with a previous one, either way, there is a new contact
			ObjectsInContact.Add(OtherActor, 1);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d >>>>>>> [%f] FIRST Finger=%d; Other=%s; Contacts=%d;"),
					*FString(__func__), __LINE__, 
					GetWorld()->GetTimeSeconds(),
					ObjectsInContact[OtherActor],
					*OtherActor->GetName(),
					ObjectsInContact.Num());
			const float CurrTime = GetWorld()->GetTimeSeconds();
			if(!SkipRecentContactEndEventBroadcast(*OtherItem, CurrTime))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d >>>>>>> [%f] \t\t\t\t\t\t @@@@@@@@ BCAST START!"),
					*FString(__func__), __LINE__)
				// Broadcast begin of semantic overlap event
				OnBeginManipulatorContact.Broadcast(FSLContactResult(SemanticOwner, *OtherItem, CurrTime, false));
			}
		}
	}
}

// Process ending of contact
void USLManipulatorListener::OnEndOverlapContact(AActor* OtherActor)
{
	if (FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(OtherActor))
	{
		if (int32* NumContacts = ObjectsInContact.Find(OtherActor))
		{
			(*NumContacts)--;
			UE_LOG(LogTemp, Error, TEXT("%s::%d <<<<<<<< [%f]  DEC Finger=%d; Other=%s;"),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),*NumContacts, *OtherActor->GetName());
			
			if ((*NumContacts) < 1)
			{
				// Remove contact object
				ObjectsInContact.Remove(OtherActor);
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t CONTACT ENDED; Add to delay! Contacts=%d"),
					*FString(__func__), __LINE__, ObjectsInContact.Num());
				
				// Manipulator contact ended
				RecentlyEndedContactEvents.Emplace(FSLContactEndEvent(*OtherItem, GetWorld()->GetTimeSeconds()));
				
				// Delay publishing for a while, in case the new event is of the same type and should be concatenated
				if(!GetWorld()->GetTimerManager().IsTimerActive(ContactDelayTimerHandle))
				{
					GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this, &USLManipulatorListener::DelayedContactEndEventCallback,
						MaxContactEventTimeGap * 2.f, false);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}

// End all contacts
void USLManipulatorListener::EndAllContacts()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d **** %f  ObjectsInContactNum=%d"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), RecentlyEndedContactEvents.Num());

	for (const auto& Obj : ObjectsInContact)
	{
		if (FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Obj.Key))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d **** %f \t\t ObjectsInContact BCAST=%s"),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *OtherItem->Obj->GetName());
			
			OnEndManipulatorContact.Broadcast(SemanticOwner, *OtherItem, GetWorld()->GetTimeSeconds());
		}
	}
	ObjectsInContact.Empty();
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorListener::DelayedContactEndEventCallback()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d ~~~~~~~~~~ [%f] LOOP RecentlyEndedContactEvents=%d"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), RecentlyEndedContactEvents.Num());
	
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > MaxContactEventTimeGap)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Enough time has passed [%f/%f] \t\t\t @@@@@@@@@@ BCAST END!"),
				*FString(__func__), __LINE__, CurrTime - EvItr->Time, MaxContactEventTimeGap);
			OnEndManipulatorContact.Broadcast(SemanticOwner, EvItr->OtherItem, EvItr->Time);
			EvItr.RemoveCurrent();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t NOT enough time has passed [%f/%f] --> RE-SPIN"),
				*FString(__func__), __LINE__, CurrTime - EvItr->Time, MaxContactEventTimeGap);
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedContactEvents.Num() > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t Confirm RE-SPIN!"), *FString(__func__), __LINE__);
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this, &USLManipulatorListener::DelayedContactEndEventCallback,
			MaxContactEventTimeGap*2.f, false);
	}
}

// Check if this begin event happened right after the previous one ended
// if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorListener::SkipRecentContactEndEventBroadcast(const FSLEntity& OtherItem, float StartTime)
{
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->OtherItem.EqualsFast(OtherItem))
		{
			// Check time difference between the previous and current event
			const float TimeGap = StartTime - EvItr->Time;
			if(TimeGap < MaxContactEventTimeGap)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d >__< \t\t [%f < %f] Concatenate event, abort new event.."),
					*FString(__func__), __LINE__, TimeGap, MaxContactEventTimeGap);
				
				// Event will be concatenated
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedContactEvents.Num() == 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Last event, delay timer cleared"), *FString(__func__), __LINE__);
					GetWorld()->GetTimerManager().ClearTimer(ContactDelayTimerHandle);
				}
				return true;
			}
			UE_LOG(LogTemp, Warning, TEXT("%s::%d >____________< \t\t [%f > %f] DON'T concatenate, start new event.."),
				*FString(__func__), __LINE__, TimeGap, MaxContactEventTimeGap);
		}
	}
	return false;
}
/* End contact related */
