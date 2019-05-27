// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLGraspOverlapShape.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
USLGraspOverlapShape::USLGraspOverlapShape()
{
	// Default sphere radius
	InitSphereRadius(1.0f);

	//bMultiBodyOverlap = true;

	// Default group of the finger
	Group = ESLGraspOverlapGroup::A;
	bVisualDebug = true;
	bSnapToBone = true;
	bIsNotSkeletal = false;

#if WITH_EDITOR
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITOR
}

// Dtor
USLGraspOverlapShape::~USLGraspOverlapShape()
{
}

// Attach to bone 
bool USLGraspOverlapShape::Init()
{
	if (!bIsInit)
	{
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
void USLGraspOverlapShape::Start()
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

		// https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Delegates/Dynamic/index.html
		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLGraspOverlapShape::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLGraspOverlapShape::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue listening to overlaps 
void USLGraspOverlapShape::Idle(bool bInIdle)
{
	if (bInIdle != bIsIdle)
	{
		// TODO this triggers on next tick for ending the overlap events
		// Pause / start overlap events
		SetGenerateOverlapEvents(!bInIdle);

		if (bInIdle)
		{
			// Broadcast ending of any active grasp related contacts
			for (auto& AC : ActiveContacts)
			{
				OnEndSLGraspOverlap.Broadcast(AC);
			}
			ActiveContacts.Empty();

			if (bVisualDebug)
			{
				SetColor(FColor::Yellow);
			}
		}
		else
		{
			// Broadcast currently overlapping components
			TriggerInitialOverlaps();

			if (bVisualDebug)
			{
				SetColor(FColor::Red);
			}
		}
		bIsIdle = bInIdle;
	}
}

// Stop publishing overlap events
void USLGraspOverlapShape::Finish(bool bForced)
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
void USLGraspOverlapShape::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, bAttachButton))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, BoneName))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, bIsNotSkeletal))
	{
		if (bIsNotSkeletal)
		{
			BoneName = NAME_None;
			FString NewName = Group == ESLGraspOverlapGroup::A ? FString("GraspOverlapGroupA") : FString("GraspOverlapGroupB");
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, IgnoreList))
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
		//		TArray<UActorComponent*> Comps = IA->GetComponentsByClass(USLGraspOverlapShape::StaticClass());
		//		for (const auto& C : Comps)
		//		{
		//			USLGraspOverlapShape* CastC = CastChecked<USLGraspOverlapShape>(C);
		//			if (Group == CastChecked<USLGraspOverlapShape>(C)->Group)
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, bVisualDebug))
	{
		SetHiddenInGame(bVisualDebug);
	}
}
#endif // WITH_EDITOR

// Attach component to bone
bool USLGraspOverlapShape::AttachToBone()
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
void USLGraspOverlapShape::SetColor(FColor Color)
{
	if (ShapeColor != Color)
	{
		ShapeColor = Color;
		MarkRenderStateDirty();
	}
}

// Publish currently overlapping components
void USLGraspOverlapShape::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		OnOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Called on overlap begin events
void USLGraspOverlapShape::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		//if (bVisualDebug)
		//{
		//	if (ActiveContacts.Num() == 0)
		//	{
		//		SetColor(FColor::Red);
		//	}
		//}
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass()) 
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

// Called on overlap end events
void USLGraspOverlapShape::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// It seems SetGenerateOverlapEvents(false) will trigger the overlap end event, this flag avoids those triggers
	if (bIsIdle)
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		return;
	}

	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		//if (bVisualDebug)
		//{
		//	if (ActiveContacts.Num() == 0)
		//	{
		//		SetColor(FColor::Red);
		//	}
		//}
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass()) && !IgnoreList.Contains(OtherActor))
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

