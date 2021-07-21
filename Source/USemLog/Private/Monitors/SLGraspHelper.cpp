// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLGraspHelper.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/ShapeComponent.h"

// Init grasp helper
void FSLGraspHelper::Init(USLManipulatorMonitor* ManipulatorMonitor)
{
	if (bIsInit)
	{
		return;
	}

	if (!ManipulatorMonitor || !ManipulatorMonitor->IsValidLowLevel() || ManipulatorMonitor->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner monitor is not valid.. aborting grasp help.."),
			*FString(__FUNCTION__), __LINE__);
		return;
	}

	if (!ManipulatorMonitor->GetOwner()->IsA(ASkeletalMeshActor::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner monitor's owner (%s) is not a skeletal mesh actor.. aborting grasp help.."),
			*FString(__FUNCTION__), __LINE__, *ManipulatorMonitor->GetOwner()->GetName());
		return;
	}

	// Bind timer delegate function
	TimerDelegate.BindRaw(this, &FSLGraspHelper::DelayedStartHelp);

	// Set owner reference
	OwnerMM = ManipulatorMonitor;

	// Set world reference
	World = OwnerMM->GetOwner()->GetWorld();

	// Set the skeletal mesh compononent
	HandSkelMeshComp = CastChecked<ASkeletalMeshActor>(OwnerMM->GetOwner())->GetSkeletalMeshComponent();
	if (HandSkelMeshComp->GetBoneIndex(DefaultBoneName) == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Hand skeletal mesh (%s) bone %s does not exist.. aborting grasp help.."),
			*FString(__FUNCTION__), __LINE__, *GetOwnerActorName(), *DefaultBoneName.ToString());
		return;
	}

	// Setup constraints
	if (GraspHelpType == ESLGraspHelperType::Default)
	{
		DefaultConstraint = CreateGraspHelperConstraint("GraspHelperConstraint");
		DefaultConstraint->AttachToComponent(HandSkelMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, DefaultBoneName);
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Initialized %s's grasp helper with default constraint=[%s;] .."),
				*FString(__FUNCTION__), __LINE__, *GetOwnerActorName(), *DefaultConstraint->GetName());
		}
	}
	else if (GraspHelpType == ESLGraspHelperType::ABGroup)
	{
		ConstraintGroupA = CreateGraspHelperConstraint("GraspHelperConstraintA");
		ConstraintGroupB = CreateGraspHelperConstraint("GraspHelperConstraintB");
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Initialized %s's grasp helper with constraints A+B=[%s,%s;] .."),
				*FString(__FUNCTION__), __LINE__, *GetOwnerActorName(), *ConstraintGroupA->GetName(), *ConstraintGroupB->GetName());
		}
	}
	else if (GraspHelpType == ESLGraspHelperType::AllBones)
	{				
		TArray<FName> BoneNames;
		HandSkelMeshComp->GetBoneNames(BoneNames);
		for (const auto& BoneName : BoneNames)
		{					
			FName ConstraintName(*FString("GraspHelperConstraint").Append(BoneName.ToString()));					
			AllBonesConstraints.Add(CreateGraspHelperConstraint(ConstraintName), BoneName);
		}
		if (bLogDebug)
		{
			FString ConstraintsNamesStr;
			for (const auto& CPair : AllBonesConstraints)
			{
				ConstraintsNamesStr.Append(CPair.Key->GetName()).Append(";");
			}
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Initialized %s's grasp helper with all bones constraints=[%s] .."),
				*FString(__FUNCTION__), __LINE__, *GetOwnerActorName(), *ConstraintsNamesStr);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Unkown type, this should not happen.. aborting grasp help.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	bIsInit = true;
}

