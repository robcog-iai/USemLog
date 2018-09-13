// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "EventData/SLGraspPublisher.h"
#include "Engine/StaticMeshActor.h"
#include "SLGraspTrigger.generated.h"

/**
 * Structure containing information about the semantic grasp event
 */
USTRUCT()
struct FSLGraspResult
{
	GENERATED_USTRUCT_BODY()

	// Unique UObjcet id of other
	uint32 Id;

	// Semantic id of other
	FString SemId;

	// Semantic class of other
	FString SemClass;

	// Timestamp in seconds of the event triggering
	float TriggerTime;

	// Default ctor
	FSLGraspResult() {};

	// Helper constructor
	FSLGraspResult(uint32 InId, const FString& InSemId, const FString& InSemClass, float Time) :
		Id(InId), SemId(InSemId), SemClass(InSemClass), TriggerTime(Time)
	{};

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("Id:%ld SemId:%s SemClass:%s TriggerTime:%f "),
			Id, *SemId, *SemClass, TriggerTime);
	}
};

/** Delegate to notify that a grasp begins between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_OneParam(FSLGraspBeginSignature, const FSLGraspResult&);

/** Delegate to notify that a grasp begins between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_TwoParams(FSLGraspEndSignature, uint32 /*OtherId*/, float /*Time*/);

/**
 * Grasp listener actor component
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics))
class USEMLOG_API USLGraspTrigger : public UObject
{
	GENERATED_BODY()

protected:
	// Give access to private data
	friend class FSLGraspPublisher;

public:
	// Default constructor
	USLGraspTrigger();

	// Initialize trigger, check if the hand is semantically annotated
	bool Init(USkeletalMeshComponent* InHand);

	// Finish trigger
	void Finish(float Time);

	// Start grasp
	void BeginGrasp(AStaticMeshActor* Other, float Time);

	// End grasp
	void EndGrasp(AStaticMeshActor* Other, float Time);

public:
	// Contact publisher
	TSharedPtr<FSLGraspPublisher> SLGraspPub;

private:
	// Event called when a semantic grasp begins
	FSLGraspBeginSignature OnBeginSLGrasp;

	// Event called when a semantic grasp ends
	FSLGraspEndSignature OnEndSLGrasp;

	// Cache the id of the hand (owner) unique id (unreal)
	uint32 OwnerId;

	// Cache of the semlog id of the outer (owner) (hand)
	FString OwnerSemId;

	// Cache of the semantic class of the outer (owner) (hand)
	FString OwnerSemClass;
};
