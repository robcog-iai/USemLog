// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVisionPoseableMeshActor.generated.h"

class ASkeletalMeshActor;
class UPoseableMeshComponent;

UCLASS()
class ASLVisionPoseableMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisionPoseableMeshActor();

	// Setup the poseable mesh
	bool Init(ASkeletalMeshActor* SkMA);

	// Check if the actor is initialized
	bool IsInit() const { return bIsInit; };

	// Apply bone transformations
	void SetBoneTransforms(const TMap<FName, FTransform>& BoneTransfroms);
	
private:
	// Init flab
	bool bIsInit;

	// Clone of the skeletal mesh component with movable capabilities
	UPROPERTY()
	UPoseableMeshComponent* PoseableMeshComponent;

};
