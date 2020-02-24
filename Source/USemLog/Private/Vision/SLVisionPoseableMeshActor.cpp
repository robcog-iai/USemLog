// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionPoseableMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ASLVisionPoseableMeshActor::ASLVisionPoseableMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create the poseable mesh and set it as root
	PoseableMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMeshComponent"));
	PoseableMeshComponent->bPerBoneMotionBlur = false;	
	RootComponent = PoseableMeshComponent;
	
	bIsInit = false;
}

// Setup the poseable mesh component
bool ASLVisionPoseableMeshActor::Init(ASkeletalMeshActor* SkMA)
{
	if(!bIsInit)
	{
		if(USkeletalMeshComponent* SkMC = SkMA->GetSkeletalMeshComponent())
		{
			// Move the actor to the skeletal actor
			SetActorTransform(SkMA->GetTransform());

			// Create the poseable mesh component from the skeletal mesh
			PoseableMeshComponent->SetSkeletalMesh(SkMC->SkeletalMesh);

			bIsInit = true;
			return true;
		}
	}
	return false;
}

// Apply bone transformations
void ASLVisionPoseableMeshActor::SetBoneTransforms(const TMap<FName, FTransform>& BoneTransfroms)
{
	for(const auto& Pair : BoneTransfroms)
	{
		PoseableMeshComponent->SetBoneTransformByName(Pair.Key, Pair.Value, EBoneSpaces::WorldSpace);
		
		//// Hack to cut off hands
		//if (Pair.Key.IsEqual(FName("LeftHand")) || Pair.Key.IsEqual(FName("RightHand")))
		//{
		//	FTransform Scale0(Pair.Value);
		//	Scale0.SetScale3D(FVector(0.f));
		//	PoseableMeshComponent->SetBoneTransformByName(Pair.Key, Scale0, EBoneSpaces::WorldSpace);
		//}
		//else
		//{
		//	PoseableMeshComponent->SetBoneTransformByName(Pair.Key, Pair.Value, EBoneSpaces::WorldSpace);
		//}
	}
}

// Set a custom material on the skeletal mesh at the given index
bool ASLVisionPoseableMeshActor::SetCustomMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	if(bIsInit)
	{
		PoseableMeshComponent->SetMaterial(ElementIndex, Material);
		return true;
	}
	return false;
}
