// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SLBoneOverlapShape.generated.h"

/**
 * Semantic overlap generator between skeletal bones at the world
 */
UCLASS()
class USEMLOGSKEL_API USLBoneOverlapShape : public USphereComponent
{
	GENERATED_BODY()

public:
	// Ctor
	USLBoneOverlapShape();

	// Dtor
	~USLBoneOverlapShape();
	
	// Attach to bone 
	bool Init(USkeletalMeshComponent* Parent, FName BoneName);

	// Start listening to overlaps
	void Start();

	// Stop publishing overlap events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

#if WITH_EDITORONLY_DATA
	// Location and orientation visualization of the component
	UPROPERTY()
	class UArrowComponent* ArrowVis;
#endif // WITH_EDITORONLY_DATA
};
