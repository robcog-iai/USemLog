// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLManipulatorBoneContactMonitor.h"
#include "Individuals/SLIndividualUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

// Ctor
USLManipulatorBoneContactMonitor::USLManipulatorBoneContactMonitor()
{
	// Default sphere radius
	InitSphereRadius(1.25f);

	//bMultiBodyOverlap = true;

	// Default group of the finger
	Group = ESLManipulatorContactMonitorGroup::A;
	bSnapToBone = true;
	bIsNotSkeletal = false;
	bGraspPaused = false;
	bDetectGrasps = false;
	bDetectContacts = false;

#if WITH_EDITORONLY_DATA
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITORONLY_DATA

	// Set overlap area collision parameters
	SetCollisionParameters();
}

// Attach to bone 
bool USLManipulatorBoneContactMonitor::Init(bool bGrasp, bool bContact)
{
	if (!bIsInit)
	{
		bDetectGrasps = bGrasp;
		bDetectContacts = bContact;
		// Remove any unset references in the array
		IgnoreList.Remove(nullptr);

		// Make sure the shape is attached to it bone
		if (!bIsNotSkeletal)
		{
			if (AttachToBone())
			{
				if (bVisualDebug)
				{
					SetHiddenInGame(false);
					bGraspPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
				}
				bIsInit = true;
				return true;
			}
			else 
			{
				return false;
			}
		}
		else
		{
			if (bVisualDebug)
			{
				SetHiddenInGame(false);
				bGraspPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
			}
			bIsInit = true;
			return true;
		}
	}
	return true;
}

// Start listening to overlaps 
void USLManipulatorBoneContactMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (bVisualDebug)
		{
			bGraspPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
		}

		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Bind overlap events
		if (bDetectGrasps)
		{
			TriggerInitialGraspOverlaps(); // TODO this was commented out, any reason?
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorBoneContactMonitor::OnGraspOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorBoneContactMonitor::OnGraspOverlapEnd);
		}

		if (bDetectContacts)
		{
			TriggerInitialContactOverlaps();
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorBoneContactMonitor::OnContactOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorBoneContactMonitor::OnContactOverlapEnd);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// PauseGraspDetection/continue listening to overlaps 
void USLManipulatorBoneContactMonitor::PauseGrasp(bool bInPause)
{
	if (bInPause != bGraspPaused)
	{
		bGraspPaused = bInPause;

		if (bVisualDebug)
		{
			bGraspPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
		}

		if (bInPause)
		{
			// Broadcast ending of any active grasp related contacts
			for (auto& AC : ActiveContacts)
			{
				OnEndManipulatorGraspOverlap.Broadcast(AC);
			}
			ActiveContacts.Empty();

			// Remove grasp related overlap bindings
			OnComponentBeginOverlap.RemoveDynamic(this, &USLManipulatorBoneContactMonitor::OnGraspOverlapBegin);
			OnComponentEndOverlap.RemoveDynamic(this, &USLManipulatorBoneContactMonitor::OnGraspOverlapEnd);
		}
		else
		{
			TriggerInitialGraspOverlaps();
			// Re-bind the grasp related overlap bindings
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorBoneContactMonitor::OnGraspOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorBoneContactMonitor::OnGraspOverlapEnd);
		}
		
	}
}

