// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLManipulatorMonitor.h"
#include "Monitors/SLBoneContactMonitor.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h" // AdHoc grasp helper
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
	bIsGraspDetectionPaused = false;
	InputAxisName = "LeftGrasp";
	bIsNotSkeletal = false;
	InputAxisTriggerThresholdValue = 0.3f;
	
	bLogContactDebug = false;
	bLogGraspDebug = false;
	bLogVerboseGraspDebug = false;

	// Editor button hack
	bLoadBoneMonitorsButtonHack = false;

#if WITH_EDITORONLY_DATA
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITORONLY_DATA

	ActiveGraspType = "Default";

	GraspConcatenateIfSmaller = 0.11f;
	ContactConcatenateIfSmaller = 0.14f;

	// Grasp helper
	bUseGraspHelper = false;
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

	if (bIsInit)
	{
		return;
	}

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

		// Grasp helper
		if (bUseGraspHelper)
		{
			// Init, if it fails, disable grasp help flag
			GraspHelper.Init(this);
			bUseGraspHelper = GraspHelper.IsInit();
		}

	// True if each group has at least one bone overlap
	if (LoadBoneMonitorGroups())
	{
		for (auto BoneMonitor : BoneMonitorsGroupA)
		{
			BoneMonitor->Init(bDetectGrasps, bDetectContacts);
		}
		for (auto BoneMonitor : BoneMonitorsGroupB)
		{
			BoneMonitor->Init(bDetectGrasps, bDetectContacts);
		}

		bIsInit = true;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully initialized %s::%s at %.4fs.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
	}
}

// Start listening to grasp events, update currently overlapping objects
void USLManipulatorMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (bDetectGrasps)
		{
			SetupInputBindings();
			BindGraspContactCallbacks();
		}

		if (bDetectContacts)
		{
			BindContactCallbacks();
		}

		// Start bone publishers (after subscribing)
		if (bDetectContacts || bDetectGrasps)
		{
			for (auto BoneMonitor : BoneMonitorsGroupA)
			{
				BoneMonitor->Start();
			}
			for (auto BoneMonitor : BoneMonitorsGroupB)
			{
				BoneMonitor->Start();
			}

			// Mark as started
			bIsStarted = true;

			UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully started %s::%s at %.4fs.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
		}
	}
}

// Pause/continue grasp detection
void USLManipulatorMonitor::PauseGraspDetection(bool bNewValue)
{
	if (bNewValue != bIsGraspDetectionPaused)
	{
		bIsGraspDetectionPaused = bNewValue;
		
		if (bLogGraspDebug)
		{
			if (bIsGraspDetectionPaused)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Pausing Grasp Detection: \t\t %s::%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Starting Grasp Detection: \t\t %s::%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *GetName());
			}
		}

		// Pause bone detectors, publish the end of grasp contacts, these will eventually trigger the grasp end
		for (auto BoneMonitor : BoneMonitorsGroupA)
		{
			BoneMonitor->PauseGraspDetection(bIsGraspDetectionPaused);
		}
		for (auto BoneMonitor : BoneMonitorsGroupB)
		{
			BoneMonitor->PauseGraspDetection(bIsGraspDetectionPaused);
		}

		// Manual grasp trigger is not needed since the bones publish the grasp end
		//if (bIsGraspDetectionPaused)
		//{
		//	//// Avoid ranged for "Container has changed during ranged-for iteration!"
		//	//TArray<USLBaseIndividual*> GraspedIndividualsCopy = GraspedIndividuals.Array();
		//	//// Broadcast grasp ending through release trigger
		//	//for (const auto& Individual : GraspedIndividualsCopy)
		//	//{
		//	//	if (bLogGraspDebug)
		//	//	{
		//	//		UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Grasp Ended (trigger grasp ended): \t\t %s::%s->%s;"),
		//	//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
		//	//			*GetOwner()->GetName(), *GetName(), *Individual->GetParentActor()->GetName());
		//	//	}
		//	//	//OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, Individual, GetWorld()->GetTimeSeconds());
		//	//	GraspEnded(Individual);
		//	//}

		//	//// Clear bone grasp contacts and any grasped individuals
		//	//GraspedIndividuals.Empty();
		//	//GroupANumGraspContacts.Empty();
		//	//GroupBNumGraspContacts.Empty();
		//	//if (bLogVerboseGraspDebug)
		//	//{
		//	//	UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Cleard group A+B grasp contacts \t\t %s::%s;"),
		//	//		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *GetName());
		//	//}
		//}
	}
}