// Start grasp help
void FSLGraspHelper::StartGraspHelp()
{
	// Grasp should not be active when this is called
	if (bIsGraspHelpActive)
	{		
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s grasp is already active (%s), this should not happen.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(),
			*GetOwnerActorName(), *GraspedStaticMeshComp->GetOwner()->GetName());		
		return;
	}

	// Grasped static mesh should be set when this is called
	if (!GraspedStaticMeshComp || !GraspedStaticMeshComp->IsValidLowLevel() || GraspedStaticMeshComp->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's grasped static mesh component is not valid, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName());
		GraspedStaticMeshComp = nullptr;
		return;
	}

	// Apply grasp constraint type
	if (GraspHelpType == ESLGraspHelperType::Default)
	{
		DefaultConstraint->SetConstrainedComponents(HandSkelMeshComp, DefaultBoneName, GraspedStaticMeshComp, NAME_None);
	}
	else if (GraspHelpType == ESLGraspHelperType::ABGroup)
	{
		// Default names of the bones
		FName BoneNameGroupA = NAME_None;
		FName BoneNameGroupB = NAME_None;

		// Get the bone names for the given grasped actor
		if (FName* FoundName = BoneNameMapGroupA.Find(GraspedStaticMeshComp->GetOwner()))
		{
			BoneNameGroupA = *FoundName;
		}
		if (FName* FoundName = BoneNameMapGroupB.Find(GraspedStaticMeshComp->GetOwner()))
		{
			BoneNameGroupB = *FoundName;
		}

		// Make sure the bone names are set
		if (BoneNameGroupA.IsNone() || BoneNameGroupB.IsNone())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's AB group bones (A=%s;B=%s) are not set, aborting.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*BoneNameGroupA.ToString(), *BoneNameGroupB.ToString());
			return;
		}

		// Make sure the bones exist 
		if (HandSkelMeshComp->GetBoneIndex(BoneNameGroupA) == INDEX_NONE 
			|| HandSkelMeshComp->GetBoneIndex(BoneNameGroupB) == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's AB group bones (A=%s;B=%s) are not part of the skeletal mesh, aborting.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*BoneNameGroupA.ToString(), *BoneNameGroupB.ToString());
			return;
		}

		// Attach constraints to bones
		ConstraintGroupA->AttachToComponent(HandSkelMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneNameGroupA);
		ConstraintGroupB->AttachToComponent(HandSkelMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneNameGroupB);

		// Create constraints
		ConstraintGroupA->SetConstrainedComponents(HandSkelMeshComp, BoneNameGroupA, GraspedStaticMeshComp, NAME_None);
		ConstraintGroupB->SetConstrainedComponents(HandSkelMeshComp, BoneNameGroupB, GraspedStaticMeshComp, NAME_None);

	}
	else if (GraspHelpType == ESLGraspHelperType::AllBones)
	{
		for (const auto& CPair : AllBonesConstraints)
		{
			CPair.Key->SetConstrainedComponents(HandSkelMeshComp, CPair.Value, GraspedStaticMeshComp, NAME_None);
		}
	}

	// Apply the properties to the grasped object
	if (bDisableGravityOnGraspedSMC)
	{
		GraspedStaticMeshComp->SetEnableGravity(false);
	}
	if (bScaleMassOnGraspedSMC)
	{
		GraspedStaticMeshComp->SetMassScale(NAME_None, ScaleMassValue);
	}

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's grasp help is active on %s.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*GraspedStaticMeshComp->GetOwner()->GetName());
	}
	bIsGraspHelpActive = true;
}

// End grasp help
void FSLGraspHelper::EndGraspHelp()
{
	// Grasp should be active when this is called
	if (!bIsGraspHelpActive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's grasp is not active, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName());

		// Even though this should not happen make sure the grasp is clear
		if (GraspHelpType == ESLGraspHelperType::Default)
		{
			DefaultConstraint->BreakConstraint();
		}
		else if (GraspHelpType == ESLGraspHelperType::ABGroup)
		{
			ConstraintGroupA->BreakConstraint();
			ConstraintGroupB->BreakConstraint();
		}
		else if (GraspHelpType == ESLGraspHelperType::AllBones)
		{
			for (const auto& CPair : AllBonesConstraints)
			{
				CPair.Key->BreakConstraint();
			}
		}
		GraspedStaticMeshComp = nullptr;
		bIsGraspHelpActive = false;
		bWaitingForUserInputEnabled = false;
		return;
	}

	// Grasped static mesh should be set when this is called
	if (!GraspedStaticMeshComp || !GraspedStaticMeshComp->IsValidLowLevel() || GraspedStaticMeshComp->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's grasped static mesh component is not valid, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName());

		// Even though this should not happen make sure the grasp is clear
		if (GraspHelpType == ESLGraspHelperType::Default)
		{
			DefaultConstraint->BreakConstraint();
		}
		else if (GraspHelpType == ESLGraspHelperType::ABGroup)
		{
			ConstraintGroupA->BreakConstraint();
			ConstraintGroupB->BreakConstraint();
		}
		else if (GraspHelpType == ESLGraspHelperType::AllBones)
		{
			for (const auto& CPair : AllBonesConstraints)
			{
				CPair.Key->BreakConstraint();
			}
		}
		GraspedStaticMeshComp = nullptr;
		bIsGraspHelpActive = false;
		bWaitingForUserInputEnabled = false;
		return;
	}

	// Break constraints
	if (GraspHelpType == ESLGraspHelperType::Default)
	{
		DefaultConstraint->BreakConstraint();
	}
	else if (GraspHelpType == ESLGraspHelperType::ABGroup)
	{
		ConstraintGroupA->BreakConstraint();
		ConstraintGroupB->BreakConstraint();
	}
	else if (GraspHelpType == ESLGraspHelperType::AllBones)
	{
		for (const auto& CPair : AllBonesConstraints)
		{
			CPair.Key->BreakConstraint();
		}
	}

	// Apply the properties to the grasped object
	if (bDisableGravityOnGraspedSMC)
	{
		GraspedStaticMeshComp->SetEnableGravity(false);
	}
	if (bScaleMassOnGraspedSMC)
	{
		GraspedStaticMeshComp->SetMassScale(NAME_None, ScaleMassValue);
	}

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's grasp help ended on %s.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*GraspedStaticMeshComp->GetOwner()->GetName());
	}

	GraspedStaticMeshComp = nullptr;
	bIsGraspHelpActive = false;
	bWaitingForUserInputEnabled = false;
}

