// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLBoneContactMonitor.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

// Ctor
USLBoneContactMonitor::USLBoneContactMonitor()
{
	// Default sphere radius
	InitSphereRadius(1.25f);

	//bMultiBodyOverlap = true;

	// Default group of the finger
	Group = ESLBoneContactGroup::A;
	bSnapToBone = true;
	bIsNotSkeletal = false;
	bIsGraspDetectionPaused = false;
	bDetectGrasps = false;
	bDetectContacts = false;

	bLogContactDebug = false;
	bLogGraspDebug = false;

#if WITH_EDITORONLY_DATA
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITORONLY_DATA

	// Set overlap area collision parameters
	SetCollisionParameters();
}

// Attach to bone 
void USLBoneContactMonitor::Init(bool bGrasp, bool bContact)
{
	if (!bGrasp && !bContact)
	{
		// Nothing to init
		return;
	}

	if (!bIsInit)
	{
		bDetectGrasps = bGrasp;
		bDetectContacts = bContact;

		// Remove any unset references in the array
		IgnoreList.Remove(nullptr);

		// Disable overlaps until start
		SetGenerateOverlapEvents(false);

		// Bind overlap events
		if (bDetectGrasps)
		{
			BindGraspOverlapCallbacks();
		}
		if (bDetectContacts)
		{
			BindContactOverlapCallbacks();
		}
		
		if (bIsNotSkeletal)
		{
			if (bVisualDebug)
			{
				SetHiddenInGame(false);
				bIsGraspDetectionPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
			}
			bIsInit = true;
		}
		else if(AttachToBone())
		{
			// Make sure the shape is attached to it bone
			if (bVisualDebug)
			{
				SetHiddenInGame(false);
				bIsGraspDetectionPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
			}
			bIsInit = true;			
		}
	}
}

// Start listening to overlaps 
void USLBoneContactMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (bVisualDebug)
		{
			bIsGraspDetectionPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
		}

		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Mark as started
		bIsStarted = true;
	}
}

// Attach component to bone
bool USLBoneContactMonitor::AttachToBone()
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

// PauseGraspDetection/continue listening to overlaps 
void USLBoneContactMonitor::PauseGraspDetection(bool bNewValue)
{	
	if (bNewValue != bIsGraspDetectionPaused)
	{
		bIsGraspDetectionPaused = bNewValue;

		if (bVisualDebug)
		{
			bIsGraspDetectionPaused ? SetColor(FColor::Yellow) : SetColor(FColor::Red);
		}

		if (bIsGraspDetectionPaused)
		{
			if (bLogGraspDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Pausing Grasp Detection: \t\t %s::%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *GetName());
			}

			// Broadcast ending of any active grasp related contacts
			for (auto& AC : ActiveContacts)
			{
				OnEndGraspBoneOverlap.Broadcast(AC, BoneName);
			}
			ActiveContacts.Empty();

			// Grasp check is paused, stop listening to grasp overlaps
			UnbindGraspOverlapCallbacks();
		}
		else
		{
			if (bLogGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Starting Grasp Detection: \t\t %s::%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *GetName());
			}

			// Grasp check is re-started, start listening to grasp overlaps
			TriggerInitialGraspOverlaps();
			BindGraspOverlapCallbacks();
		}		
	}
}

// Stop publishing overlap events
void USLBoneContactMonitor::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedGraspOverlapEvents)
		{
			OnEndGraspBoneOverlap.Broadcast(EvItr.Other, BoneName);
		}
		RecentlyEndedGraspOverlapEvents.Empty();

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedContactOverlapEvents)
		{
			OnEndContactBoneOverlap.Broadcast(EvItr.Other, BoneName);
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
void USLBoneContactMonitor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLBoneContactMonitor, bAttachButton))
	{
		if (!bIsNotSkeletal && AttachToBone())
		{
			if (!GetName().Equals(BoneName.ToString()))
			{
				if (Rename(*BoneName.ToString()))
				{
					// TODO find the 'refresh' to see the renaming
					//GetOwner()->MarkPackageDirty();
					//MarkPackageDirty();
				}
			}
			SetColor(FColor::Green);
		}
		else
		{
			SetColor(FColor::Red);
		}
		bAttachButton = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLBoneContactMonitor, BoneName))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLBoneContactMonitor, bIsNotSkeletal))
	{
		if (bIsNotSkeletal)
		{
			BoneName = NAME_None;
			FString NewName = Group == ESLBoneContactGroup::A ? FString("GraspOverlapGroupA") : FString("GraspOverlapGroupB");
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLBoneContactMonitor, IgnoreList))
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
		//		TArray<UActorComponent*> Comps = IA->GetComponentsByClass(USLBoneContactMonitor::StaticClass());
		//		for (const auto& C : Comps)
		//		{
		//			USLBoneContactMonitor* CastC = CastChecked<USLBoneContactMonitor>(C);
		//			if (Group == CastChecked<USLBoneContactMonitor>(C)->Group)
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLBoneContactMonitor, bVisualDebug))
	{
		SetHiddenInGame(bVisualDebug);
	}
}
#endif // WITH_EDITOR

