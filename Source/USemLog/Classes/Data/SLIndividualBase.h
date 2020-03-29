// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLIndividualBase.generated.h"

/**
 * 
 */
UCLASS()
class USLIndividualBase : public UObject
{
	GENERATED_BODY()

public:
	// Save data to owners tag
	virtual bool SaveToTag(bool bOverwrite = false);

	// Load data from owners tag
	virtual bool LoadFromTag(bool bOverwrite = false);

	// All properties are set for runtime
	//virtual bool IsRuntimeReady() const { return Owner == nullptr; };

	// Set/get owner
	void SetOwner(AActor* InOwner) { Owner = InOwner; };
	AActor* GetOwner() const { return Owner; };

protected:
	// Pointer to the actor described by the semantic description class
	AActor* Owner;
};
