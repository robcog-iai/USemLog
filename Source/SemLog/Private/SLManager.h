// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/Actor.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLRawDataExporter.h"
#include "SLMapExporter.h"
#include "SLEventsExporter.h"
#include "SLManager.generated.h"

UCLASS()
class SEMLOG_API ASLManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLManager();
	
	// Actor initialization, log items init
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called when the game is terminated
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Get semantic events exporter
	FSLEventsExporter* GetEventsExporter() { return SemEventsExporter; };

private:
	// Create directory path for logging
	void CreateDirectoryPath(FString Path);

	// Set items to be loggeed (from tags)
	void InitLogItems();

	// Read previously stored unique names from file
	bool ReadPrevUniqueNames(const FString Path);

	// Set unique names of items
	void GenerateNewUniqueNames();

	// Write generated unique names to file
	void StoreNewUniqueNames(const FString Path);

	// Directory to save the logs
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString LogRootDirectoryName;

	// Log raw data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogRawData;

	// Distance threshold for raw data logging (cm)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float DistanceThreshold;

	// Log semantic map (if not already logged)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogSemanticMap;

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogSemanticEvents;

	// Map of actors to be logged to the semlog key-value pair info
	TMap<AActor*, TArray<TPair<FString, FString>>> ActorToSemLogInfo;

	// Map of actors to be logged to their unique name
	TMap<AActor*, FString> ActorToUniqueName;






	//// Distance threshold squared for raw data logging
	////float DistanceThresholdSquared;

	//// Map of skeletal component (to be logged) names to actor map 
	//TMap<FString, ASkeletalMeshActor*> SkelActNameToActPtrMap; // NameToSkelMap check  // check if skel can be used as AActor, and compare with IsA afterwards

	//// Map of dynamic actors (to be logged) names to actor map 
	//TMap<FString, AStaticMeshActor*> DynamicActNameToActPtrMap; // NameToDynamicActMap

	//// Map of static actors (to be logged) names to actor map 
	//TMap<FString, AStaticMeshActor*> StaticActNameToActPtrMap; // NameToStaticActMap

	//// Map of skeletal component (to be logged) to unique name
	//TMap<ASkeletalMeshActor*, FString> SkelActPtrToUniqNameMap; // SkelToUniqueName // Cast from AActor?

	//// Map of dynamic actors (to be logged) to unique name
	//TMap<AStaticMeshActor*, FString> DynamicActPtrToUniqNameMap; // DynamicActToUniqueName

	//// Map of static map actors (to be logged) to unique name
	//TMap<AStaticMeshActor*, FString> StaticActPtrToUniqNameMap; // StaticActToUniqueName // Make sure static functions are only called once? At beginplay, or laterz.. (the raw exporter could have a function called UpdateWithStatic() and Update() have a funct
	//
	//// Map of actors to their unique name
	//TMap<AActor*, FString> ActorToUniqueNameMap;

	//// Map of actors to their class type
	//TMap<AActor*, FString> ActorToClassTypeMap;

	//// User camera to unique name pair
	//TPair<USceneComponent*, FString> CameraToUniqueName;


	//TArray<ASLItem*> StaticItems;
	//TArray<ASLItem*> DynamicItems;




	// Raw data exporter
	FSLRawDataExporter* RawDataExporter;

	// Semantic map exporter
	FSLMapExporter* SemMapExporter;

	// Semantic events exporter
	FSLEventsExporter* SemEventsExporter;

	// Level path
	FString LevelPath;

	// Episode path
	FString EpisodePath;

	// Raw data path
	FString RawDataPath;

	// Episode unique tag
	FString EpisodeUniqueTag;
};
