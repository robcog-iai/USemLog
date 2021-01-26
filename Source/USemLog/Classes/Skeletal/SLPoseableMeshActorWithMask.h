// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Skeletal/SLPoseableMeshActor.h"
#include "SLPoseableMeshActorWithMask.generated.h"

class USkeletalMesh;
class ASkeletalMeshActor;
class UPoseableMeshComponent;

UCLASS()
class ASLPoseableMeshActorWithMask : public ASLPoseableMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLPoseableMeshActorWithMask();

	// Setup the poseable mesh
	virtual void SetSkeletalMesh(USkeletalMesh* SkelMesh) override;

	// Setup the poseable mesh from a skeletal mesh actor
	virtual void SetSkeletalMeshAndPose(ASkeletalMeshActor* SkelMeshActor) override;

	// Apply bone transformations
	virtual void SetSkeletalPose(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose) override;

	// Set a custom material on the skeletal mesh at the given index
	void SetCustomMaterial(int32 ElementIndex, UMaterialInterface* Material);

	// Set a custom material on all the indexes
	void SetCustomMaterial(UMaterialInterface* Material);

	// Show mask
	void ShowMask();

	// Show original
	void ShowOriginal();

	// Returns PoseableMeshComponent subobject 
	UPoseableMeshComponent* GetPoseableMeshComponentMask() const { return PoseableMeshComponentMask; }

protected:
	// Clone of the skeletal mesh component with movable capabilities
	UPROPERTY(EditAnywhere)
	UPoseableMeshComponent* PoseableMeshComponentMask;

};
