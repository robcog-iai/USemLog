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
	InputAxisTriggerThresholdValue = 0.3f;
	
	bLogDebug = false;


#if WITH_EDITORONLY_DATA
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITORONLY_DATA

	ActiveGraspType = "Default";

	GraspConcatenateIfSmaller = 0.42f;
	ContactConcatenateIfSmaller = 0.22f;

	// Grasp helper
	bUseGraspHelper = false;
	bGraspHelperManualOverride = false;
	GraspHelperInputActionName = "LeftGraspHelper";
	GraspHelperHandBoneName = "lHand";
	bGraspHelperItemDisableGravity = true;
	bGraspHelperItemScaleMass = true;
	GraspHelperItemMassScaleValue = 0.1f;
	GraspHelperConstraintLimit = 0.1f;
	GraspHelperConstraintStiffness = 500.f;
	GraspHelperConstraintDamping = 5.f;
	GraspHelperConstraintContactDistance = 1.f;
	bGraspHelperConstraintParentDominates = false;
	bAdHocGraspHelpIsActive = false;
	GraspHelperConstraint = nullptr;
	GraspHelperItemSMC = nullptr;
	GraspHelperOwnerSkelMC = nullptr;
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
			// Set the individual object
			OwnerIndividualObject = IndividualComponent->GetIndividualObject();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return;
		}

		// Remove any unset references in the array
		Fingers.Remove(nullptr);

#if SL_WITH_MC_GRASP
		// Subscribe to grasp type changes
		 SubscribeToGraspTypeChanges();
#endif // SL_WITH_MC_GRASP

		 // Ad Hoc grasp helper
		 if (bUseGraspHelper)
		 {
			 if (!InitGraspHelper())
			 {
				 // Init failed, disable grasp helper
				 bUseGraspHelper = false;
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
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully initialized %s::%s at %.4fs.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
		}
	}
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
				BoneOverlap->OnBeginContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnContactBoneOverlapBegin);
				BoneOverlap->OnEndContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnContactBoneOverlapEnd);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnGraspGroupAOverlapBegin);
				BoneOverlap->OnEndGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnGraspGroupAOverlapEnd);
			}
			
			// Start after subscribing
			if (bDetectContacts || bDetectGrasps)
			{
				BoneOverlap->Start();
			}
		}
		for (auto BoneOverlap : GroupB)
		{
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnContactBoneOverlapBegin);
				BoneOverlap->OnEndContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnContactBoneOverlapEnd);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapGroupBGrasp);
				BoneOverlap->OnEndGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapGroupBGrasp);
			}
			// Start after subscribing
			if (bDetectContacts || bDetectGrasps)
			{
				BoneOverlap->Start();
			}
		}

		// Mark as started
		bIsStarted = true;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully started %s::%s at %.4fs.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
	}
}

// Pause/continue grasp detection
void USLManipulatorMonitor::PauseGraspDetection(bool bInPause)
{
	if (bInPause != bIsPaused)
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->SetGraspCheckPaused(bInPause);
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->SetGraspCheckPaused(bInPause);
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

		UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s at %.4fs.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
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
			GraspHelperInputActionName = "LeftGraspHelper";
			GraspHelperHandBoneName = "lHand";
		}
		else if (HandType == ESLGraspHandType::Right)
		{
			InputAxisName = "RightGrasp";
			GraspHelperInputActionName = "RightGraspHelper";
			GraspHelperHandBoneName = "rHand";
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

			if (bUseGraspHelper && bGraspHelperManualOverride)
			{
				IC->BindAction(GraspHelperInputActionName, IE_Pressed, this, &USLManipulatorMonitor::GraspHelperInputCallback);
				//IC->BindAction(GraspHelperInputActionName, IE_Pressed, this, &USLManipulatorMonitor::GraspHelperInputCallback);
				//IC->BindAction(GraspHelperInputActionName, IE_Released, this, &USLManipulatorMonitor::GraspHelperInputCallback);
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
	if (Value >= InputAxisTriggerThresholdValue)
	{
		PauseGraspDetection(false);
	}
	else
	{	
		PauseGraspDetection(true);
	}
}

// Process beginning of grasp in group A
void USLManipulatorMonitor::OnGraspGroupAOverlapBegin(USLBaseIndividual* OtherIndividual)
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
void USLManipulatorMonitor::OnGraspGroupAOverlapEnd(USLBaseIndividual* OtherIndividual)
{
	if (SetA.Remove(OtherIndividual) > 0)
	{
		if (GraspedIndividuals.Contains(OtherIndividual))
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* init from Group A. ~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			GraspEnded(OtherIndividual);
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
			GraspEnded(OtherIndividual);
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
			GraspStarted(Individual);
		}
	}
}

