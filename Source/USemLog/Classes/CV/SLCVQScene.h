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
class AStaticMeshActor;
class ASkeletalMeshActor;
class UStaticMeshComponent;
class UPoseableMeshComponent;

/**
 * Base class for viz queries
 */
UCLASS()
class USLCVQScene : public UDataAsset
{
	GENERATED_BODY()

public:
	// Set the scene actors and cache their relative transforms to the world root
	bool InitScene(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager);

	// Visualize scene
	void ShowScene();

	// Generate mask clones
	void GenerateMaskClones(const TCHAR* MaterialPath, bool bUseInternalMask = true, FColor MaskColor = FColor::White);

	// Show mask values of the scenes
	void ShowMaskMaterials();

	// Show original material
	void ShowOriginalMaterials();

	// Hide executed scene
	void HideScene();

	// Get the scene name (if not set, it will generate a random one)
	FString GetSceneName();

	// Get the bounding sphere radius of the scene
	float GetSphereBoundsRadius() const;

	// Get the actors in the scene
	void GetSceneActors(TArray<AActor*>& OutActors) const {	SceneActorPoses.GenerateKeyArray(OutActors); }

	// Get the ids
	TArray<FString> GetIds() const { return Ids; };

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Virtual implementation for the scene initialization
	virtual bool InitSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager);

public:
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	bool bIgnore;

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

	// Scene name
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	FString SceneName;

	// Database Server Ip
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	FString MongoIp = TEXT("127.0.0.1");

	// Database server port num
	UPROPERTY(EditAnywhere, Category = "CV Scene")
	uint16 MongoPort = 27017;

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
	// Cache of the actors and their poses
	TMap<AActor*, FTransform> SceneActorPoses;

	// Static mesh mask clones
	UPROPERTY(Transient)
	TMap<AStaticMeshActor*, UStaticMeshComponent*> MaskClones;

	// Skeletal mesh mask clones
	UPROPERTY(Transient)
	TMap<ASkeletalMeshActor*, UPoseableMeshComponent*> SkelMaskClones;
};
