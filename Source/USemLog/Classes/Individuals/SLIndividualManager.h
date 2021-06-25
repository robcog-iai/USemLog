// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "HAL/ThreadSafeBool.h"
#include "SLIndividualManager.generated.h"

// Forward declaration
class USLIndividualComponent;
class USLBaseIndividual;
class USLSkeletalIndividual;
class USLRobotIndividual;

UCLASS(ClassGroup = (SL), DisplayName = "SL Individual Manager")
class USEMLOG_API ASLIndividualManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLIndividualManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Set the individual references
	bool Init(bool bReset);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

	// Check if all individuals are loaded
	bool Load(bool bReset);

	// True if all individuals are loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Listen to individual component delegates
	bool Connect();

	// True if the manager is listening to the component delegates (transient)
	bool IsConnected() const { return bIsConnected; };

	// True if the containers are currently not modified and it is safe to read from other threads
	bool ThreadSafeToRead() const { return bThreadSafeToRead; };

	// Get all the individual components
	const TArray<USLIndividualComponent*>& GetIndividualComponents() const { return IndividualComponents; };

	// Get all the individuals
	const TArray<USLBaseIndividual*>& GetIndividuals() const { return Individuals; };

	// Get skeletal individuals
	const TArray<USLSkeletalIndividual*>& GetSkeletalIndividuals() const { return SkeletalIndividuals; };

	// Get robot individuals
	const TArray<USLRobotIndividual*>& GetRobotIndividuals() const { return RobotIndividuals; };

	// Get the individual object from the unique id
	USLBaseIndividual* GetIndividual(const FString& Id);

	// Get the individual component from the unique id
	USLIndividualComponent* GetIndividualComponent(const FString& Id);

	// Get the individual component owner from the unique id
	AActor* GetIndividualActor(const FString& Id);

	// Spawn or get manager from the world
	static ASLIndividualManager* GetExistingOrSpawnNew(UWorld* World);

protected:
	// Clear all cached references
	void InitReset();

	// 
	void LoadReset();

	// Set the init flag, broadcast on new value
	void SetIsInit(bool bNewValue, bool bBroadcast  = true);

	// Set the loaded flag, broadcast on new value
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Set the connected flag, broadcast on new value
	void SetIsConnected(bool bNewValue, bool bBroadcast = true);

private:
	// Cache references
	bool InitImpl();

	// True if all cached individuals are loaded
	bool LoadImpl();

	// Bind to the cached individual component delegates
	bool BindDelegates();

	// Remove bounds from the cached individuals
	bool UnbindDelegates();

	// Check if there are any cached elements
	bool HasCache() const;

	// Remove any chached individuals
	void ClearCache();

	// Add individual info to cache
	void AddToCache(USLIndividualComponent* IC);

	// Remove from cache
	bool RemoveFromCache(USLIndividualComponent* IC);

	// Triggered by external destruction of individual component
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualComponent* DestroyedComponent);


private:
	// True if the manager is init
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the manager is loaded
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;

	// True if listening to the individual components delegates
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsConnected : 1;

	// Set to to false if the cache is being modified to avoid iterating the containers from other threads
	FThreadSafeBool bThreadSafeToRead;

	/** Containers **/
	// The individual components in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<USLIndividualComponent*> IndividualComponents;

	// All individuals
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<USLBaseIndividual*> Individuals;


	/* World state logger convenient containers */
	// Individuals with movable mobility
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<USLBaseIndividual*> MovableIndividuals;

	// Individuals without children, and ones that cannot be children
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<USLBaseIndividual*> ChildlessRootIndividuals;

	// Skeletal individuals
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<USLSkeletalIndividual*> SkeletalIndividuals;

	// Robot individuals
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<USLRobotIndividual*> RobotIndividuals;


	/* Id based quick access mappings */
	// Id to individual object
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<FString, USLBaseIndividual*> IdToIndividuals;

	// Id to 
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<FString, USLIndividualComponent*> IdToIndividualComponents;





	/* Editor button hacks */
	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bInitButton = false;

	// Triggers a call to load
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bLoadButton = false;

	// Triggered call reset argument
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bResetFlagButton = false;

	// Triggers a reset call
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bResetButton = false;

	// Searches for id value in the world
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	FString FindIdValue = "";

	// Triggers searching for the id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bFindIdButton = false;

	// Toggle between visualizing the visual mask
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bToggleVisualMaskVisiblityButton = false;
};

