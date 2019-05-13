// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLGraspOverlapShape.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
USLGraspOverlapShape::USLGraspOverlapShape()
{
	// Default sphere radius
	InitSphereRadius(1.5f);

	// Default group of the finger
	Group = ESLGraspOverlapGroup::A;
	bVisualDebug = true;

#if WITH_EDITOR
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITOR

	UE_LOG(LogTemp, Error, TEXT("%s::%d"), *FString(__func__), __LINE__);
}

// Dtor
USLGraspOverlapShape::~USLGraspOverlapShape()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d"), *FString(__func__), __LINE__);
}

// Attach to bone 
bool USLGraspOverlapShape::Init()
{
	if (!bIsInit)
	{
		// Try attaching the component to its bone
		if (AttachToBone())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);

			SetHiddenInGame(!bVisualDebug);

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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d start overlaps bindiung, loc=%s"), TEXT(__func__), __LINE__, *GetComponentLocation().ToString());

		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLGraspOverlapShape::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLGraspOverlapShape::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
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
		if (AttachToBone())
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
		if (AttachToBone())
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
				if (AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Attached component %s to the bone %s"),
						TEXT(__func__), __LINE__, *GetName(), *BoneName.ToString());
					return true;
				}
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Selfoverlap"), TEXT(__func__), __LINE__);
		return;
	}

	// Ignore overlap with other shape
	if (OtherComp->IsA(USLGraspOverlapShape::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d other bone overlap"), TEXT(__func__), __LINE__);
		return;
	}

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Forward the overlap with the item
		OnBeginSLGraspOverlap.Broadcast(OtherActor);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d good overlap"), TEXT(__func__), __LINE__);
	}
}

// Called on overlap end events
void USLGraspOverlapShape::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Selfoverlap END"), TEXT(__func__), __LINE__);
		return;
	}

	// Ignore overlap with other shape
	if (OtherComp->IsA(USLGraspOverlapShape::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d other bone overlap END"), TEXT(__func__), __LINE__);
		return;
	}

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Forward the overlap with the item
		OnEndSLGraspOverlap.Broadcast(OtherActor);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d good overlap END"), TEXT(__func__), __LINE__);
	}
}

