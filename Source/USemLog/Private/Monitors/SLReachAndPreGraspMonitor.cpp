// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLReachAndPreGraspMonitor.h"
#include "Monitors/SLMonitorStructs.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"

#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"

// Set default values
USLReachAndPreGraspMonitor::USLReachAndPreGraspMonitor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	InitSphereRadius(30.f);

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	bLogDebug = false;
	bLogVerboseDebug = false;

	CurrGraspedIndividual = nullptr;

	// Default values
	UpdateRate = 0.037;
	ConcatenateIfSmaller = 0.4f;

	ShapeColor = FColor::Orange.WithAlpha(64);

	// Set overlap area collision parameters
	SetCollisionParameters();
}

// Dtor
USLReachAndPreGraspMonitor::~USLReachAndPreGraspMonitor()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called every frame, used for timeline visualizations, activated and deactivated on request
void USLReachAndPreGraspMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateCandidatesData(DeltaTime);
}

// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
void USLReachAndPreGraspMonitor::Init()
{
	if (bIgnore)
	{
		return;
	}

	if (!bIsInit)
	{
		// Make sure the owner is semantically annotated
		if(USLIndividualComponent* IC = FSLIndividualUtils::GetIndividualComponent(GetOwner()))
		{
			OwnerIndividualComponent = IC;
			if (!OwnerIndividualComponent->IsLoaded())
			{				
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return;
			}

			// Set the individual object
			OwnerIndividualObject = OwnerIndividualComponent->GetIndividualObject();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return;
		}

		// Set tick update rate
		if (UpdateRate > 0.f)
		{
			SetComponentTickInterval(UpdateRate);
		}
		
		// Disable overlaps until start
		SetGenerateOverlapEvents(false);

		// Bind overlap events
		OnComponentBeginOverlap.AddDynamic(this, &USLReachAndPreGraspMonitor::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLReachAndPreGraspMonitor::OnOverlapEnd);

		// Subscribe for grasp notifications from sibling monitor component
		if(SubscribeForManipulatorEvents())
		{
			bIsInit = true;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully initialized %s::%s at %.4fs.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
			return;
		}
	}	
	return;
}

// Start listening to grasp events, update currently overlapping objects
void USLReachAndPreGraspMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Start listening for overlaps
		SetGenerateOverlapEvents(true);

		//// Iterate through the currently overlapping componets
		//TriggerInitialOverlaps();
		
		// Mark as started
		bIsStarted = true;

		UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully started %s::%s at %.4fs.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
	}
}

// Stop publishing grasp events
void USLReachAndPreGraspMonitor::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		OnComponentBeginOverlap.RemoveAll(this);
		OnComponentEndOverlap.RemoveAll(this);
		SetComponentTickEnabled(false);
		
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;


		// TODO - Commented out since GetWorld() can crash in newer versions in nullptr
		//if (GetWorld())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s component at %.4fs.."),
		//	*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s component at unknown (world was not valid anymore).."),
		//		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName());
		//}
	}
}

#if WITH_EDITOR
// Called after the CanidateData++ constructor and after the properties have been initialized
void USLReachAndPreGraspMonitor::PostInitProperties()
{
	Super::PostInitProperties();
	RelocateSphere();
}

// Called when a property is changed in the editor
void USLReachAndPreGraspMonitor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLReachAndPreGraspMonitor, SphereRadius))
	{
		RelocateSphere();
	}
}

// Move the sphere location so that its surface overlaps with the end of the manipulator
void USLReachAndPreGraspMonitor::RelocateSphere()
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

// Set collision parameters such as object name and collision responses
void USLReachAndPreGraspMonitor::SetCollisionParameters()
{
	SetCollisionProfileName("SLReachAndPreGrasp");
	//SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
	SetAllUseCCD(true);
}