// Stop publishing grasp events
void USLManipulatorMonitor::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		for (auto BoneMonitor : BoneMonitorsGroupA)
		{
			BoneMonitor->Finish();
		}
		for (auto BoneMonitor : BoneMonitorsGroupB)
		{
			BoneMonitor->Finish();
		}

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedGraspEvents)
		{
			if (bLogGraspDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Grasp Ended ( !!! broadcast !!! through finish): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *EvItr.Other->GetParentActor()->GetName());
			}
			OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, EvItr.Other, EvItr.Time);
		}
		RecentlyEndedGraspEvents.Empty();

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedContactEvents)
		{
			if (bLogGraspDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Contact Ended ( !!! broadcast !!! through finish): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *EvItr.Other->GetParentActor()->GetName());
			}
			OnEndManipulatorContact.Broadcast(OwnerIndividualObject, EvItr.Other, EvItr.Time);
		}
		RecentlyEndedContactEvents.Empty();

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;

		// TODO - Commented out since GetWorld() can crash in newer versions in nullptr
		//if (GetWorld())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s at %.4fs.."),
		//		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s at unknown (world was not valid).."),
		//		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName());
		//}
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
			GraspHelper.DefaultBoneName = "lHand";
			GraspHelper.InputActionName = "LeftGraspHelper";
		}
		else if (HandType == ESLGraspHandType::Right)
		{
			InputAxisName = "RightGrasp";
			GraspHelper.DefaultBoneName = "rHand";
			GraspHelper.InputActionName = "RightGraspHelper";
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorMonitor, bIsNotSkeletal))
	{
		Fingers.Empty();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorMonitor, bLoadBoneMonitorsButtonHack))
	{
		bLoadBoneMonitorsButtonHack = false;
		CreateBoneMonitors();
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLManipulatorMonitor::LoadBoneMonitorGroups()
{
	// Lambda to check grasp overlap components of owner and add them to their groups
	const auto GetOverlapComponentsLambda = [this](AActor* Owner)
	{
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
	TArray<UActorComponent*> GraspOverlaps;
	Owner->GetComponents(USLBoneContactMonitor::StaticClass(), GraspOverlaps);
#else
	TArray<UActorComponent*> GraspOverlaps = Owner->GetComponentsByClass(USLBoneContactMonitor::StaticClass());
#endif
		for (UActorComponent* GraspOverlapComp : GraspOverlaps)
		{
			USLBoneContactMonitor* GraspOverlap = CastChecked<USLBoneContactMonitor>(GraspOverlapComp);
			if (GraspOverlap->GetGroup() == ESLBoneContactGroup::A)
			{
				BoneMonitorsGroupA.Add(GraspOverlap);
			}
			else if (GraspOverlap->GetGroup() == ESLBoneContactGroup::B)
			{
				BoneMonitorsGroupB.Add(GraspOverlap);
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
	if (BoneMonitorsGroupA.Num() == 0 || BoneMonitorsGroupB.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d One of the grasp groups is empty grasp detection disabled."), *FString(__func__), __LINE__);
		bDetectGrasps = false;
	}
	return true;
}

// Create and attach bone monitors to the owner
void USLManipulatorMonitor::CreateBoneMonitors()
{
	if (ASkeletalMeshActor* AsSkelMA = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		if (USkeletalMeshComponent* SkelMC = AsSkelMA->GetSkeletalMeshComponent())
		{
			AActor* OnwerActor = GetOwner();

			// Get existing bone monitors
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
			TArray<UActorComponent*> ExistingBoneMonitors;
			AsSkelMA->GetComponents(USLBoneContactMonitor::StaticClass(), ExistingBoneMonitors);
#else
			TArray<UActorComponent*> ExistingBoneMonitors = AsSkelMA->GetComponentsByClass(USLBoneContactMonitor::StaticClass());
#endif

			// Get list of the skeletal bone names
			TArray<FName> BoneNames;
			SkelMC->GetBoneNames(BoneNames);

			// Create bone monitors for the missing bones
			for (const auto& BoneName : BoneNames)
			{
				// Check if monitor already exists
				bool bAlreadyExists = false;
				for (int32 BMIdx = 0; BMIdx < ExistingBoneMonitors.Num(); ++BMIdx)
				{
					if (CastChecked<USLBoneContactMonitor>(ExistingBoneMonitors[BMIdx])->GetAttachedBoneName().IsEqual(BoneName))
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Bone monitor %s for bone %s already exists.."),
							*FString(__FUNCTION__), __LINE__, *ExistingBoneMonitors[BMIdx]->GetName(), *BoneName.ToString());
						bAlreadyExists = true;
					}
					if (bAlreadyExists)
					{
						// Remove from array
						ExistingBoneMonitors.RemoveAt(BMIdx);					
						break;
					}
				}
	
				if (!bAlreadyExists)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Creating new bone monitor %s.."),
						*FString(__FUNCTION__), __LINE__, *BoneName.ToString());

					// Mark actor as modified
					OnwerActor->Modify();

					// Get the set of owned components that exists prior to instancing the new component.
					TInlineComponentArray<UActorComponent*> PreInstanceComponents;
					OnwerActor->GetComponents(PreInstanceComponents);

					// Create a new component
					USLBoneContactMonitor* NewComp = NewObject<USLBoneContactMonitor>(GetOwner(), BoneName, RF_Transactional);
					NewComp->SetAttachedBoneNameChecked(BoneName);
					if (!NewComp->AttachToBone())
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d Could not attach %s::%s to bone, this should not happen.."),
							*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *NewComp->GetName());
					}

					// Make visible in the components list in the editor
					OnwerActor->AddInstanceComponent(NewComp);
					OnwerActor->AddOwnedComponent(NewComp);

					NewComp->RegisterComponent();

					// Register any new components that may have been created during construction of the instanced component, but were not explicitly registered.
					TInlineComponentArray<UActorComponent*> PostInstanceComponents;
					OnwerActor->GetComponents(PostInstanceComponents);
					for (UActorComponent* ActorComponent : PostInstanceComponents)
					{
						if (!ActorComponent->IsRegistered() && ActorComponent->bAutoRegister && !ActorComponent->IsPendingKill() && !PreInstanceComponents.Contains(ActorComponent))
						{
							ActorComponent->RegisterComponent();
						}
					}

					OnwerActor->RerunConstructionScripts();
				}
			}

			// Removing monitors without corresponding bones
			for (const auto& DanglingMonitor : ExistingBoneMonitors)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Bone monitor %s has no bone skeletal bone, please remove.."),
					*FString(__FUNCTION__), __LINE__, *DanglingMonitor->GetName());
				/*OnwerActor->Modify();
				OnwerActor->RemoveOwnedComponent(DanglingMonitor);
				OnwerActor->RemoveInstanceComponent(DanglingMonitor);
				OnwerActor->ConditionalBeginDestroy();*/
			}
		}
	}
}

