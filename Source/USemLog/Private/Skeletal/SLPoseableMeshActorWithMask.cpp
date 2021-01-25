// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Skeletal/SLPoseableMeshActorWithMask.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ASLPoseableMeshActorWithMask::ASLPoseableMeshActorWithMask()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create the poseable mesh and set it as root
	PoseableMeshComponentMask = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMeshComponentMask"));
	PoseableMeshComponentMask->bPerBoneMotionBlur = false;
	PoseableMeshComponentMask->bHasMotionBlurVelocityMeshes = false;
	PoseableMeshComponentMask->SetVisibility(false);
	PoseableMeshComponentMask->SetupAttachment(GetRootComponent());
}

// Setup the poseable mesh component
void ASLPoseableMeshActorWithMask::SetSkeletalMesh(USkeletalMesh* SkelMesh)
{
	Super::SetSkeletalMesh(SkelMesh);	
	PoseableMeshComponentMask->SetSkeletalMesh(SkelMesh);
}

// Setup the poseable mesh from a skeletal mesh actor
void ASLPoseableMeshActorWithMask::SetSkeletalMeshAndPose(ASkeletalMeshActor* SkelMeshActor)
{
	Super::SetSkeletalMeshAndPose(SkelMeshActor);
	SetActorTransform(SkelMeshActor->GetTransform());
	if (auto* SkelMeshComp = SkelMeshActor->GetSkeletalMeshComponent())
	{		
		PoseableMeshComponentMask->SetWorldTransform(SkelMeshComp->GetComponentTransform());
		PoseableMeshComponentMask->SetSkeletalMesh(SkelMeshComp->SkeletalMesh);
	}
}

// Apply bone transformations
void ASLPoseableMeshActorWithMask::SetSkeletalPose(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose)
{
	Super::SetSkeletalPose(SkeletalPose);
	for (int32 Idx = 0; Idx < 5; ++Idx)
	{
		for (const auto& BonePosePair : SkeletalPose.Value)
		{
			const FName BoneName = PoseableMeshComponentMask->GetBoneName(BonePosePair.Key);
			PoseableMeshComponentMask->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
		}
	}
}

// Set a custom material on the skeletal mesh at the given index
void ASLPoseableMeshActorWithMask::SetCustomMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	if(PoseableMeshComponentMask)
	{
		PoseableMeshComponentMask->SetMaterial(ElementIndex, Material);
	}
}

// Set a custom material on all the indexes
void ASLPoseableMeshActorWithMask::SetCustomMaterial(UMaterialInterface* Material)
{
	if (PoseableMeshComponentMask)
	{
		for (int32 MatIdx = 0; MatIdx < PoseableMeshComponentMask->GetNumMaterials(); ++MatIdx)
		{
			PoseableMeshComponentMask->SetMaterial(MatIdx, Material);
		}
	}
}

// Show mask
void ASLPoseableMeshActorWithMask::ShowMask()
{
	PoseableMeshComponent->SetVisibility(false);
	PoseableMeshComponentMask->SetVisibility(true);
}

// Show original
void ASLPoseableMeshActorWithMask::ShowOriginal()
{
	PoseableMeshComponent->SetVisibility(true);
	PoseableMeshComponentMask->SetVisibility(false);
}
