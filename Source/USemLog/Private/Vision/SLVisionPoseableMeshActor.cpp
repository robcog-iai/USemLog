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
	
	//UPoseableMeshComponent* MeshComponent = NewObject<UPoseableMeshComponent>(parent->GetOwner(), *name);
	//MeshComponent->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	//MeshComponent->RegisterComponent();

	//MeshComponent->bCastDynamicShadow = false;
	//MeshComponent->CastShadow = false;
	//MeshComponent->bRenderCustomDepth = false;
	//MeshComponent->bRenderInMainPass = true;
	//MeshComponent->SetVisibility(false, true);

	//AddMeshComponent(assetID, MeshComponent);

	//return MeshComponent;
	//return true;
}

// Apply bone transformations
void ASLVisionPoseableMeshActor::SetBoneTransforms(const TMap<FName, FTransform>& BoneTransfroms)
{
	for(const auto& Pair : BoneTransfroms)
	{
		PoseableMeshComponent->SetBoneTransformByName(Pair.Key, Pair.Value, EBoneSpaces::WorldSpace);
	}
}