/* Begin grasp related */
#if SL_WITH_MC_GRASP
// Subscribe to grasp type changes
bool USLManipulatorMonitor::SubscribeToGraspTypeChanges()
{
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(UMCGraspAnimController::StaticClass()))
	{
		UMCGraspAnimController* GraspAnimController = CastChecked<UMCGraspAnimController>(AC);
		GraspAnimController->OnGraspType.AddUObject(this, &USLManipulatorMonitor::OnGraspType);
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
			if (bUseGraspHelper && GraspHelper.bUseUserInput)
			{
				IC->BindAction(GraspHelper.InputActionName, IE_Pressed, this, &USLManipulatorMonitor::GraspHelperInputCallback);				
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
void USLManipulatorMonitor::OnGroupAGraspContactBegin(USLBaseIndividual* OtherIndividual, const FName& BoneName)
{
	if (int32* NumContacts = GroupANumGraspContacts.Find(OtherIndividual))
	{
		// Already in contact with the group, increase the number of contacts
		(*NumContacts)++;
		if (bLogVerboseGraspDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s is already in contact with group, new num=%d.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
		}
	}
	else
	{
		// First contact with group, check if a new grasp is triggered
		GroupANumGraspContacts.Add(OtherIndividual, 1);
		if (bLogVerboseGraspDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s's first contact with group.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
		}

		// Make sure the individual is not already grasped
		if (GraspedIndividuals.Contains(OtherIndividual))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s::%s \t\t GroupA: %s is already grasped, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
			return;
		}

		// Set bone name
		if (bUseGraspHelper && GraspHelper.GraspHelpType == ESLGraspHelperType::ABGroup)
		{
			GraspHelper.SetBoneNameGroupA(OtherIndividual->GetParentActor(), BoneName);
		}

		// If the individual is in contact with the other group as well, trigger a grasp start event
		if (GroupBNumGraspContacts.Contains(OtherIndividual))
		{
			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s is in contact with other group as well, triggering grasp event.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
			}
			// Trigger grasp started event
			GraspStarted(OtherIndividual);
		}
		else
		{
			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s is NOT in contact with other group, grasp will not be triggered.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
			}
		}
	}
}

// Process beginning of grasp in group B
void USLManipulatorMonitor::OnGroupBGraspContactBegin(USLBaseIndividual* OtherIndividual, const FName& BoneName)
{
	if (int32* NumContacts = GroupBNumGraspContacts.Find(OtherIndividual))
	{
		// Already in contact with the group, increase the number of contacts
		(*NumContacts)++;
		if (bLogVerboseGraspDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s is already in contact with group, new num=%d.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
		}
	}
	else
	{
		// First contact with group, check if a new grasp is triggered
		GroupBNumGraspContacts.Add(OtherIndividual, 1);
		if (bLogVerboseGraspDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s's first contact with group.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
		}

		// Make sure the individual is not already grasped
		if (GraspedIndividuals.Contains(OtherIndividual))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s::%s \t\t GroupB: %s is already grasped, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
			return;
		}

		// Set bone name
		if (bUseGraspHelper && GraspHelper.GraspHelpType == ESLGraspHelperType::ABGroup)
		{
			GraspHelper.SetBoneNameGroupB(OtherIndividual->GetParentActor(), BoneName);
		}

		// If the individual is in contact with the other group as well, trigger a grasp start event
		if (GroupANumGraspContacts.Contains(OtherIndividual))
		{
			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s is in contact with other group as well, triggering grasp event.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
			}
			// Trigger grasp started event
			GraspStarted(OtherIndividual);
		}
		else
		{
			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s is NOT in contact with other group, grasp will not be triggered.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
			}
		}
	}
}

