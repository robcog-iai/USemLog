// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "EventData/SLContactEvent.h"
#include "SLContactPoolSingleton.generated.h"


/** Delegate for notification of start of overlap with a semantic entity */
DECLARE_DELEGATE_OneParam(FSLContactEventSignature, FSLContactEvent*);

/**
* Semantic contact event structure
*/
struct FSLContactData
{
	float Timestamp;
	uint32 Obj1Id;
	uint32 Obj2Id;
	FString SemLogId;
};


/**
 * Listens to semantic contacts and resolves duplicates
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLContactPoolSingleton : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

private:
	// Constructor
	USLContactPoolSingleton();

public:
	// Destructor
	~USLContactPoolSingleton();

	// Get singleton
	static USLContactPoolSingleton* GetInstance();

	// Delete instance
	static void DeleteInstance();

	/** Begin FTickableGameObject interface */
	// Called after ticking all actors, DeltaTime is the time passed since the last call.
	virtual void Tick(float DeltaTime) override;

	// Return if object is ready to be ticked
	//virtual bool IsTickable() const override; 

	// Return the stat id to use for this tickable
	virtual TStatId GetStatId() const override;
	/** End FTickableGameObject interface */

	void Register(class USLContactTrigger* ContactListener);

	// 
	TArray<FSLContactEvent*> FinishPendingContactEvents();

//protected:
//	// 
//	virtual void BeginDestroy() override;
//
//	//
//	virtual void FinishDestroy() override;

private:
	// Resolve begin semantic contact duplicates
	void ResolveBeginDuplicates();

	// Resolve end semantic contact duplicates
	void ResolveEndDuplicates();

	// Publish finished semantic contact event
	void PublishFinishedEvent();


	void Foo();



	void OnNewBeginSemanticContact(AActor* OtherActor, const FString& Id);
	
	void OnNewEndSemanticContact(AActor* OtherActor, const FString& Id);


public:
	// Event called when a semantic contact event is finished
	FSLContactEventSignature OnSemanticContactEvent;

private:
	// Set if new semantic contact event is triggered
	bool bNewData;

	bool bNewBeginContact;

	bool bNewEndContact;
};