// Subscribe for hand contact and grasp events
bool USLReachAndPreGraspMonitor::SubscribeForManipulatorEvents()
{
	if(USLManipulatorMonitor* ManipulatorMonitor = CastChecked<USLManipulatorMonitor>(
		GetOwner()->GetComponentByClass(USLManipulatorMonitor::StaticClass())))
	{
		// Timeline reaching   ,    pre grasp
		// [-----------contact][contact--------grasp]
		ManipulatorMonitor->OnBeginManipulatorContact.AddUObject(this, &USLReachAndPreGraspMonitor::OnManipulatorContactBegin);
		ManipulatorMonitor->OnEndManipulatorContact.AddUObject(this, &USLReachAndPreGraspMonitor::OnManipulatorContactEnd);
		ManipulatorMonitor->OnBeginManipulatorGrasp.AddUObject(this, &USLReachAndPreGraspMonitor::OnManipulatorGraspBegin);
		ManipulatorMonitor->OnEndManipulatorGrasp.AddUObject(this, &USLReachAndPreGraspMonitor::OnManipulatorGraspEnd);
		return true;
	}
	return false;
}

// Update callback, checks distance to hand, if it increases it resets the start time
void USLReachAndPreGraspMonitor::UpdateCandidatesData(float DeltaTime)
{
	if (bLogVerboseDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's CandidatesNum=%d; ContactNum=%d; DeltaTime=%f;"),
		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			CandidatesData.Num(), ManipulatorContactData.Num(), DeltaTime);
	}

	const float CurrTimestamp = GetWorld()->GetTimeSeconds();
	for (auto& CanidateData : CandidatesData)
	{
		const float CurrDist = FVector::Distance(GetOwner()->GetActorLocation(), CanidateData.Key->GetParentActor()->GetActorLocation());
		const float PrevDist = CanidateData.Value.Get<1>();
		const float DiffDist = PrevDist - CurrDist;

		// Ignore small difference changes (IgnoreMovementsSmallerThanValue)
		if (DiffDist > IgnoreMovementsSmallerThanValue)
		{
			if (bLogVerboseDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's is moving closer to %s; (PrevDist=%f; CurrDist=%f; DiffDist=%f;)"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *CanidateData.Key->GetParentActor()->GetName(),
					PrevDist, CurrDist, DiffDist);
			}
			// Positive difference makes the hand closer to the object, update the distance
			CanidateData.Value.Get<ESLTimeAndDist::SLDist>() = CurrDist;
		}
		else if (DiffDist < -IgnoreMovementsSmallerThanValue)
		{
			// Negative difference makes the hand further away from the object, update distance, reset the start time
			CanidateData.Value.Get<ESLTimeAndDist::SLTime>() = CurrTimestamp;
			CanidateData.Value.Get<ESLTimeAndDist::SLDist>() = CurrDist;

			if (bLogVerboseDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's is moving further to %s; (PrevDist=%f; CurrDist=%f; DiffDist=%f;)"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *CanidateData.Key->GetParentActor()->GetName(),
					PrevDist, CurrDist, DiffDist);
			}
		}
		else
		{
			// TODO reset time when idling for a longer period
			if (bLogVerboseDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's is idling relative to %s; (PrevDist=%f; CurrDist=%f; DiffDist=%f;)"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *CanidateData.Key->GetParentActor()->GetName(),
					PrevDist, CurrDist, DiffDist);
			}
		}
	}
}

// Publish currently overlapping components
void USLReachAndPreGraspMonitor::TriggerInitialOverlaps()
{
	/*UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4fs \t\t Started manual overlap trigger"),
		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());*/

	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	const FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		OnOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4fs \t\t Finished manual overlap trigger"),
	//	*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
}

// Check if the object is can be a candidate for reaching
//bool USLReachAndPreGraspMonitor::CanBeACandidate(AStaticMeshActor* InObject) const
bool USLReachAndPreGraspMonitor::CanBeACandidate(AActor* InOther) const
{
	return InOther->IsA(AStaticMeshActor::StaticClass());

	//UActorComponent* AC = InObject->GetComponentByClass(USLIndividualComponent::StaticClass());
	//return AC != nullptr;
	
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
void USLReachAndPreGraspMonitor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	// Check if the individual can be a reach candidate
	if (CanBeACandidate(OtherActor))
	{
		const float Dist = FVector::Distance(GetOwner()->GetActorLocation(), OtherActor->GetActorLocation());
		CandidatesData.Emplace(OtherIndividual, MakeTuple(GetWorld()->GetTimeSeconds(), Dist));

		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's added %s::%s as candidate (CandidatesNum=%d).."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *OtherActor->GetName(), *OtherComp->GetName(), CandidatesData.Num());
		}

		// Make sure the candidate update check is running
		if (!IsComponentTickEnabled())
		{
			if (bLogDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f \t %s's first candidate added, starting tick.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
			}
			SetComponentTickEnabled(true);
		}
	}
	else
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's overlap begin %s::%s is not a suitable candidate.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*OtherActor->GetName(), *OtherComp->GetName());
		}
	}
}