// Stop publishing overlap events
void USLManipulatorBoneContactMonitor::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedGraspOverlapEvents)
		{
			OnEndManipulatorGraspOverlap.Broadcast(EvItr.Other);
		}
		RecentlyEndedGraspOverlapEvents.Empty();

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedContactOverlapEvents)
		{
			OnEndManipulatorContactOverlap.Broadcast(EvItr.Other);
		}
		RecentlyEndedContactOverlapEvents.Empty();

		SetGenerateOverlapEvents(false);
		
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLManipulatorBoneContactMonitor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorBoneContactMonitor, bAttachButton))
	{
		if (!bIsNotSkeletal && AttachToBone())
		{
			if (Rename(*BoneName.ToString()))
			{
				// TODO find the 'refresh' to see the renaming
				//GetOwner()->MarkPackageDirty();
				//MarkPackageDirty();
			}
			SetColor(FColor::Green);
		}
		else
		{
			SetColor(FColor::Red);
		}
		bAttachButton = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorBoneContactMonitor, BoneName))
	{
		if (!bIsNotSkeletal && AttachToBone())
		{
			if (Rename(*BoneName.ToString()))
			{
			}
			SetColor(FColor::Green);
		}
		else
		{
			SetColor(FColor::Red);
		}
		bAttachButton = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorBoneContactMonitor, bIsNotSkeletal))
	{
		if (bIsNotSkeletal)
		{
			BoneName = NAME_None;
			FString NewName = Group == ESLManipulatorContactMonitorGroup::A ? FString("GraspOverlapGroupA") : FString("GraspOverlapGroupB");
			NewName.Append(FString::FromInt(GetUniqueID()));
			if (Rename(*NewName))
			{
				// TODO find the 'refresh' to see the renaming
				//GetOwner()->MarkPackageDirty();
				//MarkPackageDirty();
			}
		}
		else
		{
			IgnoreList.Empty();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorBoneContactMonitor, IgnoreList))
	{
		if (IgnoreList.Num() > 0)
		{
			SetColor(FColor::Green);
		}
		//if (IgnoreList.Num() > 0)
		//{
		//	bool bDifferntGroups = true;
		//	for (const auto& IA : IgnoreList)
		//	{
		//		TArray<UActorComponent*> Comps = IA->GetComponentsByClass(USLManipulatorBoneContactMonitor::StaticClass());
		//		for (const auto& C : Comps)
		//		{
		//			USLManipulatorBoneContactMonitor* CastC = CastChecked<USLManipulatorBoneContactMonitor>(C);
		//			if (Group == CastChecked<USLManipulatorBoneContactMonitor>(C)->Group)
		//			{
		//				bDifferntGroups = false;
		//				SetColor(FColor::Red);
		//				UE_LOG(LogTemp, Error, TEXT("%s::%d Grasp overlap component from ignored list has the same group (these should be different).."),
		//					*FString(__func__), __LINE__);
		//				break;
		//			}
		//		}
		//	}
		//	if (bDifferntGroups)
		//	{
		//		SetColor(FColor::Green);
		//	}
		//}
		//else
		//{
		//	SetColor(FColor::Red);
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d Ignore list is empty.."),
		//		*FString(__func__), __LINE__);
		//}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorBoneContactMonitor, bVisualDebug))
	{
		SetHiddenInGame(bVisualDebug);
	}
}
#endif // WITH_EDITOR

// Set collision parameters such as object name and collision responses
void USLManipulatorBoneContactMonitor::SetCollisionParameters()
{
	SetCollisionProfileName("SLManipulatorContact");
	//SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}

// Attach component to bone
bool USLManipulatorBoneContactMonitor::AttachToBone()
{
	// Check if owner is a skeletal actor
	if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		// Get the skeletal mesh component
		if (USkeletalMeshComponent* SMC = SkelAct->GetSkeletalMeshComponent())
		{
			if (SMC->GetBoneIndex(BoneName) != INDEX_NONE)
			{
				FAttachmentTransformRules AttachmentRule = bSnapToBone ? FAttachmentTransformRules::SnapToTargetIncludingScale
					: FAttachmentTransformRules::KeepRelativeTransform;

				if (AttachToComponent(SMC, AttachmentRule, BoneName))
				{
					//UE_LOG(LogTemp, Warning, TEXT("%s::%d Attached component %s to the bone %s"),
					//	*FString(__func__), __LINE__, *GetName(), *BoneName.ToString());
					return true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find bone %s for component %s"),
					*FString(__func__), __LINE__, *BoneName.ToString(), *GetName());
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not attach component %s to the bone %s"),
		*FString(__func__), __LINE__, *GetName(), *BoneName.ToString());
	return false;
}

// Set debug color
void USLManipulatorBoneContactMonitor::SetColor(FColor Color)
{
	if (ShapeColor != Color)
	{
		ShapeColor = Color;
		MarkRenderStateDirty();
	}
}

/* Grasp related */
// Publish currently grasp related overlapping components
void USLManipulatorBoneContactMonitor::TriggerInitialGraspOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		OnGraspOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Called on overlap begin events
void USLManipulatorBoneContactMonitor::OnGraspOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		// Check if it is a new event, or a concatenation with a previous one, either way, there is a new active contact
		ActiveContacts.Emplace(OtherIndividual);
		if(!SkipRecentGraspOverlapEndEventBroadcast(OtherIndividual, GetWorld()->GetTimeSeconds()))
		{
			OnBeginManipulatorGraspOverlap.Broadcast(OtherIndividual);
		}

		if (bVisualDebug)
		{
			SetColor(FColor::Green);
		}
	}
}

