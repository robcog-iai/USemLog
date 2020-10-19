// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLManipulatorMonitor.h"
#include "Monitors/SLManipulatorBoneContactMonitor.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h" // AdHoc grasp helper
#include "Engine/StaticMeshActor.h" // AdHoc grasp helper
#include "Components/StaticMeshComponent.h" // AdHoc grasp helper
#include "Components/SkeletalMeshComponent.h" // AdHoc grasp helper

#if SL_WITH_MC_GRASP
#include "MCGraspAnimController.h"
#endif // SL_WITH_MC_GRASP

// Sets default values for this component's properties
USLManipulatorMonitor::USLManipulatorMonitor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	bIgnore = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bGraspIsDirty = true;
	bIsPaused = false;
	InputAxisName = "LeftGrasp";
	bIsNotSkeletal = false;
	UnPauseTriggerVal = 0.3;

#if WITH_EDITORONLY_DATA
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITORONLY_DATA

	ActiveGraspType = "Default";

	// Grasp helper
	bUseAdHocGraspHelper = false;
	bUseAdHocManualOverrideAction = false;
	AdHocManualOverrideInputActionName = "LeftGraspHelper";
	AdHocOwnerHandBoneName = "lHand";
	bDisableGravityOfGraspedObject = true;
	bScaleMassOfGraspedObject = true;
	GraspedObjectMassScaleValue = 0.1f;
	ConstraintLimit = 0.1f;
	ConstraintStiffness = 500.f;
	ConstraintDamping = 5.f;
	ConstraintContactDistance = 1.f;
	bConstraintParentDominates = false;
	bAdHocGraspHelpIsActive = false;
	AdHocGraspHelperConstraint = nullptr;
	AdHocGraspedSMC = nullptr;
	AdHocOwnerSkelMC = nullptr;
}

// Dtor
USLManipulatorMonitor::~USLManipulatorMonitor()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
void USLManipulatorMonitor::Init(bool bInDetectGrasps, bool bInDetectContacts)
{
	if (bIgnore)
	{
		return;
	}

	if (!bIsInit)
	{
		bDetectGrasps = bInDetectGrasps;
		bDetectContacts = bInDetectContacts;

		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			IndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!IndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return;
		}

		// Set the individual object
		OwnerIndividualObject = IndividualComponent->GetIndividualObject();

		// Remove any unset references in the array
		Fingers.Remove(nullptr);

#if SL_WITH_MC_GRASP
		// Subscribe to grasp type changes
		 SubscribeToGraspTypeChanges();
#endif // SL_WITH_MC_GRASP

		 // Ad Hoc grasp helper
		 if (bUseAdHocGraspHelper)
		 {
			 if (!InitAdHocGraspHelper())
			 {
				 // Init failed, disable grasp helper
				 bUseAdHocGraspHelper = false;
			 }
		 }

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
		}
	}
	return;
}

// Start listening to grasp events, update currently overlapping objects
void USLManipulatorMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		SetupInputBindings();

		// Start listening on the bone overlaps
		for (auto BoneOverlap : GroupA)
		{
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapContact);
				BoneOverlap->OnEndManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapGroupAGrasp);
				BoneOverlap->OnEndManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapGroupAGrasp);
			}
			// Start after subscribing
			BoneOverlap->Start();
		}
		for (auto BoneOverlap : GroupB)
		{
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapContact);
				BoneOverlap->OnEndManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapGroupBGrasp);
				BoneOverlap->OnEndManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapGroupBGrasp);
			}
			// Start after subscribing
			BoneOverlap->Start();
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue grasp detection
void USLManipulatorMonitor::PauseGraspDetection(bool bInPause)
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
			const float CurrTimestamp = GetWorld()->GetTimeSeconds();
			for (const auto& Individual : GraspedIndividuals)
			{
				OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, Individual, CurrTimestamp);
			}
			GraspedIndividuals.Empty();
			SetA.Empty();
			SetB.Empty();
		}
	}
}

