// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLBaseIndividual.generated.h"

// Forward declaration
class AActor;

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
UCLASS(ClassGroup = SL)
class USLBaseIndividual : public UObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLBaseIndividual();

	// Called before destroying the object.
	virtual void BeginDestroy() override;

	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	virtual void PostInitProperties() override;

	// Init asset references (bReset forces re-initialization)
	virtual bool Init(bool bReset = false);

	// Load semantic data (bReset forces re-loading)
	virtual bool Load(bool bReset = false, bool bTryImportFromTags = false);

	// Return states
	bool IsInit() const { return bIsInit; };
	bool IsLoaded() const { return bIsLoaded; };

	// Save data to owners tag, true if any new value is written
	virtual bool ExportToTag(bool bOverwrite = false);

	// Load data from owners tag, true if any new value if imported
	virtual bool ImportFromTag(bool bOverwrite = false);

	// Get actor represented by the individual
	AActor* GetParentActor() const { return ParentActor; };

	// Check that the parent actor is set and valid
	bool HasValidParentActor() const { return ParentActor && ParentActor->IsValidLowLevel() && !ParentActor->IsPendingKill(); };

	// True if individual is part of another individual
	bool IsPartOfAnotherIndividual() const 
	{ 
		return PartOfActor && PartOfActor->IsValidLowLevel() && !PartOfActor->IsPendingKill()
			&& PartOfIndividual && PartOfIndividual->IsValidLowLevel() && !PartOfIndividual->IsPendingKill();
	}

	// Return the actor this individual is part of
	class AActor* GetPartOfActor() const { return PartOfActor; };

	// Return the individual this individual is part of
	class USLBaseIndividual* GetPartOfIndividual() const { return PartOfIndividual; };

	// Get the type name as string
	virtual FString GetTypeName() const { return FString("BaseIndividual"); };

	/* Id */
	// Set the id value, if empty, reset the individual as not loaded
	void SetId(const FString& NewId);
	void ClearId() { SetId(""); };
	FString GetId() const { return Id; };
	bool HasId() const { return !Id.IsEmpty(); };

	/* Class */
	// Set the class value, if empty, reset the individual as not loaded
	void SetClass(const FString& NewClass);
	void ClearClass() { SetClass(""); };
	FString GetClass() const { return Class; };
	bool HasClass() const { return !Class.IsEmpty(); };

protected:
	// Clear all values of the individual
	virtual void InitReset();

	// Clear all data of the individual
	virtual void LoadReset();

	// Clear any bound delegates (called when init is reset)
	virtual void ClearDelegateBounds();

	// Set the state flags, can broadcast on new value
	void SetIsInit(bool bNewValue, bool bBroadcast = true);
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

private:
	// States implementations, set references and data
	bool InitImpl();
	bool LoadImpl(bool bTryImportFromTags = true);

	// Specific imports from tag, true if new value is written
	bool ImportIdFromTag(bool bOverwrite = false);
	bool ImportClassFromTag(bool bOverwrite = false);

public:
	// Public delegates
	FSLIndividualInitChangeSignature OnInitChanged;
	FSLIndividualInitChangeSignature OnLoadedChanged;
	FSLIndividualNewIdSignature OnNewIdValue;
	FSLIndividualNewClassSignature OnNewClassValue;

protected:
	// Pointer to the actor described by the semantic description class
	UPROPERTY(VisibleAnywhere, Category = "SL")
	AActor* ParentActor;

	// Individual unique id
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FString Id;

	// Idividual class
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FString Class;
	
	// True if individual is initialized
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsInit : 1;

	// True if individual is loaded
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsLoaded : 1;

	// Actor attached to
	UPROPERTY(VisibleAnywhere, Category = "SL")
	AActor* PartOfActor;

	// Individual of the attached to actor
	UPROPERTY(VisibleAnywhere, Category = "SL")
	USLBaseIndividual* PartOfIndividual;

	/* Constants */
	// Tag type for exporting/importing data from tags
	static constexpr char TagTypeConst[] = "SemLog";
};