// A grasp has started
void USLManipulatorMonitor::GraspStarted(USLBaseIndividual* OtherIndividual)
{
	// Check if it is a new grasp event, or a concatenation with a previous one, either way, there is a new grasp
	GraspedIndividuals.Emplace(OtherIndividual);
	if(!SkipIfJitterGrasp(OtherIndividual, GetWorld()->GetTimeSeconds()))
	{
		/*UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs \t BeginManipulatorGrasp: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
				GetWorld()->GetTimeSeconds(), *OwnerIndividualObject->GetParentActor()->GetName(), *EvItr->Other->GetParentActor()->GetName());*/
		OnBeginManipulatorGrasp.Broadcast(OwnerIndividualObject, OtherIndividual, GetWorld()->GetTimeSeconds(), ActiveGraspType);

		if (bUseGraspHelper)
		{
			if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherIndividual->GetParentActor()))
			{
				if (!bGraspHelperManualOverride)
				{
					GraspHelperItemSMC = AsSMA->GetStaticMeshComponent();
					StartGraspHelper();
				}
				else if (!bAdHocGraspHelpIsActive)
				{
					// Allow manual grasp override
					GraspHelperItemSMC = AsSMA->GetStaticMeshComponent();
					bGraspHelperCanExecuteManualOverrideFlag = true;
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
void USLManipulatorMonitor::GraspEnded(USLBaseIndividual* OtherIndividual)
{
	if (GraspedIndividuals.Remove(OtherIndividual) > 0)
	{
		// Grasp ended
		RecentlyEndedGraspEvents.Emplace(FSLGraspEndEvent(OtherIndividual, GetWorld()->GetTimeSeconds()));
		
		// Delay publishing for a while, in case the new event is of the same type and should be concatenated
		if(!GetWorld()->GetTimerManager().IsTimerActive(GraspDelayTimerHandle))
		{
			const float DelayValue = GraspConcatenateIfSmaller + ConcatenateIfSmallerDelay;
			GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this,
				&USLManipulatorMonitor::DelayedGraspEndCallback, DelayValue, false);
		}
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorMonitor::DelayedGraspEndCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > GraspConcatenateIfSmaller)
		{
			/*UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs \t (wDelay) EndManipulatorGrasp: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
				GetWorld()->GetTimeSeconds(), *OwnerIndividualObject->GetParentActor()->GetName(), *EvItr->Other->GetParentActor()->GetName());*/
			// Broadcast delayed event
			OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, EvItr->Other, EvItr->Time);
			
			if (bUseGraspHelper)
			{
				if (!bGraspHelperManualOverride)
				{
					StopGraspHelper();
				}
				else if (!bAdHocGraspHelpIsActive)
				{
					// Cancel the manual override
					bGraspHelperCanExecuteManualOverrideFlag = false;
				}
			}

			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedGraspEvents.Num() > 0)
	{
		const float DelayValue = GraspConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this,
			&USLManipulatorMonitor::DelayedGraspEndCallback, DelayValue, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorMonitor::SkipIfJitterGrasp(USLBaseIndividual* OtherIndividual, float StartTime)
{
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == OtherIndividual)
		{
			// Check time difference
			if(StartTime - EvItr->Time < GraspConcatenateIfSmaller)
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
void USLManipulatorMonitor::OnContactBoneOverlapBegin(USLBaseIndividual* OtherIndividual)
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
		if(!SkipIfJitterContact(OtherIndividual, CurrTime))
		{
			/*UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs \t BeginManipulatorContact: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
				GetWorld()->GetTimeSeconds(), *OwnerIndividualObject->GetParentActor()->GetName(), *OtherIndividual->GetParentActor()->GetName());*/
			// Broadcast begin of semantic overlap event
			OnBeginManipulatorContact.Broadcast(FSLContactResult(OwnerIndividualObject, OtherIndividual, CurrTime, false));
		}
	}
}

// Process ending of contact
void USLManipulatorMonitor::OnContactBoneOverlapEnd(USLBaseIndividual* OtherIndividual)
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
				const float DelayValue = ContactConcatenateIfSmaller + ConcatenateIfSmallerDelay;
				GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this,
					&USLManipulatorMonitor::DelayedContactEndCallback,	DelayValue, false);
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
void USLManipulatorMonitor::DelayedContactEndCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > ContactConcatenateIfSmaller)
		{
			/*UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs \t (wDelay) EndManipulatorContact: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
				GetWorld()->GetTimeSeconds(), *OwnerIndividualObject->GetParentActor()->GetName(), *EvItr->Other->GetParentActor()->GetName());*/
			OnEndManipulatorContact.Broadcast(OwnerIndividualObject, EvItr->Other, EvItr->Time);
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedContactEvents.Num() > 0)
	{
		const float DelayValue = ContactConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this,
			&USLManipulatorMonitor::DelayedContactEndCallback, DelayValue, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorMonitor::SkipIfJitterContact(USLBaseIndividual* InOther, float StartTime)
{
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == InOther)
		{
			// Check time difference between the previous and current event
			const float TimeGap = StartTime - EvItr->Time;
			if(TimeGap < ContactConcatenateIfSmaller)
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


/* Begin grasp helper */
// Setup the grasp helper constraint
bool USLManipulatorMonitor::InitGraspHelper()
{
	if (ASkeletalMeshActor* AsSkelMA = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		GraspHelperOwnerSkelMC = AsSkelMA->GetSkeletalMeshComponent();

		if (GraspHelperOwnerSkelMC->GetBoneIndex(GraspHelperHandBoneName) != INDEX_NONE)
		{
			// Create and init the ad hoc grasp helper constraint
			GraspHelperConstraint = NewObject<UPhysicsConstraintComponent>(this, FName("AdHocGraspHelperConstraint"));
			GraspHelperConstraint->RegisterComponent();
			GraspHelperConstraint->AttachToComponent(GraspHelperOwnerSkelMC,
				FAttachmentTransformRules::SnapToTargetIncludingScale, GraspHelperHandBoneName);


			GraspHelperConstraint->ConstraintInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, GraspHelperConstraintLimit);
			GraspHelperConstraint->ConstraintInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, GraspHelperConstraintLimit);
			GraspHelperConstraint->ConstraintInstance.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, GraspHelperConstraintLimit);

			GraspHelperConstraint->ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, GraspHelperConstraintLimit);
			GraspHelperConstraint->ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, GraspHelperConstraintLimit);
			GraspHelperConstraint->ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, GraspHelperConstraintLimit);

			GraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = GraspHelperConstraintStiffness;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.Damping = GraspHelperConstraintDamping;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.LinearLimit.ContactDistance = GraspHelperConstraintContactDistance;

			GraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = true;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = GraspHelperConstraintStiffness;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.Damping = GraspHelperConstraintDamping;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.ConeLimit.ContactDistance = GraspHelperConstraintContactDistance;

			GraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = true;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = GraspHelperConstraintStiffness;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.Damping = GraspHelperConstraintDamping;
			GraspHelperConstraint->ConstraintInstance.ProfileInstance.TwistLimit.ContactDistance = GraspHelperConstraintContactDistance;

			if (bGraspHelperConstraintParentDominates)
			{
				GraspHelperConstraint->ConstraintInstance.EnableParentDominates();
			}

			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner attachment bone %s does not exist.. aborting grasp help.."),
				*FString(__FUNCTION__), __LINE__, *GraspHelperHandBoneName.ToString());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not a skeletal component.. aborting grasp help.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}