// Process ending of contact in group A
void USLManipulatorMonitor::OnGroupAGraspContactEnd(USLBaseIndividual* OtherIndividual, const FName& BoneName)
{
	if (int32* NumContacts = GroupANumGraspContacts.Find(OtherIndividual))
	{
		// Decrease the number of contacts
		(*NumContacts)--;

		// Check if this was the last contact with the group
		if ((*NumContacts) <= 0)
		{
			// Remove individual from group
			GroupANumGraspContacts.Remove(OtherIndividual);

			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s last contact with the group.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
			}

			// Clear bone name
			if (bUseGraspHelper && GraspHelper.GraspHelpType == ESLGraspHelperType::ABGroup)
			{
				GraspHelper.ClearBoneNameGroupA(OtherIndividual->GetParentActor());
			}

			// If currently in contact with the other group as well, it should be grasped, trigger grasp end
			if (GroupBNumGraspContacts.Contains(OtherIndividual))
			{
				// Make sure the individual is grasped
				if (GraspedIndividuals.Contains(OtherIndividual))
				{
					if (bLogVerboseGraspDebug)
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s is in the other group as well, triggering grasp end.."),
							*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
							*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
					}
					// Trigger grasp end event
					GraspEnded(OtherIndividual);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s::%s \t\t GroupA: %s is NOT grasped, this should not happen.."),
						*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
						*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
				}
			}
		}
		else
		{
			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupA: %s contact num decreased to Num=%d.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s::%s \t\t GroupA: %s is not in the contact list, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
	}
}

// Process ending of contact in group B
void USLManipulatorMonitor::OnGroupBGraspContactEnd(USLBaseIndividual* OtherIndividual, const FName& BoneName)
{
	if (int32* NumContacts = GroupBNumGraspContacts.Find(OtherIndividual))
	{
		// Decrease the number of contacts
		(*NumContacts)--;

		// Check if this was the last contact with the group
		if ((*NumContacts) <= 0)
		{
			// Remove individual from group
			GroupBNumGraspContacts.Remove(OtherIndividual);

			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s last contact with the group.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
			}

			// Clear bone name
			if (bUseGraspHelper && GraspHelper.GraspHelpType == ESLGraspHelperType::ABGroup)
			{
				GraspHelper.ClearBoneNameGroupB(OtherIndividual->GetParentActor());
			}

			// If currently in contact with the other group as well, it should be grasped, trigger grasp end
			if (GroupANumGraspContacts.Contains(OtherIndividual))
			{
				// Make sure the individual is grasped
				if (GraspedIndividuals.Contains(OtherIndividual))
				{
					if (bLogVerboseGraspDebug)
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s is in the other group as well, triggering grasp end.."),
							*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
							*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
					}
					// Trigger grasp end event
					GraspEnded(OtherIndividual);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s::%s \t\t GroupB: %s is NOT grasped, this should not happen.."),
						*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
						*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
					return;
				}
			}
		}
		else
		{
			if (bLogVerboseGraspDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f %s::%s \t\t GroupB: %s contact num decreased to Num=%d.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName(), *NumContacts);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s::%s \t\t GroupB: %s is not in the contact list, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetOwner()->GetName(), *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
	}
}

// Bind bone grasp contact callbacks 
void USLManipulatorMonitor::BindGraspContactCallbacks()
{
	for (auto BoneMonitor : BoneMonitorsGroupA)
	{
		BoneMonitor->OnBeginGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnGroupAGraspContactBegin);
		BoneMonitor->OnEndGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnGroupAGraspContactEnd);
	}
	for (auto BoneMonitor : BoneMonitorsGroupB)
	{
		BoneMonitor->OnBeginGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnGroupBGraspContactBegin);
		BoneMonitor->OnEndGraspBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnGroupBGraspContactEnd);
	}
}

// Unbind bone grasp contact callbacks
void USLManipulatorMonitor::UnbindGraspContactCallbacks()
{
	// TODO
}

// A grasp has started
void USLManipulatorMonitor::GraspStarted(USLBaseIndividual* OtherIndividual)
{
	// Check if it is a new grasp event, or a concatenation with a previous one, either way, there is a new grasp
	GraspedIndividuals.Emplace(OtherIndividual);
	if(!IsAJitterGrasp(OtherIndividual, GetWorld()->GetTimeSeconds()))
	{
		if (bLogGraspDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Grasp Started ( !!! broadcast !!! ): \t\t %s::%s->%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetName(), *OtherIndividual->GetParentActor()->GetName());
		}

		// Broadcast grasp event
		OnBeginManipulatorGrasp.Broadcast(OwnerIndividualObject, OtherIndividual, GetWorld()->GetTimeSeconds(), ActiveGraspType);

		// Check if the grasped actor should be helped
		if (bUseGraspHelper)
		{
			GraspHelper.CheckStartGraspHelp(OtherIndividual->GetParentActor());
		}
	}
	else if (bLogGraspDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Grasp Started (jitter grasp - cancelled): \t\t %s::%s->%s;"),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetOwner()->GetName(), *GetName(), *OtherIndividual->GetParentActor()->GetName());
	}
}

