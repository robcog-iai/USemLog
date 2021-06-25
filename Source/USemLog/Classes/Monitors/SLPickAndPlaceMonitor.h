#pragma once
// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/ActorComponent.h"
#include "SLContactMonitorInterface.h"
#include "SLPickAndPlaceMonitor.generated.h"

// Forward declaration
class USLBaseIndividual;
class USLIndividualComponent;


/**
* Hand type
*/
UENUM()
enum class ESLPaPStateCheck : uint8
{
	NONE								UMETA(DisplayName = "NONE"),
	Slide								UMETA(DisplayName = "Slide"),
	PickUp								UMETA(DisplayName = "PickUp"),
	TransportOrPutDown					UMETA(DisplayName = "TransportOrPutDown"),
	PutDown								UMETA(DisplayName = "PutDown"),
	
};

/**
 * Various PaP events helpers
 */
struct FSLPaPSlide
{
public:
	// Default ctor
	FSLPaPSlide() = default;
	
	float MovedDist;

	// End time of the event 
	float StartTime;
};

/** Notify the beginning and the end of the pick and place related events */
DECLARE_MULTICAST_DELEGATE_FourParams(FSLPaPSubEventSignature, USLBaseIndividual* /*Self*/, USLBaseIndividual* /*Other*/, float /*StartTime*/, float /*EndTime*/);

/**
 * Checks for manipulator related events (contact, grasp, lift, transport, slide)
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL PickAndPlace Monitor")
class USEMLOG_API USLPickAndPlaceMonitor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLPickAndPlaceMonitor();

	// Dtor
	~USLPickAndPlaceMonitor();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame, used for timeline visualizations, activated and deactivated on request
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Check if owner is valid and semantically annotated
	void Init();

	// Start listening to grasp events, update currently overlapping objects
	void Start();

	// Stop publishing grasp events
	void Finish(float EndTime, bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Subscribe to grasp events from sibling
	bool SubscribeForGraspEvents();

	// Called on grasp begin
	void OnManipulatorGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& GraspType);

	// Called on grasp end
	void OnManipulatorGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);

	// Object released, terminate active even
	void FinishActiveEvent(float CurrTime);

	// Backtrace and check if a put-down event happened
	bool HasPutDownEventHappened(const float CurrTime,const FVector& CurrObjLocation,  uint32& OutPutDownEndIdx);

	// State update functions
	void Update_NONE();
	void Update_Slide();
	void Update_PickUp();
	void Update_TransportOrPutDown();

	// Get grasped objects contact shape component
	ISLContactMonitorInterface* GetContactMonitorComponent(AActor* Actor) const;

public:
	// PaP events delegates
	FSLPaPSubEventSignature OnManipulatorSlideEvent;
	FSLPaPSubEventSignature OnManipulatorPickUpEvent;
	FSLPaPSubEventSignature OnManipulatorPutDownEvent;
	FSLPaPSubEventSignature OnManipulatorTransportEvent;
	
private:
	// Log debug messages (non event related)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogDebug : 1;

	// Log debug messages verbose (non event related)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogVerboseDebug : 1;

	// Log all events debug messages
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogAllEventsDebug : 1;

	// Log slide event debug messages only
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogSlideDebug : 1;

	// Log pick-up event debug messages only
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogPickUpDebug : 1;

	// Log transport + put down event debug messages only
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogTransportPutDownDebug : 1;

	// Skip initialization if true
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// Minimal horizontal movement to count a as  slide event
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Slide")
	float MinSlideDistXY;

	// Minimal slide duration count a as an event
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Slide")
	float MinSlideDuration;

	// Maximal horizontal movement to count as a pick-up movement
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|PickUp")
	float MaxPickUpDistXY;

	// Minimal vertical height to count as pick-up event
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|PickUp")
	float MinPickUpHeight;

	// Maximal vertical height to stop counting as pick-up event
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|PickUp")
	float MaxPickUpHeight;

	// Minimal height to count as put-down
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|PutDown")
	float MinPutDownHeight;

	// Maximal height to start counting as put-down
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|PutDown")
	float MaxPutDownHeight;

	// Maximal horzintal distance to start counting as put-down
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|PutDown")
	float MaxPutDownDistXY;

	// True if initialized
	uint8 bIsInit : 1;

	// True if started
	uint8 bIsStarted : 1;

	// True if finished
	uint8 bIsFinished : 1;

	// Candidate check update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate;

	// Semantic data component of the owner
	USLIndividualComponent* OwnerIndividualComponent;

	// Semantic individual object
	USLBaseIndividual* OwnerIndividualObject;

	// Object currently grasped
	USLBaseIndividual* CurrGraspedIndividual;

	// Previous event check
	ESLPaPStateCheck EventCheckState;

	// Cache of various relevant locations
	FVector PrevRelevantLocation;

	// Cache of various relevant times
	float PrevRelevantTime;

	// Contact shape of the grasped object, holds information if the object is supported by a surface
	ISLContactMonitorInterface* GraspedObjectContactMonitor;
	
	/* Update function bindings */
	// Function pointer type for calling the correct update function
	typedef void(USLPickAndPlaceMonitor::*UpdateFunctionPointerType)();

	// Function pointer for the state check update callback
	UpdateFunctionPointerType UpdateFunctionPtr;

	/* PickUp related */
	// Set when the object is lifted from the supported area more than the MinPickUpHeight value
	bool bPickUpHappened;

	// The location where the object was started to be lifted (use this to compare against MaxPickUpDistXY and MaxPickUpHeight)
	FVector LiftOffLocation;

	/* PutDown related */
	// Past locations and time during transport in order to backtrace and detect put-down events
	TArray<TPair<float, FVector>> RecentMovementBuffer;

	/* Constants */
	//constexpr static float UpdateRate = 0.035f;

	// Slide
	//constexpr static float MinSlideDistXY = 9.f;
	//constexpr static float MinSlideDuration = 0.9f;
	
	// PickUp
	//constexpr static float MaxPickUpDistXY = 9.f;
	//constexpr static float MinPickUpHeight = 3.f;
	//constexpr static float MaxPickUpHeight = 12.f;

	// PutDown
	constexpr static int32 RecentMovementBufferSize = 512;
	constexpr static float RecentMovementBufferDuration = 3.3f;
	constexpr static float PutDownMovementBacktrackDuration = 1.5f;

	//constexpr static float MinPutDownHeight = 2.f;
	//constexpr static float MaxPutDownHeight = 8.f;
	//constexpr static float MaxPutDownDistXY = 9.f;
};

