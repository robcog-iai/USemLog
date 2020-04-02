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
	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	virtual void PostInitProperties() override;

	// Save data to owners tag
	virtual bool SaveToTag(bool bOverwrite = false);

	// Load data from owners tag
	virtual bool LoadFromTag(bool bOverwrite = false);

	// All properties are set for runtime
	virtual bool IsRuntimeReady() const;

	// Set/get semantic owner
	void SetSemanticOwner(AActor* InSemOwner) { SemanticOwner = InSemOwner; };
	AActor* GetSemanticOwner() const { return SemanticOwner; };

protected:
	// Pointer to the actor described by the semantic description class
	class AActor* SemanticOwner;
	
	/* Constants */
	static constexpr char TagTypeConst[] = "SemLog";
};
