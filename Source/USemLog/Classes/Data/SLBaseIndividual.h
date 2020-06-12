// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLBaseIndividual.generated.h"

// Notify every time the init status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualInitChangeSignature, USLBaseIndividual*, Individual, bool, bNewInitVal);

// Notify every time the loaded status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualLoadedChangeSignature, USLBaseIndividual*, Individual, bool, bNewLoadedVal);

// Notify every time the id changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualNewIdSignature, USLBaseIndividual*, Individual, const FString&, NewId);

// Notify every time the class changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualNewClassSignature, USLBaseIndividual*, Individual, const FString&, NewClass);

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
	bool IsInit() const { return bIsInit; };

	// Load semantic data (bReset forces re-loading)
	virtual bool Load(bool bReset = false);

	// Check if semantic data is succesfully loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Save data to owners tag, true if any new value is written
	virtual bool ExportToTag(bool bOverwrite = false);

	// Load data from owners tag, true if any new value if imported
	virtual bool ImportFromTag(bool bOverwrite = false);

	// Get semantic owner
	AActor* GetSemanticOwner() const { return SemanticOwner; };

	// Get the type name as string
	virtual FString GetTypeName() const { return FString("BaseIndividual"); };

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

protected:
	// Set the init flag, broadcast on new value
	void SetIsInit(bool bNewValue);

	// Set the loaded flag, broadcast on new value
	void SetIsLoaded(bool bNewValue);

private:
	// Import id from tag, true if new value is written
	bool ImportIdFromTag(bool bOverwrite = false);

	// Import class from tag, true if new value is written
	bool ImportClassFromTag(bool bOverwrite = false);

	// Private init implementation, set the semantic owner reference
	bool InitImpl();

	// Private load implementation
	bool LoadImpl(bool bTryImportFromTags = true);

public:
	// Called when the init status changes
	FSLIndividualInitChangeSignature OnInitChanged;

	// Called when the init status changes
	FSLIndividualInitChangeSignature OnLoadedChanged;

	// Called when the id value
	FSLIndividualNewIdSignature OnNewIdValue;

	// Called when the class value
	FSLIndividualNewClassSignature OnNewClassValue;

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