// Check is the grasp help should be started on the given actor
void FSLGraspHelper::CheckStartGraspHelp(AActor* Actor)
{
	if (GraspedStaticMeshComp)
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's is already grasping %s, ignoring new actor %s.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}
		return;
	}

	// Ignore if there is a delay timer running
	if (bDelayGrasp && World->GetTimerManager().IsTimerActive(DelayTimerHandle))
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's delay timer is still running on %s, ignoring new actor %s.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}
		return;
	}

	// Ignore if it is not a static mesh actor
	if (!Actor->IsA(AStaticMeshActor::StaticClass()))
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's to be grasped %s is not a static mesh actor, ignoring.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}
		return;
	}

	// Check if the actor fullfills the to be grasped criterias
	if (!ShouldBeGrasped(Actor))
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's to be grasped %s does not fullfill the grasping criterias, ignoring.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(), *Actor->GetName());
		}
		return;
	}

	// Check if the grasp should be triggered by the user
	if (!bUseUserInput)
	{
		// Set the to-be-grasped component
		GraspedStaticMeshComp = CastChecked<AStaticMeshActor>(Actor)->GetStaticMeshComponent();

		// Check if the grasp should happen now, or after a delay
		if (bDelayGrasp)
		{
			if (bLogDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's started delay timer (%.4fs) to grasp %s .."),
					*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(),
					*GetOwnerActorName(), GraspDelayValue, *Actor->GetName());
			}
			World->GetTimerManager().SetTimer(DelayTimerHandle, TimerDelegate, GraspDelayValue, false);			
		}
		else
		{
			if (bLogDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's starting grasp help on %s .."),
					*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
					*GraspedStaticMeshComp->GetOwner()->GetName(),*Actor->GetName());
			}
			StartGraspHelp();
		}
	}
	else if(!bIsGraspHelpActive)
	{
		// Set the to-be-grasped component
		GraspedStaticMeshComp = CastChecked<AStaticMeshActor>(Actor)->GetStaticMeshComponent();

		// Enable user input
		bWaitingForUserInputEnabled = true;

		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's user input grasp enabled on %s .."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}
	}
}

// Check is the grasp help should be ended on the given actor
void FSLGraspHelper::CheckEndGraspHelp(AActor* Actor)
{
	// Ignore if the grasped or to-be-grasped static mesh is not set
	if (GraspedStaticMeshComp == nullptr)
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's grasped static mesh component is not set, ignoring (%s).."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(), *Actor->GetName());
		}
		return;
	}

	// Ignore if triggered from a different grasp end event
	if (GraspedStaticMeshComp->GetOwner() != Actor)
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's triggered from a different grasp end event (%s != %s), ignoring.."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(),
				*GetOwnerActorName(), *GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}
		return;
	}

	// Check and abort any unfinished delay timer
	if (bDelayGrasp && World->GetTimerManager().IsTimerActive(DelayTimerHandle))
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's delayed grasp timer is not finished yet, clearing to-be-graspes static mesh (%s).."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(),
				*GetOwnerActorName(), *GraspedStaticMeshComp->GetOwner()->GetName());
		}

		GraspedStaticMeshComp = nullptr;
		bIsGraspHelpActive = false;
		bWaitingForUserInputEnabled = false;
		return;
	}

	// Check if the grasp should be triggered by the user
	if (!bUseUserInput)
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s ending grasp help on %s .."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}
		EndGraspHelp();
	}
	else if (!bIsGraspHelpActive)
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's user input grasp disabled on %s .."),
				*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
				*GraspedStaticMeshComp->GetOwner()->GetName(), *Actor->GetName());
		}

		// Disable user input
		bWaitingForUserInputEnabled = false;
		GraspedStaticMeshComp = nullptr;
		bIsGraspHelpActive = false;
	}
}