// Called on overlap end events
void USLManipulatorBoneContactMonitor::OnGraspOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		if (ActiveContacts.Remove(OtherIndividual) > 0)
		{
			// Grasp overlap ended ended
			RecentlyEndedGraspOverlapEvents.Emplace(FSLManipulatorContactMonitorEndEvent(
				OtherIndividual, GetWorld()->GetTimeSeconds()));
			
			// Delay publishing for a while, in case the new event is of the same type and should be concatenated
			if(!GetWorld()->GetTimerManager().IsTimerActive(GraspDelayTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, 
					&USLManipulatorBoneContactMonitor::DelayedGraspOverlapEndEventCallback, MaxOverlapEventTimeGap*1.1f, false);
			}
		}

		if (bVisualDebug)
		{
			if (ActiveContacts.Num() == 0)
			{
				SetColor(FColor::Red);
			}
		}
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorBoneContactMonitor::DelayedGraspOverlapEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedGraspOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Timestamp > MaxOverlapEventTimeGap)
		{
			// Broadcast delayed event
			OnEndManipulatorGraspOverlap.Broadcast(EvItr->Other);
			
			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedGraspOverlapEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, &USLManipulatorBoneContactMonitor::DelayedGraspOverlapEndEventCallback,
			MaxOverlapEventTimeGap*1.1f, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorBoneContactMonitor::SkipRecentGraspOverlapEndEventBroadcast(USLBaseIndividual* OtherIndividual, float StartTime)
{
	for (auto EvItr(RecentlyEndedGraspOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == OtherIndividual)
		{
			// Check time difference
			if(StartTime - EvItr->Timestamp < MaxOverlapEventTimeGap)
			{
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedGraspOverlapEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(GraspDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}


/* Contact related*/
// Publish currently contact related overlapping components
void USLManipulatorBoneContactMonitor::TriggerInitialContactOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		OnContactOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Called on overlap begin events
void USLManipulatorBoneContactMonitor::OnContactOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		if(!SkipRecentContactOverlapEndEventBroadcast(OtherIndividual, GetWorld()->GetTimeSeconds()))
		{
			OnBeginManipulatorContactOverlap.Broadcast(OtherIndividual);
		}
	}
}

// Called on overlap end events
void USLManipulatorBoneContactMonitor::OnContactOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		// Contact overlap ended ended
		RecentlyEndedContactOverlapEvents.Emplace(FSLManipulatorContactMonitorEndEvent(
			OtherIndividual, GetWorld()->GetTimeSeconds()));
		
		// Delay publishing for a while, in case the new event is of the same type and should be concatenated
		if(!GetWorld()->GetTimerManager().IsTimerActive(ContactDelayTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this, 
				&USLManipulatorBoneContactMonitor::DelayedContactOverlapEndEventCallback, MaxOverlapEventTimeGap*1.1f, false);
		}
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorBoneContactMonitor::DelayedContactOverlapEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedContactOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Timestamp > MaxOverlapEventTimeGap)
		{
			// Broadcast delayed event
			OnEndManipulatorContactOverlap.Broadcast(EvItr->Other);
			
			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedContactOverlapEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this, &USLManipulatorBoneContactMonitor::DelayedContactOverlapEndEventCallback,
			MaxOverlapEventTimeGap*1.1f, false);
	}
}

// Check if this begin event happened right after the previous one ended
// if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorBoneContactMonitor::SkipRecentContactOverlapEndEventBroadcast(USLBaseIndividual* OtherIndividual, float StartTime)
{
	for (auto EvItr(RecentlyEndedContactOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == OtherIndividual)
		{
			// Check time difference
			if(StartTime - EvItr->Timestamp < MaxOverlapEventTimeGap)
			{
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedContactOverlapEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(ContactDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