// A grasp has ended
void USLManipulatorMonitor::GraspEnded(USLBaseIndividual* OtherIndividual)
{
	if (GraspedIndividuals.Remove(OtherIndividual) > 0)
	{
		if (bLogGraspDebug)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Grasp Ended (jitter check started): \t\t %s::%s->%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetName(), *OtherIndividual->GetParentActor()->GetName());
		}

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
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s->%s is not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *OtherIndividual->GetParentActor()->GetName());
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
			if (bLogGraspDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Grasp Ended ( !!! broadcast !!! with delay): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *EvItr->Other->GetParentActor()->GetName());
			}

			// Broadcast delayed event
			OnEndManipulatorGrasp.Broadcast(OwnerIndividualObject, EvItr->Other, EvItr->Time);

			// Check if the grasp helper should be ended
			if (bUseGraspHelper)
			{
				GraspHelper.CheckEndGraspHelp(EvItr->Other->GetParentActor());
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
bool USLManipulatorMonitor::IsAJitterGrasp(USLBaseIndividual* OtherIndividual, float StartTime)
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
void USLManipulatorMonitor::OnBoneContactBegin(USLBaseIndividual* OtherIndividual, const FName& BoneName)
{
	if (int32* NumContacts = ManipulatorNumContacts.Find(OtherIndividual))
	{
		(*NumContacts)++;
	}
	else
	{
		// Check if it is a new contact event, or a concatenation with a previous one, either way, there is a new contact
		ManipulatorNumContacts.Add(OtherIndividual, 1);
		const float CurrTime = GetWorld()->GetTimeSeconds();
		if(!IsAJitterContact(OtherIndividual, CurrTime))
		{
			if (bLogContactDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Contact Started ( !!! broadcast !!! ): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *OtherIndividual->GetParentActor()->GetName());
			}

			// Broadcast begin of semantic overlap event
			OnBeginManipulatorContact.Broadcast(FSLContactResult(OwnerIndividualObject, OtherIndividual, CurrTime, false));
		}
		else if (bLogContactDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %.4fs \t\t Contact Started (jitter contact - cancelled): \t\t %s::%s->%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *GetName(), *OtherIndividual->GetParentActor()->GetName());
		}
	}
}

// Process ending of contact
void USLManipulatorMonitor::OnBoneContactEnd(USLBaseIndividual* OtherIndividual, const FName& BoneName)
{
	if (int32* NumContacts = ManipulatorNumContacts.Find(OtherIndividual))
	{
		(*NumContacts)--;
			
		if ((*NumContacts) <= 0)
		{
			// Remove contact object
			ManipulatorNumContacts.Remove(OtherIndividual);

			if (!GetWorld())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d The world is finished.. this should not happen (often).."),
					*FString(__FUNCTION__), __LINE__);
				// Episode already finished, continuing would be futile
				return;
			}

			if (bLogContactDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Contact Ended (jitter check started): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *OtherIndividual->GetParentActor()->GetName());
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

// Bind bone contact callbacks 
void USLManipulatorMonitor::BindContactCallbacks()
{
	for (auto BoneMonitor : BoneMonitorsGroupA)
	{
		BoneMonitor->OnBeginContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnBoneContactBegin);
		BoneMonitor->OnEndContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnBoneContactEnd);
	}
	for (auto BoneMonitor : BoneMonitorsGroupB)
	{
		BoneMonitor->OnBeginContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnBoneContactBegin);
		BoneMonitor->OnEndContactBoneOverlap.AddUObject(this, &USLManipulatorMonitor::OnBoneContactEnd);
	}

	// TODO (slower) change to DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLBoneOverlapBeginSignature, USLBaseIndividual*, Other);
	//if (BoneMonitor->OnBeginContactBoneOverlap.IsAlreadyBound(this, &USLManipulatorMonitor::OnBoneContactBegin)
	//	|| BoneMonitor->OnEndContactBoneOverlap.IsAlreadyBound(this, &USLManipulatorMonitor::OnBoneContactEnd))
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs Grasp %s::%s's %s callback already bound, this should not happen.."),
	//		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
	//		*GetOwner()->GetName(), *GetName(), *BoneMonitor->GetName());
	//}
	//else
	//{
	//	BoneMonitor->OnBeginContactBoneOverlap.AddDynamic(this, &USLManipulatorMonitor::OnBoneContactBegin);
	//	BoneMonitor->OnEndContactBoneOverlap.AddDynamic(this, &USLManipulatorMonitor::OnBoneContactEnd);
	//}
}

// Unbind contact callbacks
void USLManipulatorMonitor::UnbindContactCallbacks()
{
	// TODO
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
			if (bLogContactDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t Contact Ended ( !!! broadcast !!! with delay): \t\t %s::%s->%s;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *GetName(), *EvItr->Other->GetParentActor()->GetName());
			}

			// Broadcast contact event
			OnEndManipulatorContact.Broadcast(OwnerIndividualObject, EvItr->Other, EvItr->Time);

			// Remove from the pending list
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
bool USLManipulatorMonitor::IsAJitterContact(USLBaseIndividual* InOther, float StartTime)
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


