// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EventData/SLContactPublisher.h"
#include "EventData/SLSupportedByPublisher.h"
#include "SLOverlapArea.generated.h"

/**
 * Structure containing information about the semantic overlap event
 */
USTRUCT()
struct FSLOverlapResult
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

	// Flag showing if Other is also of type Semantic Overlap Area
	bool bIsSemanticOverlapArea;

	// The mesh of the other overlapping component
	TWeakObjectPtr<UMeshComponent> MeshComponent;

	// Default ctor
	FSLOverlapResult() {};

	// Helper constructor
	FSLOverlapResult(uint32 InId, const FString& InSemId, const FString& InSemClass,
		float Time,	bool bIsSemanticOverlapArea) :
		Id(InId), SemId(InSemId), SemClass(InSemClass), TriggerTime(Time), 
		bIsSemanticOverlapArea(bIsSemanticOverlapArea)
	{};

	// Helper constructor with static mesh actor and component
	FSLOverlapResult(uint32 InId, const FString& InSemId, const FString& InSemClass,
		float Time, bool bIsSemanticOverlapArea, UMeshComponent* InMeshComponent) :
		Id(InId), SemId(InSemId), SemClass(InSemClass),
		TriggerTime(Time), bIsSemanticOverlapArea(bIsSemanticOverlapArea), MeshComponent(InMeshComponent)
	{};

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("Id:%ld SemId:%s SemClass:%s TriggerTime:%f bIsSemanticOverlapArea:%s StaticMeshActor:%s StaticMeshComponent:%s"),
			Id, *SemId, *SemClass, TriggerTime,
			bIsSemanticOverlapArea == true ? TEXT("True") : TEXT("False"),
			MeshComponent.IsValid() ? *MeshComponent->GetName() : TEXT("None"));
	}
};

/** Delegate to notify that a contact begins between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_OneParam(FSLOverlapBeginSignature, const FSLOverlapResult&);

/** Delegate to notify that a contact ended between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_TwoParams(FSLOverlapEndSignature, uint32 /*OtherId*/, float /*Time*/);

/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics))
class USEMLOG_API USLOverlapArea : public UBoxComponent
{
	GENERATED_BODY()

protected:
	// Give access to private data
	friend class FSLContactPublisher;
	friend class FSLSupportedByPublisher;

public:
	// Default constructor
	USLOverlapArea();

	// Dtor
	~USLOverlapArea();

	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	void Init();

	// Start overlap events, trigger currently overlapping objects
	void Start();

	// Stop publishing overlap events
	void Finish();

protected:
	// Called at level startup
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:


#if WITH_EDITOR
	// UObject interface
	// Called after the C++ constructor and after the properties have been initialized
	virtual void PostInitProperties() override;

	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UObject interface

	// USceneComponent interface
	// Called when this component is moved in the editor
	virtual void PostEditComponentMove(bool bFinished);
	// End of USceneComponent interface

	// Load and apply cached parameters from tags
	bool LoadAreaParameters();

	// Calculate and apply trigger area size
	bool CalculateAreaParameters();

	// Save current parameters to tags
	bool SaveAreaParameters();
#endif // WITH_EDITOR

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Event called when something starts to overlaps this component
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:
	// Contact publisher
	TSharedPtr<FSLContactPublisher> SLContactPub;

	// Supported by event publisher
	TSharedPtr<FSLSupportedByPublisher> SLSupportedByPub;

private:
	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

	// Event called when a semantic overlap begins
	FSLOverlapBeginSignature OnBeginSLOverlap;

	// Event called when a semantic overlap ends
	FSLOverlapEndSignature OnEndSLOverlap;

	// Listen for contact events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bListenForContactEvents;

	// Listen for supported by events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bListenForSupportedByEvents;

	// Init and start at begin play
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtBeginPlay;

	//// Listen for sliding events
	//UPROPERTY(EditAnywhere)
	//bool bListenForSlidingEvents;

	//// Listen for pushed by events
	//UPROPERTY(EditAnywhere)
	//bool bListenForPushedByEvents;

	// Pointer to the outer (owner) mesh component 
	UMeshComponent* OwnerMeshComp;

	// Cache of the outer (owner) unique id (unreal)
	uint32 OwnerId;

	// Cache of the semlog id of the outer (owner)
	FString OwnerSemId;

	// Cache of the semantic class of the outer (owner)
	FString OwnerSemClass;
};
