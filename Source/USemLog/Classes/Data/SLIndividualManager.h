// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLIndividualManager.generated.h"

// Forward declaration
class USLIndividualComponent;

UCLASS()
class USEMLOG_API ASLIndividualManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLIndividualManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Load components from world
	int32 Init(bool bReset = false);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

	// Re-load and re-register individual components
	void Refresh() { Init(true); };

	// Add new semantic data components to the actors
	int32 AddIndividualComponents();
	int32 AddIndividualComponents(const TArray<AActor*>& Actors);

	// Remove all semantic data components
	int32 DestroyIndividualComponents();
	int32 DestroyIndividualComponents(const TArray<AActor*>& Actors);

	// Reload components data
	int32 ReloadIndividualComponents();
	int32 ReloadIndividualComponents(const TArray<AActor*>& Actors);

	/* Functionalities */
	// Toggle perceivable individuals mask materials
	int32 ToggleMaskMaterialsVisibility();
	int32 ToggleMaskMaterialsVisibility(const TArray<AActor*>& Actors);

	// Write new unique identifiers 
	int32 WriteUniqueIds(bool bOverwrite = false);
	int32 WriteUniqueIds(const TArray<AActor*>& Actors, bool bOverwrite = false);
	int32 RemoveUniqueIds();
	int32 RemoveUniqueIds(const TArray<AActor*>& Actors);

	// Write class names
	int32 WriteClassNames(bool bOverwrite = false);
	int32 WriteClassNames(const TArray<AActor*>& Actors, bool bOverwrite = false);
	int32 RemoveClassNames();
	int32 RemoveClassNames(const TArray<AActor*>& Actors);

	// Write visual masks
	int32 WriteVisualMasks(bool bOverwrite = false);
	int32 WriteVisualMasks(const TArray<AActor*>& Actors, bool bOverwrite = false);
	int32 RemoveVisualMasks();
	int32 RemoveVisualMasks(const TArray<AActor*>& Actors);

	// Export/imort data to/from tags
	int32 ExportToTag(bool bOverwrite = false);
	int32 ExportToTag(const TArray<AActor*>& Actors, bool bOverwrite = false);
	int32 ImportFromTag(bool bOverwrite = false);
	int32 ImportFromTag(const TArray<AActor*>& Actors, bool bOverwrite = false);

private:
	// Triggered by external destruction of individual component
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualComponent* DestroyedComponent);

	// Triggered by external destruction of semantic owner 
	UFUNCTION()
	void OnSemanticOwnerDestroyed(AActor* DestroyedActor);

	// Find the individual component of the actor, return nullptr if none found
	USLIndividualComponent* GetIndividualComponent(AActor* Actor);

	// Create and add new individual component
	USLIndividualComponent* AddNewIndividualComponent(AActor* Actor);

	// Check if actor type is supported for creating an individual component
	bool CanHaveIndividualComponents(AActor* Actor);

	// Remove individual component from owner
	void DestroyIndividualComponent(USLIndividualComponent* Component);

	// Cache component, bind delegates
	bool RegisterIndividualComponent(USLIndividualComponent* Component);

	// Remove component from cache, unbind delegates
	bool UnregisterIndividualComponent(USLIndividualComponent* Component);

	// Unregister and clear all cached components (return the number of cleared components)
	int32 ClearIndividualComponents();

	// Check if component is registered (one check)
	bool IsIndividualComponentRegisteredFast(USLIndividualComponent* Component) const { return RegisteredIndividualComponents.Contains(Component); };

	// Check if component is registered (full check)
	bool IsIndividualComponentRegisteredFull(USLIndividualComponent* Component) const;

private:
	// Marks manager as initialized
	bool bIsInit;

	// Cached components
	TSet<USLIndividualComponent*> RegisteredIndividualComponents;
	TMap<AActor*, USLIndividualComponent*> IndividualComponentOwners;
	// TODO
	// TSet<ISLIndividualComponent*> PerceivableIndividualComponents;

	// TSet<ISLIndividualComponent*> RigidIndividualComponents;
	// TSet<ISLIndividualComponent*> SkeletalIndividualComponents;
	// TSet<ISLIndividualComponent*> RobotIndividualComponents;

	// TSet<ISLIndividualComponent*> ConstraintIndividualComponents;
};