// Checks for candidates in the overlap area
void USLReachAndPreGraspMonitor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	// Remove candidate
	if (CandidatesData.Remove(OtherIndividual) > 0)
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's removed %s::%s as candidate (CandidatesNum=%d).."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *OtherActor->GetName(), *OtherComp->GetName(), CandidatesData.Num());
		}

		// Stop candidate update if this was the last candidate
		if (CandidatesData.Num() == 0)
		{
			if (bLogDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f \t %s's last candidate removed, stopping tick.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
			}
			SetComponentTickEnabled(false);
		}
	}
	else
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's overlap end %s::%s was not a candidate to remove.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*OtherActor->GetName(), *OtherComp->GetName());
		}
	}
}


// Called when sibling detects a grasp, used for ending the manipulator positioning event
void USLReachAndPreGraspMonitor::OnManipulatorGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Timestamp, const FString& GraspType)
{
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Grasp event started received: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	if(CurrGraspedIndividual)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasp individual is already set (%s), this can happen if grasping mulitple individuals, ignoring grasp %s.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
		return;
	}

	// Check if the grasped object is a candidate and is in contact with the hand
	if(FSLTimeAndDist* CandidateTimeAndDist = CandidatesData.Find(Other))
	{
		// TODO this could be an outdated time due to the delay, it however makes sense to keep it this way
		// since if there is a grasp with the object, it should also be in contact with
		if(float* ContactTime = ManipulatorContactData.Find(Other))
		{
			if (bLogDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's setting grasped individual to %s.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(),	*Other->GetParentActor()->GetName());
			}

			// Grasp is active, ignore future contact/grasp events
			CurrGraspedIndividual = Other;

			// Cancel delay callback if active
			GetWorld()->GetTimerManager().ClearTimer(DelayTimerHandle);

			// Broadcast reach and pre grasp events
			const float ReachStartTime = CandidateTimeAndDist->Get<ESLTimeAndDist::SLTime>();
			const float ReachEndTime = *ContactTime;
			OnReachAndPreGraspEvent.Broadcast(OwnerIndividualObject, Other, ReachStartTime, ReachEndTime, Timestamp);

			if (bLogDebug)
			{
				FString CandidatesStr;
				for (const auto& C : CandidatesData)
				{
					CandidatesStr.Append(C.Key->GetParentActor()->GetName() + ";");
				}
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's removing candidates %s.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *CandidatesStr);
			}

			// Remove existing candidates and pause the update callback while the hand is grasping
			CandidatesData.Empty();
			ManipulatorContactData.Empty();
			SetComponentTickEnabled(false);

			// Disable overlaps until grasp is released
			SetGenerateOverlapEvents(false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%4.f %s's grasped individual %s is not in the contacts list, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetParentActor()->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%4.f %s's grasped individual %s is not in the candidates list, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetParentActor()->GetName());
	}

}

// Reset looking for the events
void USLReachAndPreGraspMonitor::OnManipulatorGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{	
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Grasp event ended received: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	if (CurrGraspedIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's no individual is set as grasped (whilst grasp ended with %s), this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetParentActor()->GetName());
		return;
	}

	if (CurrGraspedIndividual != Other)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's unrelated grasp ended (%s != %s), this can happen if grasping multiple individuals.. skipping.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
		return;
	}

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasp ended (%s==%s) start listening to overlaps.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	// Set individual to nullptr
	CurrGraspedIndividual = nullptr;
	
	// Grasp released start listening to overlaps
	SetGenerateOverlapEvents(true);

	// TODO seems this is not needed anymore since the generate overlap events function already triggers the values
	//// Start looking for new candidates
	//TriggerInitialOverlaps();
}

