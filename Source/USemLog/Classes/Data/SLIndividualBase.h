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
	// Ctor
	USLIndividualBase();

	// Destructor
	~USLIndividualBase();

private:
	// Pointer to the actor described by the semantic description class
	AActor* Owner;
};