///* Begin grasp helper */
//// Setup the grasp helper constraint
//bool USLManipulatorMonitor::InitGraspHelper()
//{
//	if (ASkeletalMeshActor* AsSkelMA = Cast<ASkeletalMeshActor>(GetOwner()))
//	{
//		GraspHelperOwnerSkelMC = AsSkelMA->GetSkeletalMeshComponent();
//		if (GraspHelperOwnerSkelMC->GetBoneIndex(GraspHelperDefaultBoneAttachmentName) != INDEX_NONE)
//		{
//			GraspHelperConstraint = CreateGraspHelperConstraint("GraspHelperConstraint");
//			if (bGraspHelperUseGroupABConstraints)
//			{
//				GraspHelperConstraintGroupA = CreateGraspHelperConstraint("GraspHelperConstraintA");
//				GraspHelperConstraintGroupB = CreateGraspHelperConstraint("GraspHelperConstraintB");
//			}
//			return true;
//		}
//		else
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Owner (%s) attachment bone %s does not exist.. aborting grasp help.."),
//				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GraspHelperDefaultBoneAttachmentName.ToString());
//			return false;
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not a skeletal component.. aborting grasp help.."), *FString(__FUNCTION__), __LINE__);
//		return false;
//	}
//}
//
//// Create and register the grasp helper constraint
//UPhysicsConstraintComponent* USLManipulatorMonitor::CreateGraspHelperConstraint(const FName& Name)
//{
//	// Create and init the ad hoc grasp helper constraint
//	UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(this, Name);
//	Constraint->RegisterComponent();
//	Constraint->AttachToComponent(GraspHelperOwnerSkelMC,
//		FAttachmentTransformRules::SnapToTargetIncludingScale, GraspHelperDefaultBoneAttachmentName);
//
//	Constraint->ConstraintInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, GraspHelperConstraintLimit);
//	Constraint->ConstraintInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, GraspHelperConstraintLimit);
//	Constraint->ConstraintInstance.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, GraspHelperConstraintLimit);
//
//	Constraint->ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, GraspHelperConstraintLimit);
//	Constraint->ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, GraspHelperConstraintLimit);
//	Constraint->ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, GraspHelperConstraintLimit);
//
//	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
//	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = GraspHelperConstraintStiffness;
//	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.Damping = GraspHelperConstraintDamping;
//	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.ContactDistance = GraspHelperConstraintContactDistance;
//
//	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = true;
//	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = GraspHelperConstraintStiffness;
//	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Damping = GraspHelperConstraintDamping;
//	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.ContactDistance = GraspHelperConstraintContactDistance;
//
//	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = true;
//	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = GraspHelperConstraintStiffness;
//	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Damping = GraspHelperConstraintDamping;
//	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.ContactDistance = GraspHelperConstraintContactDistance;
//
//	if (bGraspHelperConstraintParentDominates)
//	{
//		Constraint->ConstraintInstance.EnableParentDominates();
//	}
//	if (bLogGraspHelpDebug)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Initialized %s's grasp helper constraint %s .."),
//			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *Constraint->GetName());
//	}
//	return Constraint;
//}

