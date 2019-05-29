// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManipulatorOverlapShape.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
USLManipulatorOverlapShape::USLManipulatorOverlapShape()
{
	// Default sphere radius
	InitSphereRadius(1.25f);

	//bMultiBodyOverlap = true;

	// Default group of the finger
	Group = ESLManipulatorOverlapGroup::A;
	bVisualDebug = true;
	bSnapToBone = true;
	bIsNotSkeletal = false;
	bIsPaused = false;
	bDetectGrasps = false;
	bDetectContacts = false;

#if WITH_EDITOR
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITOR
}

// Attach to bone 
bool USLManipulatorOverlapShape::Init(bool bGrasp, bool bContact)
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
					SetColor(FColor::Yellow);
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
				SetColor(FColor::Yellow);
			}
			bIsInit = true;
			return true;
		}
	}
	return true;
}

// Start listening to overlaps 
void USLManipulatorOverlapShape::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (bVisualDebug)
		{
			SetColor(FColor::Red);
		}

		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Broadcast currently overlapping components
		TriggerInitialOverlaps();

		// Bind overlap events
		if (bDetectGrasps)
		{
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorOverlapShape::OnGraspOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorOverlapShape::OnGraspOverlapEnd);
		}

		if (bDetectContacts)
		{
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorOverlapShape::OnContactOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorOverlapShape::OnContactOverlapEnd);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue listening to overlaps 
void USLManipulatorOverlapShape::PauseGrasp(bool bInPause)
{
	if (bInPause != bIsPaused)
	{
		if (bInPause)
		{
			// Broadcast ending of any active grasp related contacts
			for (auto& AC : ActiveContacts)
			{
				OnEndSLGraspOverlap.Broadcast(AC);
			}
			ActiveContacts.Empty();

			// Remove grasp related overlap bindings
			OnComponentBeginOverlap.RemoveDynamic(this, &USLManipulatorOverlapShape::OnGraspOverlapBegin);
			OnComponentEndOverlap.RemoveDynamic(this, &USLManipulatorOverlapShape::OnGraspOverlapEnd);

			if (bVisualDebug)
			{
				SetColor(FColor::Yellow);
			}
		}
		else
		{
			// Re-bind the grasp related overlap bindings
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorOverlapShape::OnGraspOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorOverlapShape::OnGraspOverlapEnd);

			if (bVisualDebug)
			{
				SetColor(FColor::Red);
			}
		}
		bIsPaused = bInPause;
	}
}

// Stop publishing overlap events
void USLManipulatorOverlapShape::Finish(bool bForced)
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
// Called when a property is changed in the editor
void USLManipulatorOverlapShape::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapShape, bAttachButton))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapShape, BoneName))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapShape, bIsNotSkeletal))
	{
		if (bIsNotSkeletal)
		{
			BoneName = NAME_None;
			FString NewName = Group == ESLManipulatorOverlapGroup::A ? FString("GraspOverlapGroupA") : FString("GraspOverlapGroupB");
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapShape, IgnoreList))
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
		//		TArray<UActorComponent*> Comps = IA->GetComponentsByClass(USLManipulatorOverlapShape::StaticClass());
		//		for (const auto& C : Comps)
		//		{
		//			USLManipulatorOverlapShape* CastC = CastChecked<USLManipulatorOverlapShape>(C);
		//			if (Group == CastChecked<USLManipulatorOverlapShape>(C)->Group)
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapShape, bVisualDebug))
	{
		SetHiddenInGame(bVisualDebug);
	}
}
#endif // WITH_EDITOR

// Attach component to bone
bool USLManipulatorOverlapShape::AttachToBone()
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
					//	TEXT(__func__), __LINE__, *GetName(), *BoneName.ToString());
					return true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find bone %s for component %s"),
					TEXT(__func__), __LINE__, *BoneName.ToString(), *GetName());
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not attach component %s to the bone %s"),
		TEXT(__func__), __LINE__, *GetName(), *BoneName.ToString());
	return false;
}

// Set debug color
void USLManipulatorOverlapShape::SetColor(FColor Color)
{
	if (ShapeColor != Color)
	{
		ShapeColor = Color;
		MarkRenderStateDirty();
	}
}

// Publish currently overlapping components
void USLManipulatorOverlapShape::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		if (bDetectGrasps)
		{
			OnGraspOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
		}
		if (bDetectContacts)
		{
			OnContactOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
		}
	}
}

// Called on overlap begin events
void USLManipulatorOverlapShape::OnGraspOverlapBegin(UPrimitiveComponent* OverlappedComp,
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

	if (OtherComp->IsA(UStaticMeshComponent::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		ActiveContacts.Emplace(OtherActor);
		OnBeginSLGraspOverlap.Broadcast(OtherActor);
		if (bVisualDebug)
		{
			SetColor(FColor::Green);
		}
	}
}

// Called on overlap begin events
void USLManipulatorOverlapShape::OnContactOverlapBegin(UPrimitiveComponent* OverlappedComp,
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

	if (OtherComp->IsA(UStaticMeshComponent::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		OnBeginSLContactOverlap.Broadcast(OtherActor);
	}
}

// Called on overlap end events
void USLManipulatorOverlapShape::OnGraspOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// It seems SetGenerateOverlapEvents(false) will trigger the overlap end event, this flag avoids those triggers
	if (bIsPaused)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		return;
	}

	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (OtherComp->IsA(UStaticMeshComponent::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		if (ActiveContacts.Remove(OtherActor) > 0)
		{
			OnEndSLGraspOverlap.Broadcast(OtherActor);
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

// Called on overlap end events
void USLManipulatorOverlapShape::OnContactOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (OtherComp->IsA(UStaticMeshComponent::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		OnEndSLContactOverlap.Broadcast(OtherActor);
	}
}
