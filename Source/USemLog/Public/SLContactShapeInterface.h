// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Engine/StaticMeshActor.h"
#include "SLStructs.h"
#include "SLContactShapeInterface.generated.h"

UINTERFACE(Blueprintable)
class USLContactShapeInterface : public UInterface
{
	GENERATED_BODY()
};

class ISLContactShapeInterface
{
	GENERATED_BODY()

//public:
//	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
//	virtual void Init() = 0;
//
//	// Start publishing overlap events, trigger currently overlapping objects
//	virtual void Start() = 0;
//
//	// Stop publishing overlap events
//	virtual void Finish(bool bForced = false) = 0;
//
//	// Get init state
//	bool IsInit() const { return bIsInit; };
//
//	// Get started state
//	bool IsStarted() const { return bIsStarted; };
//
//	// Get finished state
//	bool IsFinished() const { return bIsFinished; };

protected:
#if WITH_EDITOR
	//// Load and apply cached parameters from tags
	//virtual bool LoadShapeBounds2() = 0;

	//// Calculate and apply trigger area size
	//virtual bool CalcShapeBounds2() = 0;

	//// Save current parameters to tags
	//virtual bool StoreShapeBounds2() = 0;
#endif // WITH_EDITOR

	// Event called when something starts to overlaps this component
	UFUNCTION()
	virtual void OnOverlapBegin2(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	virtual void OnOverlapEnd2(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

protected:
	//// True if initialized
	//bool bIsInit;

	//// True if started
	//bool bIsStarted;

	//// True if finished
	//bool bIsFinished;

	// Pointer to the outer (owner) mesh component 
	UMeshComponent* OwnerMeshComp2;

	// Semantic data of the owner
	FSLEntity SemanticOwner2;
};