// Ad hoc manual override
void USLManipulatorMonitor::GraspHelperInputCallback()
{
	GraspHelper.UserInputCallback();
	//if (bGraspHelperCanExecuteManualOverrideFlag)
	//{
	//	if (!bIsGraspHelpActive)
	//	{
	//		if (bLogGraspHelpDebug)
	//		{
	//			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s's start grasp helper manual override.."),
	//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	//		}
	//		StartGraspHelper();
	//	}
	//	else
	//	{
	//		if (bLogGraspHelpDebug)
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4f \t\t %s's stop grasp helper manual override.."),
	//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	//		}
	//		StopGraspHelper();
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s has nothing to manually override.."),
	//		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	//}
}
//
//// Called from the grasp started callback function
//void USLManipulatorMonitor::GraspHelperStartLogic(AActor* OtherActor)
//{
//	if (GraspHelperOtherSMC)
//	{
//		if (bLogGraspHelpDebug)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s's grasp helper item is already set %s, ignoring this trigger (%s) .."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//				*GetOwner()->GetName(), *GraspHelperOtherSMC->GetOwner()->GetName(), *OtherActor->GetName());
//		}
//		return;
//	}
//
//	// Ignore if there is a delay timer running
//	if (bDelayGraspHelper && GetWorld()->GetTimerManager().IsTimerActive(GraspHelperDelayTimerHandle))
//	{
//		if (bLogGraspDebug)
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t %s's delay grasp call is still waiting, ignoring this trigger (%s) .."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//				*GetOwner()->GetName(), *GetName(), *OtherActor->GetName());
//		}
//		return;
//	}
//
//	// Check if other actor can be helped
//	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(OtherActor))
//	{
//		if (!ShouldGraspHelperBeAppliedToActor(OtherActor))
//		{
//			if (bLogGraspHelpDebug)
//			{
//				UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4f \t\t %s grasped actor %s does not fulfill grasp helper criterias, skippin.."),
//					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *OtherActor->GetName());
//			}
//			return;
//		}
//
//		if (!bGraspHelperManualOverride)
//		{
//			// Set the grasped item (or soon the be grasped)
//			GraspHelperOtherSMC = AsSMA->GetStaticMeshComponent();
//
//			// Check if the helper should happen now or after a delay
//			if (bDelayGraspHelper)
//			{
//				if (bLogGraspHelpDebug)
//				{
//					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s grasped actor set %s starting helper (with delay).."),
//						*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *OtherActor->GetName());
//				}
//				GetWorld()->GetTimerManager().SetTimer(GraspHelperDelayTimerHandle, this,
//					&USLManipulatorMonitor::DelayedStartGraspHelper, GraspHelperDelay, false);
//			}
//			else
//			{
//				if (bLogGraspHelpDebug)
//				{
//					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s grasped actor set %s starting helper.."),
//						*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *OtherActor->GetName());
//				}
//				StartGraspHelper();		
//			}
//		}
//		else if (!bIsGraspHelpActive)
//		{
//			if (bLogGraspHelpDebug)
//			{
//				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s grasped actor set %s manual grasp help enabled.."),
//					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *OtherActor->GetName());
//			}
//			// Allow manual grasp override
//			GraspHelperOtherSMC = AsSMA->GetStaticMeshComponent();
//			bGraspHelperCanExecuteManualOverrideFlag = true;
//		}
//	}
//	else
//	{
//		if (bLogGraspDebug)
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t %s could not help grasp %s because it is not a static mesh actor.."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//				*GetOwner()->GetName(), *GetName(), *OtherActor->GetName());
//		}
//	}
//}
//
//// Triggered from the grasp end callback function
//void USLManipulatorMonitor::GraspHelpStopTrigger(AActor* OtherActor)
//{
//	if (GraspHelperOtherSMC == nullptr)
//	{
//		if (bLogGraspHelpDebug)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s grasp helper is clear, nothing to release.."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//		}
//		return;
//	}
//
//	if (GraspHelperOtherSMC->GetOwner() != OtherActor)
//	{
//		if (bLogGraspHelpDebug)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s grasp helper triggered from another grasp (%s != %s), ignoring.."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
//				*GraspHelperOtherSMC->GetOwner()->GetName(), *OtherActor->GetName());
//		}
//		return;
//	}
//
//	// Abort any started waiting trigger
//	if (bDelayGraspHelper && GetWorld()->GetTimerManager().IsTimerActive(GraspHelperDelayTimerHandle))
//	{
//		if (bLogGraspHelpDebug)
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d \t %.4fs \t\t %s' delay grasp call is still active (%s == %s), canceling timer, aborting grasp stop.."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//				*GetOwner()->GetName(), *GraspHelperOtherSMC->GetOwner()->GetName(), *OtherActor->GetName());
//		}
//
//		// Clear timer
//		GetWorld()->GetTimerManager().ClearTimer(GraspHelperDelayTimerHandle);
//
//		// Clear any set flags
//		GraspHelperOtherSMC = nullptr;
//		bIsGraspHelpActive = false;
//		bGraspHelperCanExecuteManualOverrideFlag = false;
//		return;
//	}
//
//	if (!bGraspHelperManualOverride)
//	{
//		if (bLogGraspHelpDebug)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t calling %s's stop grasp helper on %s.."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//				*GetOwner()->GetName(), *GraspHelperOtherSMC->GetOwner()->GetName());
//		}
//		StopGraspHelper();
//	}
//	else if (!bIsGraspHelpActive)
//	{
//		if (bLogGraspHelpDebug)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s manual grasp can now be manually disabled.."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//		}
//		// Cancel the manual override
//		bGraspHelperCanExecuteManualOverrideFlag = false;
//	}
//}
//
//// Call start grasp helper with a delay
//void USLManipulatorMonitor::DelayedStartGraspHelper()
//{
//	if (bLogGraspHelpDebug)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t calling  %s's start grasp helper after delay.."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//	}
//	StartGraspHelper();
//}
//
//// Start grasp help
//void USLManipulatorMonitor::StartGraspHelper()
//{
//	if (bIsGraspHelpActive)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasp help is already active, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//		return;
//	}
//
//	if (!GraspHelperOtherSMC || !GraspHelperOtherSMC->IsValidLowLevel() || GraspHelperOtherSMC->IsPendingKillOrUnreachable())
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasped object is not valid, cannot start helping, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//		return;
//	}
//
//	// Apply the properties to the grasped object
//	if (bGraspHelperItemDisableGravity)
//	{
//		GraspHelperOtherSMC->SetEnableGravity(false);
//	}
//
//	if (bGraspHelperItemScaleMass)
//	{
//		GraspHelperOtherSMC->SetMassScale(NAME_None, GraspHelperItemMassScaleValue);
//	}
//
//	// Set the constraint
//	GraspHelperConstraint->SetConstrainedComponents(GraspHelperOwnerSkelMC,
//		GraspHelperDefaultBoneAttachmentName, GraspHelperOtherSMC, NAME_None);
//
//	if (bGraspHelperUseGroupABConstraints)
//	{
//		if (!GraspHelperBoneNameGroupA.IsNone() && !GraspHelperBoneNameGroupB.IsNone())
//		{
//			if (GraspHelperOwnerSkelMC->GetBoneIndex(GraspHelperBoneNameGroupA) != INDEX_NONE
//				&& GraspHelperOwnerSkelMC->GetBoneIndex(GraspHelperBoneNameGroupB) != INDEX_NONE)
//			{
//				// Attach to hand bones and create the constraint
//				GraspHelperConstraintGroupA->AttachToComponent(GraspHelperOwnerSkelMC,
//					FAttachmentTransformRules::SnapToTargetIncludingScale, GraspHelperBoneNameGroupA);
//				GraspHelperConstraintGroupA->SetConstrainedComponents(GraspHelperOwnerSkelMC,
//					GraspHelperBoneNameGroupA, GraspHelperOtherSMC, NAME_None);
//
//				GraspHelperConstraintGroupB->AttachToComponent(GraspHelperOwnerSkelMC,
//					FAttachmentTransformRules::SnapToTargetIncludingScale, GraspHelperBoneNameGroupB);
//				GraspHelperConstraintGroupB->SetConstrainedComponents(GraspHelperOwnerSkelMC,
//					GraspHelperBoneNameGroupB, GraspHelperOtherSMC, NAME_None);
//			}
//			else
//			{
//				UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4f \t\t %s's group AB bones (A=%s;B=%s) are not in the owner skeletal component, this should not happen .."),
//					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//					*GetOwner()->GetName(), *GraspHelperBoneNameGroupA.ToString(), *GraspHelperBoneNameGroupB.ToString());
//			}
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4f \t\t %s's group AB bones (A=%s;B=%s) are not set, this should not happen .."),
//				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//				*GetOwner()->GetName(), *GraspHelperBoneNameGroupA.ToString(), *GraspHelperBoneNameGroupB.ToString());
//		}
//	}
//
//	if (bLogGraspHelpDebug)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t %.4f \t\t %s's grasp help is active on %s.."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//			*GetOwner()->GetName(), *GraspHelperOtherSMC->GetOwner()->GetName());
//	}
//	bIsGraspHelpActive = true;
//}
//
//// Stop grasp help
//void USLManipulatorMonitor::StopGraspHelper()
//{
//	if (!bIsGraspHelpActive)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasp help is already stopped, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//		// Even though this should not happen make sure the flags are in order
//		GraspHelperConstraint->BreakConstraint();
//		if (bGraspHelperUseGroupABConstraints)
//		{
//			GraspHelperConstraintGroupA->BreakConstraint();
//			GraspHelperConstraintGroupB->BreakConstraint();
//		}
//		GraspHelperOtherSMC = nullptr;
//		bIsGraspHelpActive = false;
//		bGraspHelperCanExecuteManualOverrideFlag = false;
//		return;
//	}
//
//	if (!GraspHelperOtherSMC || !GraspHelperOtherSMC->IsValidLowLevel() || GraspHelperOtherSMC->IsPendingKillOrUnreachable())
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d [%.4f] %s's grasped object is not valid, cannot reset the grasped object values, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
//		// Even though this should not happen make sure the flags are in order
//		GraspHelperConstraint->BreakConstraint();
//		if (bGraspHelperUseGroupABConstraints)
//		{
//			GraspHelperConstraintGroupA->BreakConstraint();
//			GraspHelperConstraintGroupB->BreakConstraint();
//		}
//		GraspHelperOtherSMC = nullptr;
//		bIsGraspHelpActive = false;
//		bGraspHelperCanExecuteManualOverrideFlag = false;
//		return;
//	}
//
//	// Reset the properties to the grasped object
//	if (bGraspHelperItemDisableGravity)
//	{
//		GraspHelperOtherSMC->SetEnableGravity(true);
//	}
//	if (bGraspHelperItemScaleMass)
//	{
//		GraspHelperOtherSMC->SetMassScale(NAME_None, 1.f);
//	}
//
//	if (bLogGraspHelpDebug)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %.4f \t\t %s's grasp help disabled on %s .."),
//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
//			*GetOwner()->GetName(), *GraspHelperOtherSMC->GetOwner()->GetName());
//	}
//
//	// Clear grasped individual
//	GraspHelperConstraint->BreakConstraint();
//	if (bGraspHelperUseGroupABConstraints)
//	{
//		GraspHelperConstraintGroupA->BreakConstraint();
//		GraspHelperConstraintGroupB->BreakConstraint();
//	}
//	GraspHelperOtherSMC = nullptr;
//	bIsGraspHelpActive = false;
//	bGraspHelperCanExecuteManualOverrideFlag = false;
//}
//
//// Check if the object should be helped
//bool USLManipulatorMonitor::ShouldGraspHelperBeAppliedToActor(AActor* Actor)
//{
//	// Check if object has a contact area
//	for (const auto& C : Actor->GetComponentsByClass(UShapeComponent::StaticClass()))
//	{
//		if (C->GetName().StartsWith("SLContactMonitor"))
//		{
//			return true;
//		}
//	}
//	return false;
//}
///* End grasp helper */
