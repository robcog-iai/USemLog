// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Skeletal/SLPoseableMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ASLPoseableMeshActor::ASLPoseableMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create the poseable mesh and set it as root
	PoseableMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMeshComponent"));
	PoseableMeshComponent->bPerBoneMotionBlur = false;
//	PoseableMeshComponent->bHasMotionBlurVelocityMeshes = false;
	RootComponent = PoseableMeshComponent;
}

// Setup the poseable mesh component
void ASLPoseableMeshActor::SetSkeletalMesh(USkeletalMesh* SkelMesh)
{
	// Create the poseable mesh component from the skeletal mesh
	PoseableMeshComponent->SetSkeletalMesh(SkelMesh);
}

// Setup the poseable mesh from a skeletal mesh actor
void ASLPoseableMeshActor::SetSkeletalMeshAndPose(ASkeletalMeshActor* SkelMeshActor)
{
	SetActorTransform(SkelMeshActor->GetTransform());
	if (auto* SkelMeshComp = SkelMeshActor->GetSkeletalMeshComponent())
	{		
		PoseableMeshComponent->SetWorldTransform(SkelMeshComp->GetComponentTransform());
		PoseableMeshComponent->SetSkeletalMesh(SkelMeshComp->SkeletalMesh);
	}
}

// Apply bone transformations
void ASLPoseableMeshActor::SetSkeletalPose(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose)
{
	SetActorTransform(SkeletalPose.Key);
	for (int32 Idx = 0; Idx < 5; ++Idx)
	{
		for (const auto& BonePosePair : SkeletalPose.Value)
		{
			const FName BoneName = PoseableMeshComponent->GetBoneName(BonePosePair.Key);
			PoseableMeshComponent->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
		}
	}
}
