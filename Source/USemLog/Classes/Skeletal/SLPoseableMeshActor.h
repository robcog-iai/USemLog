// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLPoseableMeshActor.generated.h"

class USkeletalMesh;
class ASkeletalMeshActor;
class UPoseableMeshComponent;

UCLASS()
class ASLPoseableMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLPoseableMeshActor();

	// Setup the poseable mesh
	virtual void SetSkeletalMesh(USkeletalMesh* SkelMesh);

	// Setup the poseable mesh from a skeletal mesh actor
	virtual void SetSkeletalMeshAndPose(ASkeletalMeshActor* SkelMeshActor);

	// Apply bone transformations
	virtual void SetSkeletalPose(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose);

	// Returns PoseableMeshComponent subobject 
	UPoseableMeshComponent* GetPoseableMeshComponent() const { return PoseableMeshComponent; }

protected:
	// Clone of the skeletal mesh component with movable capabilities
	UPROPERTY(EditAnywhere)
	UPoseableMeshComponent* PoseableMeshComponent;

};
