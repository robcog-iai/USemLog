// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLBaseIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USLBaseIndividual : public UObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLBaseIndividual();

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

	// Save data to owners tag, true if any new value is written
	virtual bool ExportToTag(bool bOverwrite = false);

	// Load data from owners tag, true if any new value if imported
	virtual bool ImportFromTag(bool bOverwrite = false);

	// Get semantic owner
	AActor* GetSemanticOwner() const { return SemanticOwner; };

	/* Id */
	// Set the id value, if empty, reset the individual as not loaded
	void SetId(const FString& NewId);
	FString GetId() const { return Id; };
	FORCEINLINE bool HasId() const { return !Id.IsEmpty(); };

	/* Class */
	// Set the class value, if empty, reset the individual as not loaded
	void SetClass(const FString& NewClass);
	FString GetClass() const { return Class; };
	FORCEINLINE bool HasClass() const { return !Class.IsEmpty(); };

private:
	// Import id from tag, true if new value is written
	bool ImportIdFromTag(bool bOverwrite = false);

	// Import class from tag, true if new value is written
	bool ImportClassFromTag(bool bOverwrite = false);

	// Private init implementation, set the semantic owner reference
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

protected:
	// Pointer to the actor described by the semantic description class
	UPROPERTY(VisibleAnywhere, Category = "SL")
	class AActor* SemanticOwner;

	// Individual unique id
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FString Id;

	// Idividual class
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FString Class;
	
	// State of the individual
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsInit : 1;

	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsLoaded : 1;

	/* Constants */
	// Tag type for exporting/importing data from tags
	static constexpr char TagTypeConst[] = "SemLog";

};