// Ad hoc manual override
void USLManipulatorMonitor::GraspHelperInputCallback()
{
	if (bGraspHelperCanExecuteManualOverrideFlag)
	{
		if (!bAdHocGraspHelpIsActive)
		{
			StartGraspHelper();
		}
		else
		{
			StopGraspHelper();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s has nothing to manually override.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}
}

// Start grasp help
void USLManipulatorMonitor::StartGraspHelper()
{
	if (bAdHocGraspHelpIsActive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasp help is already active, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		return;
	}

	if (GraspHelperItemSMC && GraspHelperItemSMC->IsValidLowLevel() && !GraspHelperItemSMC->IsPendingKillOrUnreachable())
	{
		// Apply the properties to the grasped object
		if (bGraspHelperItemDisableGravity)
		{
			GraspHelperItemSMC->SetEnableGravity(false);
		}

		if (bGraspHelperItemScaleMass)
		{
			GraspHelperItemSMC->SetMassScale(NAME_None, GraspHelperItemMassScaleValue);
		}

		GraspHelperConstraint->SetConstrainedComponents(GraspHelperOwnerSkelMC, GraspHelperHandBoneName,
			GraspHelperItemSMC, NAME_None);

		bAdHocGraspHelpIsActive = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasped object is not valid, cannot start helping, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}
}

// Stop grasp help
void USLManipulatorMonitor::StopGraspHelper()
{
	if (!bAdHocGraspHelpIsActive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasp help is already stopped, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		return;
	}

	GraspHelperConstraint->BreakConstraint();

	if (GraspHelperItemSMC && GraspHelperItemSMC->IsValidLowLevel() && !GraspHelperItemSMC->IsPendingKillOrUnreachable())
	{
		// Reset the properties to the grasped object
		if (bGraspHelperItemDisableGravity)
		{
			GraspHelperItemSMC->SetEnableGravity(true);
		}

		if (bGraspHelperItemScaleMass)
		{
			GraspHelperItemSMC->SetMassScale(NAME_None, 1.f);
		}

		GraspHelperItemSMC = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasped object is not valid, cannot reset the grasped object values, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}

	bAdHocGraspHelpIsActive = false;
	bGraspHelperCanExecuteManualOverrideFlag = false;
}
/* End grasp helper */
