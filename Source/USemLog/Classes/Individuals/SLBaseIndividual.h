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
	virtual bool Load(bool bReset = false, bool bTryImport = false);

	// Depenencies set
	bool IsInit() const { return bIsInit; };

	// Data loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Save values externally
	virtual bool ExportValues(bool bOverwrite = false);

	// Load values externally
	virtual bool ImportValues(bool bOverwrite = false);

	// Get actor represented by the individual
	AActor* GetParentActor() const { return ParentActor; };

	// True if individual is part of another individual
	bool IsAttachedToAnotherIndividual() const;

	// Return the actor this individual is part of
	class AActor* GetPartOfActor() const { return AttachedToActor; };

	// Return the individual this individual is part of
	class USLBaseIndividual* GetPartOfIndividual() const { return AttachedToIndividual; };

	// Get the type name as string
	virtual FString GetTypeName() const { return FString("BaseIndividual"); };

	/* Id */
	// Set the id value, if empty, reset the individual as not loaded
	void SetIdValue(const FString& NewVal);
	void ClearIdValue() { SetIdValue(""); };
	FString GetIdValue() const { return Id; };
	bool IsIdValueSet() const { return !Id.IsEmpty(); };

	/* Class */
	// Set the class value, if empty, reset the individual as not loaded
	void SetClassValue(const FString& NewClass);
	void ClearClassValue() { SetClassValue(""); };
	FString GetClassValue() const { return Class; };
	bool IsClassValueSet() const { return !Class.IsEmpty(); };

protected:
	// Clear all references of the individual
	virtual void InitReset();

	// Clear all data of the individual
	virtual void LoadReset();

	// Clear any bound delegates (called when init is reset)
	virtual void ClearDelegateBounds();

	// Set the state flags, can broadcast on new value
	void SetIsInit(bool bNewValue, bool bBroadcast = true);
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);
	
	// Check that the parent actor is set and valid
	bool HasValidParentActor() const;

	// Set pointer to parent actor
	virtual bool SetParentActor();

	// Set attachment parent (part of individual)
	bool SetAttachedToParent();

private:
	// Set dependencies
	bool InitImpl();

	// Set data
	bool LoadImpl(bool bTryImport = true);

	// Import values expernally
	bool ImportIdValue(bool bOverwrite = false);
	bool ImportClassValue(bool bOverwrite = false);

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
	AActor* AttachedToActor;

	// Individual of the attached to actor
	UPROPERTY(VisibleAnywhere, Category = "SL")
	USLBaseIndividual* AttachedToIndividual;

	/* Constants */
	// Tag type for exporting/importing data from tags
	static constexpr char TagTypeConst[] = "SemLog";
};