// Set collision parameters such as object name and collision responses
void USLBoneContactMonitor::SetCollisionParameters()
{
	SetCollisionProfileName("SLManipulatorContact");
	//SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
	SetAllUseCCD(true);
}

// Set debug color
void USLBoneContactMonitor::SetColor(FColor Color)
{
	if (ShapeColor != Color)
	{
		ShapeColor = Color;
		MarkRenderStateDirty();
	}
}

// Bind grasp related overlaps
void USLBoneContactMonitor::BindGraspOverlapCallbacks()
{
	if(OnComponentBeginOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnGraspOverlapBegin)
		|| OnComponentEndOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnGraspOverlapEnd))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs Grasp callback already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
			return;
	}
	OnComponentBeginOverlap.AddDynamic(this, &USLBoneContactMonitor::OnGraspOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &USLBoneContactMonitor::OnGraspOverlapEnd);
}

// Remove grasp related overlap callbacks
void USLBoneContactMonitor::UnbindGraspOverlapCallbacks()
{
	if (!OnComponentBeginOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnGraspOverlapBegin)
		|| !OnComponentEndOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnGraspOverlapEnd))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs Grasp callback not bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
		return;
	}
	OnComponentBeginOverlap.RemoveDynamic(this, &USLBoneContactMonitor::OnGraspOverlapBegin);
	OnComponentEndOverlap.RemoveDynamic(this, &USLBoneContactMonitor::OnGraspOverlapEnd);
}

/* Grasp related */
// Publish currently grasp related overlapping components
void USLBoneContactMonitor::TriggerInitialGraspOverlaps()
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
void USLBoneContactMonitor::OnGraspOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s self overlap, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not annotated, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if on the ignore list
	if (IgnoreList.Contains(OtherActor))
	{
		return;
	}

	// Make sure actir is a static mesh actor
	if (!OtherActor->IsA(AStaticMeshActor::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not a static mesh, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if it is a new event, or a concatenation with a previous one, either way, there is a new active contact
	ActiveContacts.Emplace(OtherIndividual);
	if(!IsAJitterGrasp(OtherIndividual, GetWorld()->GetTimeSeconds()))
	{
		if (bLogGraspDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Grasp Contact Started ( !!! broadcast !!! ): \t\t %s::%s->%s::%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetName(), *OtherActor->GetName(), *OtherComp->GetName());
		}
		OnBeginGraspBoneOverlap.Broadcast(OtherIndividual, BoneName);
	}
	else if (bLogGraspDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Grasp Contact Started (jitter grasp - cancelled): \t\t %s::%s->%s::%s;"),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetOwner()->GetName(), *GetName(), *OtherActor->GetName(), *OtherComp->GetName());
	}

	if (bVisualDebug)
	{
		SetColor(FColor::Green);
	}
}

// Called on overlap end events
void USLBoneContactMonitor::OnGraspOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s self overlap, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not annotated, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if on the ignore list
	if (IgnoreList.Contains(OtherActor))
	{
		return;
	}

	// Make sure actir is a static mesh actor
	if (!OtherActor->IsA(AStaticMeshActor::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not a static mesh, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if grasp is registered
	if (ActiveContacts.Remove(OtherIndividual) > 0)
	{
		if (bLogGraspDebug)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Grasp Contact Ended (jitter check started): \t\t %s->%s::%s::%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetName(), *OtherActor->GetName(), *OtherComp->GetName());
		}

		// Grasp overlap ended ended
		RecentlyEndedGraspOverlapEvents.Emplace(FSLBoneContactEndEvent(
			OtherIndividual, GetWorld()->GetTimeSeconds()));
			
		// Delay publishing for a while, in case the new event is of the same type and should be concatenated
		if(!GetWorld()->GetTimerManager().IsTimerActive(GraspDelayTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, 
				&USLBoneContactMonitor::DelayedGraspOverlapEndEventCallback, ConcatenateIfSmaller*1.1f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
	}

	if (bVisualDebug)
	{
		if (ActiveContacts.Num() == 0)
		{
			SetColor(FColor::Red);
		}
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLBoneContactMonitor::DelayedGraspOverlapEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedGraspOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Timestamp > ConcatenateIfSmaller)
		{
			if (bLogGraspDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Grasp Contact Ended ( !!! broadcast !!! with delay): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *EvItr->Other->GetParentActor()->GetName());
			}

			// Broadcast delayed event
			OnEndGraspBoneOverlap.Broadcast(EvItr->Other, BoneName);
			
			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedGraspOverlapEvents.Num() > 0)
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle,
			this, &USLBoneContactMonitor::DelayedGraspOverlapEndEventCallback, DelayValue, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLBoneContactMonitor::IsAJitterGrasp(USLBaseIndividual* OtherIndividual, float StartTime)
{
	for (auto EvItr(RecentlyEndedGraspOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == OtherIndividual)
		{
			// Check time difference
			if(StartTime - EvItr->Timestamp < ConcatenateIfSmaller)
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

// Bind contact related overlaps
void USLBoneContactMonitor::BindContactOverlapCallbacks()
{
	if (OnComponentBeginOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnContactOverlapBegin)
		|| OnComponentEndOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnContactOverlapEnd))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs Contact callback already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
		return;
	}
	OnComponentBeginOverlap.AddDynamic(this, &USLBoneContactMonitor::OnContactOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &USLBoneContactMonitor::OnContactOverlapEnd);
}

// Remove contact related overlap callbacks
void USLBoneContactMonitor::UnbindContactOverlapCallbacks()
{
	if (!OnComponentBeginOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnContactOverlapBegin)
		|| !OnComponentEndOverlap.IsAlreadyBound(this, &USLBoneContactMonitor::OnContactOverlapEnd))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs Contact callback not bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
		return;
	}
	OnComponentBeginOverlap.RemoveDynamic(this, &USLBoneContactMonitor::OnContactOverlapBegin);
	OnComponentEndOverlap.RemoveDynamic(this, &USLBoneContactMonitor::OnContactOverlapEnd);
}


