// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLContactPoolSingleton.generated.h"

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

	void Foo();

	void Register(class USLContactListener* ContactListener);

	void OnNewBeginSemanticContact(AActor* OtherActor, const FString& Id);
	
	void OnNewEndSemanticContact(AActor* OtherActor, const FString& Id);


private:
	// Set if new semantic contact event is triggered
	bool bNewData;
};
