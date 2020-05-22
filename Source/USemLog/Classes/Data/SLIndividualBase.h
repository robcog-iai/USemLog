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

	// Init asset references (bReset forces re-initialization)
	virtual bool Init(bool bReset = false);

	// Check if individual is initialized
	virtual bool IsInit() const;

	// Load semantic data (bReset forces re-loading)
	virtual bool Load(bool bReset = false);

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
	UPROPERTY(VisibleAnywhere, Category = "SL")
	class AActor* SemanticOwner;
	
	// State of the individual
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsInit : 1;

	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsLoaded : 1;

	/* Constants */
	// Tag type for exporting/importing data from tags
	static constexpr char TagTypeConst[] = "SemLog";

//private:
//	// State of the individual
//	uint8 bIsInitPrivate : 1;
//	uint8 bIsLoadedPrivate : 1;
};
