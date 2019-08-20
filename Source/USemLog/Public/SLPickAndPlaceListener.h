#pragma once
// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLStructs.h" // FSLEntity
#include "SLPickAndPlaceListener.generated.h"

/** Notify the beginning and the end of the lift/slide/transport events */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLBeginLiftSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLEndLiftSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLBeginSlideSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLEndSlideSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLBeginTransportSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLEndTransportSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);

/**
 * Checks for manipulator related events (contact, grasp, lift, transport, slide)
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL PickAndPlace Listener")
class USEMLOG_API USLPickAndPlaceListener : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLPickAndPlaceListener();

	// Dtor
	~USLPickAndPlaceListener();

	// Check if owner is valid and semantically annotated
	bool Init();

	// Start listening to grasp events, update currently overlapping objects
	void Start();

	// Stop publishing grasp events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };
	
public:
	// Lift/slide/transport events begin end 
	FSLBeginLiftSignature OnBeginManipulatorLift;
	FSLEndLiftSignature OnEndManipulatorLift;

	FSLBeginSlideSignature OnBeginManipulatorSlide;
	FSLEndSlideSignature OnEndManipulatorSlide;

	FSLBeginSlideSignature OnBeginManipulatorTransport;
	FSLEndSlideSignature OnEndManipulatorTransport;
	
private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Flags for detecting various stages of the pick and place
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bDetectLiftEvents;

	// Flags for detecting various stages of the pick and place
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bDetectSlideEvents;

	// Flags for detecting various stages of the pick and place
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bDetectTransportEvents;

	// Semantic data of the owner
	FSLEntity SemanticOwner;

	// Objects currently grasped
	TArray<AActor*> GraspedObjects;
};
