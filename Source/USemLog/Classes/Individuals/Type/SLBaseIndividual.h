// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//#if SL_WITH_LIBMONGO_C
//THIRD_PARTY_INCLUDES_START
//	#if PLATFORM_WINDOWS
//	#include "Windows/AllowWindowsPlatformTypes.h"
//	#include <mongoc/mongoc.h>
//	#include "Windows/HideWindowsPlatformTypes.h"
//	#else
//	#include <mongoc/mongoc.h>
//	#endif // #if PLATFORM_WINDOWS
//THIRD_PARTY_INCLUDES_END
//#endif //SL_WITH_LIBMONGO_C
#include "SLBaseIndividual.generated.h"

// Notify every time the init status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualInitChangeSignature, USLBaseIndividual*, Individual, bool, bNewInitVal);

// Notify every time the loaded status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualLoadedChangeSignature, USLBaseIndividual*, Individual, bool, bNewLoadedVal);

// Notify every time an individual value changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSLIndividualNewValueSignature, USLBaseIndividual*, Individual, const FString&, Key, const FString&, Value);

// Notify listeners that the delegates have been cleared
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLIndividualDelegatesClearedSignature, USLBaseIndividual*, Individual);

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

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Called before destroying the object.
	virtual void BeginDestroy() override;

	// Init asset references (bReset forces re-initialization)
	virtual bool Init(bool bReset);

	// Load semantic data (bReset forces re-loading)
	virtual bool Load(bool bReset, bool bTryImport);

	// Depenencies set
	bool IsInit() const { return bIsInit; };

	// Data loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Trigger values as new value broadcast
	virtual void TriggerValuesBroadcast();

	// Save values externally
	virtual bool ExportValues(bool bOverwrite = false);

	// Load values externally
	virtual bool ImportValues(bool bOverwrite = false);

	// Clear exported values
	virtual bool ClearExportedValues();

	// Calculate and cache the individuals transform (returns true on a new value)
	virtual bool UpdateCachedPose(float Tolerance = 0.25f, FTransform* OutPose = nullptr);

	// Get the cached transform of the individual
	FTransform GetCachedPose() const { return CachedPose; };

	// Get actor represented by the individual
	AActor* GetParentActor() const { return ParentActor; };

	// True if individual is part of another individual
	bool IsAttachedToAnotherIndividual() const;

	// Return the actor this individual is part of
	class AActor* GetAttachedToActor() const { return AttachedToActor; };

	// Return the individual this individual is part of
	USLBaseIndividual* GetAttachedToIndividual() const { return AttachedToIndividual; };

	// Get all children of the individual in a newly created array
	virtual const TArray<USLBaseIndividual*> GetChildrenIndividuals() const;

	// Get the type name as string
	virtual FString GetTypeName() const { return FString("BaseIndividual"); };

	// Check if the individual is movable
	bool IsMovable() const { return bIsMovable; };

	// Get info about the individual
	virtual FString GetInfo() const;

	/* SemLog World state logger workaround helper */
	// Marks if an individual has moved since last check
	void SetHasMovedFlag(bool Val) { bHasMovedFlag = Val; };
	bool HasMovedFlagSet() const { return  bHasMovedFlag; };

	/* Id */
	// Set the id value, if empty, reset the individual as not loaded
	void SetIdValue(const FString& NewVal);
	void GenerateNewIdValue();
	void ClearIdValue() { SetIdValue(""); };
	FString GetIdValue() const { return Id; };
	bool IsIdValueSet() const { return !Id.IsEmpty(); };

	/* Class */
	// Set the class value, if empty, reset the individual as not loaded
	void SetClassValue(const FString& NewClass);
	void SetDefaultClassValue();
	void ClearClassValue() { SetClassValue(""); };
	FString GetClassValue() const { return Class; };
	bool IsClassValueSet() const { return !Class.IsEmpty(); };

	/*OId*/
	// TODO sync with Id
//	// BSON Id of the individual, used for db optimized queries
//	void SetOIdValue(const FString& NewVal);
//	void ClearOIdValue() { SetOIdValue(""); };
//	FString GetOIdValue() const { return OId; };
//	bool IsOIdValueSet() const { return !OId.IsEmpty(); };
//#if SL_WITH_LIBMONGO_C
//	bson_oid_t GetOId() const { return oid; };
//#endif // SL_WITH_LIBMONGO_C

protected:
	// Generate class name, virtual since each invidiual type will have different name
	virtual FString CalcDefaultClassValue();

	// Mark individual as init, broadcast change
	void SetIsInit(bool bNewValue, bool bBroadcast  = true);

	// Mark individual as loaded, broadcast change
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

	// Clear all references of the individual
	void InitReset();

	// Clear all data of the individual
	void LoadReset();

	// Clear any bound delegates (called when init is reset)
	void ClearDelegates();

	// Generate a new id
	FString GenerateNewId() const;

	// Import values expernally
	bool ImportIdValue(bool bOverwrite = false);
	bool ImportClassValue(bool bOverwrite = false);
	//bool ImportOIdValue(bool bOverwrite = false);

	//// Load the oid value from the persitent OId string (generate a new one if none is available)
	//bool LoadOId(bool bGenerateNew = false);

public:
	// Public delegates
	FSLIndividualInitChangeSignature OnInitChanged;
	FSLIndividualInitChangeSignature OnLoadedChanged;
	FSLIndividualNewValueSignature OnNewValue;
	FSLIndividualDelegatesClearedSignature OnDelegatesCleared;

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

	// BSON Id of the individual as persistent string
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FString OId;
	
	// True if individual is initialized
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsInit : 1;

	// True if individual is loaded
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsLoaded : 1;

	// By default every individual is movable, it might be that they are not though (e.g. sky)
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsMovable : 1;

	// Cached transform of the individual
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FTransform CachedPose;

	// Actor attached to
	UPROPERTY(VisibleAnywhere, Category = "SL")
	AActor* AttachedToActor;

	// Individual of the attached to actor
	UPROPERTY(VisibleAnywhere, Category = "SL")
	USLBaseIndividual* AttachedToIndividual;

//#if SL_WITH_LIBMONGO_C
//	// Native, non-persistent bson oid of the individual
//	bson_oid_t oid;
//#endif

	// SemLog tag key (children will overwrite this)
	UPROPERTY(VisibleAnywhere, Category = "SL")
	FString TagType;

	/* Edit buttons */
	// Reload parent pointer
	UPROPERTY(EditAnywhere, Category = "SL|Edit Buttons")
	bool bReloadParentButton = false;

	/* SemLog World state logger workaround helper */
	// Marks if an individual has moved since last check
	bool bHasMovedFlag;


};