/* Contact related*/
// Publish currently contact related overlapping components
void USLBoneContactMonitor::TriggerInitialContactOverlaps()
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
void USLBoneContactMonitor::OnContactOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s self overlap, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not annotated, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if on the ignore list
	if (IgnoreList.Contains(OtherActor))
	{
		return;
	}

	// Make sure actir is a static mesh actor
	if (!OtherActor->IsA(AStaticMeshActor::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not a static mesh, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check for jitters in the events
	if(!IsAJitterContact(OtherIndividual, GetWorld()->GetTimeSeconds()))
	{
		if (bLogContactDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Contact Started ( !!! broadcast !!! ): \t\t %s::%s->%s::%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetName(), *OtherActor->GetName(), *OtherComp->GetName());
		}

		// Broadcast the start of the contact event
		OnBeginContactBoneOverlap.Broadcast(OtherIndividual, BoneName);
	}
	else if (bLogContactDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Contact Started (jitter contact - cancelled): \t\t %s::%s->%s::%s;"),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetOwner()->GetName(), *GetName(), *OtherActor->GetName(), *OtherComp->GetName());
	}
}

// Called on overlap end events
void USLBoneContactMonitor::OnContactOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s self overlap, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not annotated, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}

	// Check if on the ignore list
	if (IgnoreList.Contains(OtherActor))
	{
		return;
	}

	// Make sure actir is a static mesh actor
	if(!OtherActor->IsA(AStaticMeshActor::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not a static mesh, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherActor->GetName());
		return;
	}


	if (bLogContactDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4fs \t\t Contact Ended (jitter check started): \t\t %s::%s->%s::%s;"),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetOwner()->GetName(), *GetName(), *OtherActor->GetName(), *OtherComp->GetName());
	}

	// Contact overlap ended ended
	RecentlyEndedContactOverlapEvents.Emplace(FSLBoneContactEndEvent(
		OtherIndividual, GetWorld()->GetTimeSeconds()));
		
	// Delay publishing for a while, in case the new event is of the same type and should be concatenated
	if(!GetWorld()->GetTimerManager().IsTimerActive(ContactDelayTimerHandle))
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, 
			this, &USLBoneContactMonitor::DelayContactEndCallback, DelayValue, false);
	}

}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLBoneContactMonitor::DelayContactEndCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedContactOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Timestamp > ConcatenateIfSmaller)
		{
			if (bLogContactDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4fs \t\t Contact Ended ( !!! broadcast !!! with delay): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *EvItr->Other->GetParentActor()->GetName());
			}

			// Broadcast delayed event
			OnEndContactBoneOverlap.Broadcast(EvItr->Other, BoneName);
			
			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedContactOverlapEvents.Num() > 0)
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle,
			this, &USLBoneContactMonitor::DelayContactEndCallback, DelayValue, false);
	}
}

// Check if this begin event happened right after the previous one ended,
// if so remove it from the array, and cancel publishing the begin event
bool USLBoneContactMonitor::IsAJitterContact(USLBaseIndividual* OtherIndividual, float StartTime)
{
	for (auto EvItr(RecentlyEndedContactOverlapEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == OtherIndividual)
		{
			// Check time difference
			if(StartTime - EvItr->Timestamp < ConcatenateIfSmaller)
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