// Set group A bone name
void FSLGraspHelper::SetBoneNameGroupA(AActor* Actor, const FName& BoneName)
{
	if (BoneNameMapGroupA.Contains(Actor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's BoneNameGroupA %s::%s should had been empty here.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*Actor->GetName(), *BoneNameMapGroupA[Actor].ToString());
	}

	BoneNameMapGroupA.Emplace(Actor, BoneName);

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's BoneNameGroupA set to %s::%s.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*Actor->GetName(), *BoneName.ToString());
	}
}

// Clear group A bone name
void FSLGraspHelper::ClearBoneNameGroupA(AActor* Actor)
{
	if (!BoneNameMapGroupA.Contains(Actor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's BoneNameGroupA %s::NONE should had been set here.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),*Actor->GetName());
	}

	BoneNameMapGroupA.Remove(Actor);

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's BoneNameGroupA cleared (%s::NONE) .."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(), *Actor->GetName());
	}
}

// Set group B bone name
void FSLGraspHelper::SetBoneNameGroupB(AActor* Actor, const FName& BoneName)
{
	if (BoneNameMapGroupB.Contains(Actor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's BoneNameGroupB %s::%s should had been empty here.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*Actor->GetName(), *BoneNameMapGroupB[Actor].ToString());
	}

	BoneNameMapGroupB.Emplace(Actor, BoneName);

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's BoneNameGroupB set to %s::%s.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*Actor->GetName(), *BoneName.ToString());
	}
}

// Clear group B bone name
void FSLGraspHelper::ClearBoneNameGroupB(AActor* Actor)
{
	if (!BoneNameMapGroupB.Contains(Actor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f %s's BoneNameGroupB %s::NONE should had been set here.."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(), *Actor->GetName());
	}

	BoneNameMapGroupB.Remove(Actor);

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's BoneNameGroupB cleared (%s::NONE) .."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(), *Actor->GetName());
	}
}

// Delayd all for start help
void FSLGraspHelper::DelayedStartHelp()
{
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s starting grasp (after delay) .."),
			*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *GetOwnerActorName(),
			*GraspedStaticMeshComp->GetOwner()->GetName());
	}
	StartGraspHelp();
}

// Create and register the grasp helper constraint
UPhysicsConstraintComponent* FSLGraspHelper::CreateGraspHelperConstraint(const FName& Name)
{
	UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(OwnerMM, Name);
	Constraint->RegisterComponent();

	Constraint->ConstraintInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, ConstraintLimit);
	Constraint->ConstraintInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, ConstraintLimit);
	Constraint->ConstraintInstance.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, ConstraintLimit);

	Constraint->ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, ConstraintLimit);
	Constraint->ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, ConstraintLimit);
	Constraint->ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, ConstraintLimit);

	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = ConstraintStiffness;
	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.Damping = ConstraintDamping;
	Constraint->ConstraintInstance.ProfileInstance.LinearLimit.ContactDistance = ConstraintContactDistance;

	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = true;
	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = ConstraintStiffness;
	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Damping = ConstraintDamping;
	Constraint->ConstraintInstance.ProfileInstance.ConeLimit.ContactDistance = ConstraintContactDistance;

	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = true;
	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = ConstraintStiffness;
	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Damping = ConstraintDamping;
	Constraint->ConstraintInstance.ProfileInstance.TwistLimit.ContactDistance = ConstraintContactDistance;

	if (bConstraintParentDominates)
	{
		Constraint->ConstraintInstance.EnableParentDominates();
	}

	return Constraint;
}

// Check if actor can/should be grasped
bool FSLGraspHelper::ShouldBeGrasped(AActor* Actor) const
{
	// Check if object has a contact area
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
	TArray<UActorComponent*> Components;
	Actor->GetComponents(UShapeComponent::StaticClass(), Components);
	for (const auto C : Components)
	{
		if (C->GetName().StartsWith("SLContactMonitor"))
		{
			return true;
		}
	}
#else
	for (const auto& C : Actor->GetComponentsByClass(UShapeComponent::StaticClass()))
	{
		if (C->GetName().StartsWith("SLContactMonitor"))
		{
			return true;
		}
	}
#endif
	return false;
}

/* Debug helper functions */
// Get owner actor name
 FString FSLGraspHelper::GetOwnerActorName() const
{
	return OwnerMM->GetOwner()->GetName();
}
