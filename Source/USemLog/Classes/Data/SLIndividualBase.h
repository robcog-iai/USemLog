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

	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	virtual void PostInitProperties() override;

	// Init asset references (bForced forces re-initialization)
	virtual bool Init(bool bForced = false);

	// Check if individual is initialized
	virtual bool IsInit() const;

	// Load semantic data (bForced forces re-loading)
	virtual bool Load(bool bForced = false);

	// Check if semantic data is succesfully loaded
	virtual bool IsLoaded() const;

	// Save data to owners tag
	virtual bool ExportToTag(bool bOverwrite = false);

	// Load data from owners tag
	virtual bool ImportFromTag(bool bOverwrite = false);

	// Get semantic owner
	AActor* GetSemanticOwner() const { return SemanticOwner; };

private:
	// Private init implementation, set the semantic owner reference
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

protected:
	// Pointer to the actor described by the semantic description class
	class AActor* SemanticOwner;
	
	/* Constants */
	static constexpr char TagTypeConst[] = "SemLog";

private:
	// State of the individual
	uint8 bIsInitPrivate : 1;
	uint8 bIsLoadedPrivate : 1;
	uint8 bIsDirty : 1; // TODO

};