// Stop publishing grasp events
void USLManipulatorMonitor::Finish(bool bForced)
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
			OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, EvItr.Other, EvItr.Time);
		}
		RecentlyEndedGraspEvents.Empty();

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedContactEvents)
		{
			OnEndManipulatorContact.Broadcast(OwnerIndividualObject, EvItr.Other, EvItr.Time);
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
void USLManipulatorMonitor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorMonitor, HandType))
	{
		if (HandType == ESLGraspHandType::Left)
		{
			InputAxisName = "LeftGrasp";
			AdHocManualOverrideInputActionName = "LeftGraspHelper";
			AdHocOwnerHandBoneName = "lHand";
		}
		else if (HandType == ESLGraspHandType::Right)
		{
			InputAxisName = "RightGrasp";
			AdHocManualOverrideInputActionName = "RightGraspHelper";
			AdHocOwnerHandBoneName = "rHand";
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorMonitor, bIsNotSkeletal))
	{
		Fingers.Empty();
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLManipulatorMonitor::LoadOverlapGroups()
{
	// Lambda to check grasp overlap components of owner and add them to their groups
	const auto GetOverlapComponentsLambda = [this](AActor* Owner)
	{
		TArray<UActorComponent*> GraspOverlaps = Owner->GetComponentsByClass(USLManipulatorBoneContactMonitor::StaticClass());
		for (UActorComponent* GraspOverlapComp : GraspOverlaps)
		{
			USLManipulatorBoneContactMonitor* GraspOverlap = CastChecked<USLManipulatorBoneContactMonitor>(GraspOverlapComp);
			if (GraspOverlap->GetGroup() == ESLManipulatorContactMonitorGroup::A)
			{
				GroupA.Add(GraspOverlap);
			}
			else if (GraspOverlap->GetGroup() == ESLManipulatorContactMonitorGroup::B)
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
bool USLManipulatorMonitor::SubscribeToGraspTypeChanges()
{
	if (UMCGraspAnimController* Sibling = CastChecked<UMCGraspAnimController>(
		GetOwner()->GetComponentByClass(UMCGraspAnimController::StaticClass())))
	{
		Sibling->OnGraspType.AddUObject(this, &USLManipulatorMonitor::OnGraspType);
		return true;
	}
	return false;
}

// Callback on grasp type change
void USLManipulatorMonitor::OnGraspType(const FString& Type)
{
	ActiveGraspType = Type;
	ActiveGraspType.RemoveFromStart("GA_");
	ActiveGraspType.RemoveFromEnd("_Left");
	ActiveGraspType.RemoveFromEnd("_Right");
	ActiveGraspType.Append("Grasp");
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d ActiveGraspType=%s"), *FString(__func__), __LINE__, *ActiveGraspType);
}
#endif // SL_WITH_MC_GRASP

// Bind user inputs
void USLManipulatorMonitor::SetupInputBindings()
{
	// Bind grasp trigger input and update check functions
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAxis(InputAxisName, this, &USLManipulatorMonitor::GraspInputAxisCallback);

			if (bUseAdHocGraspHelper && bUseAdHocManualOverrideAction)
			{
				IC->BindAction(AdHocManualOverrideInputActionName, IE_Pressed, this, &USLManipulatorMonitor::AdHocManualOverride);
				//IC->BindAction(AdHocManualOverrideInputActionName, IE_Pressed, this, &USLManipulatorMonitor::AdHocManualOverride);
				//IC->BindAction(AdHocManualOverrideInputActionName, IE_Released, this, &USLManipulatorMonitor::AdHocManualOverride);
			}
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
}

// Check if the grasp trigger is active
void USLManipulatorMonitor::GraspInputAxisCallback(float Value)
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
void USLManipulatorMonitor::OnBeginOverlapGroupAGrasp(USLBaseIndividual* OtherIndividual)
{
	bool bAlreadyInSet = false;
	SetA.Emplace(OtherIndividual, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupB.Num() != 0 && !GraspedIndividuals.Contains(OtherIndividual))
	{
		CheckGraspState();
	}
}

// Process beginning of grasp in group B
void USLManipulatorMonitor::OnBeginOverlapGroupBGrasp(USLBaseIndividual* OtherIndividual)
{
	bool bAlreadyInSet = false;
	SetB.Emplace(OtherIndividual, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupA.Num() != 0 && !GraspedIndividuals.Contains(OtherIndividual))
	{
		CheckGraspState();
	}
}

// Process ending of contact in group A
void USLManipulatorMonitor::OnEndOverlapGroupAGrasp(USLBaseIndividual* OtherIndividual)
{
	if (SetA.Remove(OtherIndividual) > 0)
	{
		if (GraspedIndividuals.Contains(OtherIndividual))
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* init from Group A. ~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			EndGrasp(OtherIndividual);
		}
	}
}

// Process ending of contact in group B
void USLManipulatorMonitor::OnEndOverlapGroupBGrasp(USLBaseIndividual* OtherIndividual)
{
	if (SetB.Remove(OtherIndividual) > 0)
	{
		if (GraspedIndividuals.Contains(OtherIndividual))
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* init from Group B. ~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			EndGrasp(OtherIndividual);
		}
	}
}

// Check for grasping state
void USLManipulatorMonitor::CheckGraspState()
{
	for (const auto& Individual : SetA.Intersect(SetB))
	{
		if (!GraspedIndividuals.Contains(Individual))
		{
			BeginGrasp(Individual);
		}
	}
}

// A grasp has started
void USLManipulatorMonitor::BeginGrasp(USLBaseIndividual* OtherIndividual)
{
	// Check if it is a new grasp event, or a concatenation with a previous one, either way, there is a new grasp
	GraspedIndividuals.Emplace(OtherIndividual);
	if(!SkipRecentGraspEndEventBroadcast(OtherIndividual, GetWorld()->GetTimeSeconds()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] \t ~~~~~~~~~~~~~~~~~~~~~~~~~~ *BEGIN GRASP BCAST* with %s ~~~~~~~~~~~~~~~~~~~~~~~~~~"),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *OtherIndividual->GetName());
		OnBeginManipulatorGrasp.Broadcast(OwnerIndividualObject, OtherIndividual, GetWorld()->GetTimeSeconds(), ActiveGraspType);

		if (bUseAdHocGraspHelper)
		{
			if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherIndividual->GetParentActor()))
			{
				if (!bUseAdHocManualOverrideAction)
				{
					AdHocGraspedSMC = AsSMA->GetStaticMeshComponent();
					StartAdHocGraspHelper();
				}
				else if (!bAdHocGraspHelpIsActive)
				{
					// Allow manual grasp override
					AdHocGraspedSMC = AsSMA->GetStaticMeshComponent();
					bCanExecuteManualOverride = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s could not help grasp %s because it is not a static mesh actor.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* cancelled (concatenation) ~~~~~~~~~~"),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
	}
}

// A grasp has ended
void USLManipulatorMonitor::EndGrasp(USLBaseIndividual* OtherIndividual)
{
	if (GraspedIndividuals.Remove(OtherIndividual) > 0)
	{
		// Grasp ended
		RecentlyEndedGraspEvents.Emplace(FSLGraspEndEvent(OtherIndividual, GetWorld()->GetTimeSeconds()));
		
		// Delay publishing for a while, in case the new event is of the same type and should be concatenated
		if(!GetWorld()->GetTimerManager().IsTimerActive(GraspDelayTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this,
				&USLManipulatorMonitor::DelayedGraspEndEventCallback,
				MaxGraspJitterInterval + 0.05f, false);
		}
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorMonitor::DelayedGraspEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > MaxGraspJitterInterval)
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ~~~~~~~~~~~~~~~~~~~~~~~~~~ *END GRASP BCAST* (with delay) GraspEnd=%f; with %s;  ~~~~~~~~~~~~~~~~~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), EvItr->Time, *EvItr->OtherIndividual->GetName());
			// Broadcast delayed event
			OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, EvItr->Other, EvItr->Time);
			
			if (bUseAdHocGraspHelper)
			{
				if (!bUseAdHocManualOverrideAction)
				{
					StopAdHocGraspHelper();
				}
				else if (!bAdHocGraspHelpIsActive)
				{
					// Cancel the manual override
					bCanExecuteManualOverride = false;
				}
			}

			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedGraspEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this,
			&USLManipulatorMonitor::DelayedGraspEndEventCallback,
			MaxGraspJitterInterval + 0.05f, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorMonitor::SkipRecentGraspEndEventBroadcast(USLBaseIndividual* OtherIndividual, float StartTime)
{
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == OtherIndividual)
		{
			// Check time difference
			if(StartTime - EvItr->Time < MaxGraspJitterInterval)
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
void USLManipulatorMonitor::OnBeginOverlapContact(USLBaseIndividual* OtherIndividual)
{

	if (int32* NumContacts = IndividualsInContact.Find(OtherIndividual))
	{
		(*NumContacts)++;
	}
	else
	{
		// Check if it is a new contact event, or a concatenation with a previous one, either way, there is a new contact
		IndividualsInContact.Add(OtherIndividual, 1);
		const float CurrTime = GetWorld()->GetTimeSeconds();
		if(!SkipRecentContactEndEventBroadcast(OtherIndividual, CurrTime))
		{
			// Broadcast begin of semantic overlap event
			OnBeginManipulatorContact.Broadcast(FSLContactResult(OwnerIndividualObject, OtherIndividual, CurrTime, false));
		}
	}
}

// Process ending of contact
void USLManipulatorMonitor::OnEndOverlapContact(USLBaseIndividual* OtherIndividual)
{
	if (int32* NumContacts = IndividualsInContact.Find(OtherIndividual))
	{
		(*NumContacts)--;
			
		if ((*NumContacts) < 1)
		{
			// Remove contact object
			IndividualsInContact.Remove(OtherIndividual);

			if (!GetWorld())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d The world is finished.. this should not happen (often).."),
					*FString(__FUNCTION__), __LINE__);
				// Episode already finished, continuing would be futile
				return;
			}

			// Manipulator contact ended
			RecentlyEndedContactEvents.Emplace(FSLContactEndEvent(OtherIndividual, GetWorld()->GetTimeSeconds()));
				
			// Delay publishing for a while, in case the new event is of the same type and should be concatenated
			if(!GetWorld()->GetTimerManager().IsTimerActive(ContactDelayTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this,
					&USLManipulatorMonitor::DelayedContactEndEventCallback,
					MaxContactJitterInterval + 0.05, false);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's overlap end object (%s) is not in the contacts list, this should not happen.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
	}

}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorMonitor::DelayedContactEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > MaxContactJitterInterval)
		{
			OnEndManipulatorContact.Broadcast(OwnerIndividualObject, EvItr->Other, EvItr->Time);
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedContactEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this,
			&USLManipulatorMonitor::DelayedContactEndEventCallback,
			MaxContactJitterInterval + 0.05f, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorMonitor::SkipRecentContactEndEventBroadcast(USLBaseIndividual* InOther, float StartTime)
{
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == InOther)
		{
			// Check time difference between the previous and current event
			const float TimeGap = StartTime - EvItr->Time;
			if(TimeGap < MaxContactJitterInterval)
			{
				// Event will be concatenated
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedContactEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(ContactDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
/* End contact related */

/* Ad Hoc grasp helper */
// Ad hoc manual override
void USLManipulatorMonitor::AdHocManualOverride()
{
	if (bCanExecuteManualOverride)
	{
		if (!bAdHocGraspHelpIsActive)
		{
			StartAdHocGraspHelper();
		}
		else
		{
			StopAdHocGraspHelper();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s has nothing to manually override.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}
}

// Setup the grasp helper constraint
bool USLManipulatorMonitor::InitAdHocGraspHelper()
{
	if (ASkeletalMeshActor* AsSkelMA = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		AdHocOwnerSkelMC = AsSkelMA->GetSkeletalMeshComponent();

		if (AdHocOwnerSkelMC->GetBoneIndex(AdHocOwnerHandBoneName) != INDEX_NONE)
		{
			// Create and init the ad hoc grasp helper constraint
			AdHocGraspHelperConstraint = NewObject<UPhysicsConstraintComponent>(this, FName("AdHocGraspHelperConstraint"));
			AdHocGraspHelperConstraint->RegisterComponent();
			AdHocGraspHelperConstraint->AttachToComponent(AdHocOwnerSkelMC,
				FAttachmentTransformRules::SnapToTargetIncludingScale, AdHocOwnerHandBoneName);


			AdHocGraspHelperConstraint->ConstraintInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, ConstraintLimit);
			AdHocGraspHelperConstraint->ConstraintInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, ConstraintLimit);
			AdHocGraspHelperConstraint->ConstraintInstance.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, ConstraintLimit);

			AdHocGraspHelperConstraint->ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, ConstraintLimit);
			AdHocGraspHelperConstraint->ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, ConstraintLimit);
			AdHocGraspHelperConstraint->ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, ConstraintLimit);

			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = ConstraintStiffness;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.Damping = ConstraintDamping;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.ContactDistance = ConstraintContactDistance;

			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = true;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = ConstraintStiffness;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.Damping = ConstraintDamping;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.ContactDistance = ConstraintContactDistance;

			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = true;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = ConstraintStiffness;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.Damping = ConstraintDamping;
			AdHocGraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.ContactDistance = ConstraintContactDistance;

			if (bConstraintParentDominates)
			{
				AdHocGraspHelperConstraint->ConstraintInstance.EnableParentDominates();
			}

			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner attachment bone %s does not exist.. aborting grasp help.."),
				*FString(__FUNCTION__), __LINE__, *AdHocOwnerHandBoneName.ToString());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not a skeletal component.. aborting grasp help.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}

// Start grasp help
void USLManipulatorMonitor::StartAdHocGraspHelper()
{
	if (bAdHocGraspHelpIsActive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasp help is already active, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		return;
	}

	if (AdHocGraspedSMC && AdHocGraspedSMC->IsValidLowLevel() && !AdHocGraspedSMC->IsPendingKillOrUnreachable())
	{
		// Apply the properties to the grasped object
		if (bDisableGravityOfGraspedObject)
		{
			AdHocGraspedSMC->SetEnableGravity(false);
		}

		if (bScaleMassOfGraspedObject)
		{
			AdHocGraspedSMC->SetMassScale(NAME_None, GraspedObjectMassScaleValue);
		}

		AdHocGraspHelperConstraint->SetConstrainedComponents(AdHocOwnerSkelMC, AdHocOwnerHandBoneName,
			AdHocGraspedSMC, NAME_None);

		bAdHocGraspHelpIsActive = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasped object is not valid, cannot start helping, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}
}

// Stop grasp help
void USLManipulatorMonitor::StopAdHocGraspHelper()
{
	if (!bAdHocGraspHelpIsActive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasp help is already stopped, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		return;
	}

	AdHocGraspHelperConstraint->BreakConstraint();

	if (AdHocGraspedSMC && AdHocGraspedSMC->IsValidLowLevel() && !AdHocGraspedSMC->IsPendingKillOrUnreachable())
	{
		// Reset the properties to the grasped object
		if (bDisableGravityOfGraspedObject)
		{
			AdHocGraspedSMC->SetEnableGravity(true);
		}

		if (bScaleMassOfGraspedObject)
		{
			AdHocGraspedSMC->SetMassScale(NAME_None, 1.f);
		}

		AdHocGraspedSMC = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasped object is not valid, cannot reset the grasped object values, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}

	bAdHocGraspHelpIsActive = false;
	bCanExecuteManualOverride = false;
}