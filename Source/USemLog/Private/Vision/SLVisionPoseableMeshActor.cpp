// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionPoseableMeshActor.h"

// Sets default values
ASLVisionPoseableMeshActor::ASLVisionPoseableMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
}

// Setup the poseable mesh component
bool ASLVisionPoseableMeshActor::Init(USkeletalMeshComponent* SkeletalMesh)
{
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
	return true;
}
