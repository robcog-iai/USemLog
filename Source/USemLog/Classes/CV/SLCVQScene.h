// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "SLCVQScene.generated.h"

// Forward declaration
class ASLIndividualManager;
class ASLMongoQueryManager;

/**
 * Base class for viz queries
 */
UCLASS()
class USLCVQScene : public UDataAsset
{
	GENERATED_BODY()

public:
	// Public execute function
	void ShowScene(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager);

	// Hide executed scene
	void HideScene();

	// Get the scene name
	FString GetSceneName() const { return "DefaultSceneName"; };

	// Get the bounding sphere radius of the scene
	float GetSphereBoundsRadius() const;

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Virtual implementation of the execute function
	virtual void ShowSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager);

	// Virtual implementation of the hide executed scene function
	virtual void HideSceneImpl();

protected:
	/* Individuals in the scene */
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	TArray<FString> Ids;

	// Timestamp of the scene
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	float Timestamp;

	// Task id of the scene
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	FString Task;

	// Episode id of the scene
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	FString Episode;

	UPROPERTY(EditAnywhere, Category = "CV Scene")
	bool bIgnore;

	/* Editor interaction */
	UPROPERTY(EditAnywhere, Category = "CV Scene|Edit")
	bool bAddSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "CV Scene|Edit")
	bool bRemoveSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "CV Scene|Edit")
	bool bOverwrite = false;

	UPROPERTY(EditAnywhere, Category = "CV Scene|Edit")
	bool bEnsureUniqueness = true;

private:
	// Current cache of the actors from the ids
	TArray<AActor*> SceneActors;
};