// Called when the sibling is in contact with an object, used for ending the reaching event and starting the manipulator positioning event
void USLReachAndPreGraspMonitor::OnManipulatorContactBegin(const FSLContactResult& ContactResult)
{
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Contact event started received: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*ContactResult.Self->GetParentActor()->GetName(), *ContactResult.Other->GetParentActor()->GetName());
	}

	if(CurrGraspedIndividual)
	{
		// TODO ususbscribe on grasp event (needs to use dynamic delegates)
		//UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasp individual is set, there should be no contact publishing happening.."),
		//	*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
		//	*ContactResult.Self->GetParentActor()->GetName(), *ContactResult.Other->GetParentActor()->GetName());
		// Ignore any manipulator contacts while in grasp mode
		return;
	}

	// Make sure the individual in contact with the hand is in the candidates list
	if (!CandidatesData.Contains(ContactResult.Other))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%4.f %s's %s IS in contact with the manipulator, but it is not in the candidates list, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *ContactResult.Other->GetParentActor()->GetName());
		return;
	}

	// Check if the contact should be concatenated 
	if(!SkipIfJitterContact(ContactResult.Other, ContactResult.Time))
	{
		// Overwrite previous time or create a new contact result
		ManipulatorContactData.Emplace(ContactResult.Other, ContactResult.Time);

		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Added %s to contacts with manipulator list.."), *FString(__FUNCTION__), __LINE__,
				GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *ContactResult.Other->GetParentActor()->GetName());
		}
	}
}

// Manipulator is not in contact with object anymore, check for possible concatenation, or reset the potential reach time
void USLReachAndPreGraspMonitor::OnManipulatorContactEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float EndTime)
{
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Contact event ended received: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	if(CurrGraspedIndividual)
	{
		// TODO ususbscribe on grasp event (needs to use dynamic delegates)
		//UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasp individual is set, there should be no contact publishing happening.."),
		//	*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
		//	*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
		// Ignore any manipulator contacts while in grasp mode
		return;
	}

	// Make sure the individual in contact with the hand is in the candidates list
	if (!CandidatesData.Contains(Other))
	{
		// Might happen due to the contact event end jitter check publishing delay
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%4.f %s's %s WAS in contact with the manipulator, but it is not in the candidates list, this can happen if moving fast.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),	*Other->GetParentActor()->GetName());
		return;
	}

	// Cache the event
	RecentlyEndedEvents.Emplace(Other, EndTime);

	if (!GetWorld())
	{
		// The episode finished, going further is futile
		return;
	}

	// Delay reseting the reach time, it might be a small disconnection with the hand
	if(!GetWorld()->GetTimerManager().IsTimerActive(DelayTimerHandle))
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(DelayTimerHandle,
			this, &USLReachAndPreGraspMonitor::DelayContactEndCallback, DelayValue, false);
	}
}

// Delayed call of setting finished event to check for possible concatenation of jittering events of the same type
void USLReachAndPreGraspMonitor::DelayContactEndCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, reset the reach time
		if(CurrTime - EvItr->Time > ConcatenateIfSmaller)
		{
			// Reset reach start in the candidate
			if(FSLTimeAndDist* TimeAndDist = CandidatesData.Find(EvItr->Other))
			{
				// No new contact happened, remove and reset reach time
				if(ManipulatorContactData.Remove(EvItr->Other) > 0)
				{
					//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s removed as object in contact with the manipulator.. (after delay, contact end time=%f)"),
					//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *EvItr->Other->GetName(), EvItr->Timestamp);
					TimeAndDist->Get<ESLTimeAndDist::SLTime>() = GetWorld()->GetTimeSeconds();
				}
				else
				{
					// Might happen due to the contact event end jitter check publishing delay
					UE_LOG(LogTemp, Error, TEXT("%s::%d::%4.f %s's %s is not in the contact list.. this should not happen.."),
						*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
						*GetOwner()->GetName(),	*EvItr->Other->GetParentActor()->GetName());
				}
			}
			else
			{
				// Might happen due to the contact event end jitter check publishing delay
				UE_LOG(LogTemp, Error, TEXT("%s::%d::%4.f %s's %s is not in the candidates list.. this should not happen.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *EvItr->Other->GetParentActor()->GetName());
			}
			
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedEvents.Num() > 0)
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(DelayTimerHandle,
			this, &USLReachAndPreGraspMonitor::DelayContactEndCallback, DelayValue, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLReachAndPreGraspMonitor::SkipIfJitterContact(USLBaseIndividual* Other, float StartTime)
{
	for (auto EvItr(RecentlyEndedEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == Other)
		{
			// Check time difference between the previous and current event
			const float TimeGap = StartTime - EvItr->Time;
			if(TimeGap < ConcatenateIfSmaller)
			{
				// Event will be concatenated
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(DelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
