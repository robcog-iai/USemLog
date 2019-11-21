// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVisionPoseableMeshActor.generated.h"

class UPoseableMeshComponent;

UCLASS()
class ASLVisionPoseableMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisionPoseableMeshActor();

	// Setup the poseable mesh
	bool Init(USkeletalMeshComponent* SkeletalMesh);

	// Check if the actor is initialized
	bool IsInit() const { return bIsInit; };
	
private:
	// Init flab
	bool bIsInit;
	
	UPROPERTY()
	UPoseableMeshComponent* PoseableMesh;

};
