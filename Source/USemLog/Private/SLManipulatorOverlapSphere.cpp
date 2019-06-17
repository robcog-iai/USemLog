// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManipulatorOverlapSphere.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
USLManipulatorOverlapSphere::USLManipulatorOverlapSphere()
{
	// Default sphere radius
	InitSphereRadius(1.25f);

	//bMultiBodyOverlap = true;

	// Default group of the finger
	Group = ESLManipulatorOverlapGroup::A;
	bVisualDebug = true;
	bSnapToBone = true;
	bIsNotSkeletal = false;
	bGraspPaused = false;
	bDetectGrasps = false;
	bDetectContacts = false;

#if WITH_EDITOR
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITOR
}

// Attach to bone 
bool USLManipulatorOverlapSphere::Init(bool bGrasp, bool bContact)
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
void USLManipulatorOverlapSphere::Start()
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
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorOverlapSphere::OnGraspOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorOverlapSphere::OnGraspOverlapEnd);
		}

		if (bDetectContacts)
		{
			TriggerInitialContactOverlaps();
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorOverlapSphere::OnContactOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorOverlapSphere::OnContactOverlapEnd);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue listening to overlaps 
void USLManipulatorOverlapSphere::PauseGrasp(bool bInPause)
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
				OnEndSLGraspOverlap.Broadcast(AC);
			}
			ActiveContacts.Empty();

			// Remove grasp related overlap bindings
			OnComponentBeginOverlap.RemoveDynamic(this, &USLManipulatorOverlapSphere::OnGraspOverlapBegin);
			OnComponentEndOverlap.RemoveDynamic(this, &USLManipulatorOverlapSphere::OnGraspOverlapEnd);
		}
		else
		{
			TriggerInitialGraspOverlaps();
			// Re-bind the grasp related overlap bindings
			OnComponentBeginOverlap.AddDynamic(this, &USLManipulatorOverlapSphere::OnGraspOverlapBegin);
			OnComponentEndOverlap.AddDynamic(this, &USLManipulatorOverlapSphere::OnGraspOverlapEnd);
		}
		
	}
}

// Stop publishing overlap events
void USLManipulatorOverlapSphere::Finish(bool bForced)
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
void USLManipulatorOverlapSphere::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapSphere, bAttachButton))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapSphere, BoneName))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapSphere, bIsNotSkeletal))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapSphere, IgnoreList))
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
		//		TArray<UActorComponent*> Comps = IA->GetComponentsByClass(USLManipulatorOverlapSphere::StaticClass());
		//		for (const auto& C : Comps)
		//		{
		//			USLManipulatorOverlapSphere* CastC = CastChecked<USLManipulatorOverlapSphere>(C);
		//			if (Group == CastChecked<USLManipulatorOverlapSphere>(C)->Group)
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorOverlapSphere, bVisualDebug))
	{
		SetHiddenInGame(bVisualDebug);
	}
}
#endif // WITH_EDITOR

// Attach component to bone
bool USLManipulatorOverlapSphere::AttachToBone()
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
void USLManipulatorOverlapSphere::SetColor(FColor Color)
{
	if (ShapeColor != Color)
	{
		ShapeColor = Color;
		MarkRenderStateDirty();
	}
}

// Publish currently grasp related overlapping components
void USLManipulatorOverlapSphere::TriggerInitialGraspOverlaps()
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

// Publish currently contact related overlapping components
void USLManipulatorOverlapSphere::TriggerInitialContactOverlaps()
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
void USLManipulatorOverlapSphere::OnGraspOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d START OA:%s; T:%f START"), *FString(__func__), __LINE__,
	//	*OtherActor->GetName(), GetWorld()->GetTimeSeconds());
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		//UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d START OA:%s; T:%f START"), *FString(__func__), __LINE__,
		//	*OtherActor->GetName(), GetWorld()->GetTimeSeconds());
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
			FString::Printf(TEXT(" *  *  *START* *BCAST* GraspContact %s<-->%s T:%f"),
				*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds()), false, FVector2D(1.5f, 1.5f));
		ActiveContacts.Emplace(OtherActor);
		OnBeginSLGraspOverlap.Broadcast(OtherActor);

		if (bVisualDebug)
		{
			SetColor(FColor::Green);
		}
	}
}

// Called on overlap begin events
void USLManipulatorOverlapSphere::OnContactOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, 
		//	FString::Printf(TEXT(" !! SELF !! T:%f"), GetWorld()->GetTimeSeconds()),
		//	false, FVector2D(1.5f, 1.5f));
		return;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
	//	FString::Printf(TEXT(" *  *START* *CONTACT* %s<-->%s T:%f"),
	//		*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds()), false, FVector2D(1.5f, 1.5f));

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
		//	FString::Printf(TEXT(" *  *  *START* *BCAST* CONTACT %s<-->%s T:%f C=%d"),
		//		*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds(), OvCounter), false, FVector2D(1.5f, 1.5f));
		OnBeginSLContactOverlap.Broadcast(OtherActor);
	}
}

// Called on overlap end events
void USLManipulatorOverlapSphere::OnGraspOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d END OA:%s; T:%f END"), *FString(__func__), __LINE__,
	//	*OtherActor->GetName(), GetWorld()->GetTimeSeconds());
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		if (ActiveContacts.Remove(OtherActor) > 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d END OA:%s; T:%f END"), *FString(__func__), __LINE__,
			//	*OtherActor->GetName(), GetWorld()->GetTimeSeconds());
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
				FString::Printf(TEXT(" *  *  *END* *BCAST* GraspContact %s<-->%s T:%f"),
					*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds()), false, FVector2D(1.5f, 1.5f));
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
void USLManipulatorOverlapSphere::OnContactOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta,
			FString::Printf(TEXT(" !! SELF !! T:%f"),
				GetWorld()->GetTimeSeconds()), false, FVector2D(1.5f, 1.5f));
		return;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
	//	FString::Printf(TEXT(" *  *END* *CONTACT* %s<-->%s T:%f"),
	//		*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds()), false, FVector2D(1.5f, 1.5f));

	if (OtherActor->IsA(AStaticMeshActor::StaticClass())
		&& !IgnoreList.Contains(OtherActor))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
		//	FString::Printf(TEXT(" *  *  *END* *BCAST* *CONTACT* %s<-->%s T:%f C:%d"),
		//		*GetName(), *OtherActor->GetName(), GetWorld()->GetTimeSeconds(), OvCounter), false, FVector2D(1.5f, 1.5f));
		OnEndSLContactOverlap.Broadcast(OtherActor);
	}
}
