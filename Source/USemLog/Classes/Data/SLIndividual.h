// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLIndividualBase.h"
#include "SLIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USLIndividual : public USLIndividualBase
{
	GENERATED_BODY()

public:
	// Ctor
	USLIndividual();

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
	virtual bool ExportToTag(bool bOverwrite = false) override;

	// Load data from owners tag
	virtual bool ImportFromTag(bool bOverwrite = false) override;

	// Set get Id
	void SetId(const FString& InId) { Id = InId; };
	FString GetId() const { return Id; };
	bool HasId() const { return !Id.IsEmpty(); };

	// Set get class
	void SetClass(const FString& InClass) { Class = InClass; };
	FString GetClass() const { return Class; };
	bool HasClass() const { return !Class.IsEmpty(); };

private:
	// Private init implementation
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

protected:
	// Individual unique id
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Id;
	
	// Idividual class
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Class;

//private:
//	// State of the individual
//	uint8 bIsInitPrivate : 1;
//	uint8 bIsLoadedPrivate : 1;
};
