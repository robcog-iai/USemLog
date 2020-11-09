// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLImgCalibrator.generated.h"

// Forward declarations
class ASLIndividualManager;
class USLVisibleIndividual;
class ASkeletalMeshActor;
class AStaticMeshActor;
class UGameViewportClient;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Img Calibrator")
class ASLImgCalibrator : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLImgCalibrator();

	// Dtor
	~ASLImgCalibrator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Set up any required references and connect to server
	void Init();

	// Start processing any incomming messages
	void Start();

	// Stop processing the messages, and disconnect from server
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Set and calibrate the next individual (return false if there are no more indvidiuals)
	bool CalibrateIndividual();

private:
	/* Managers */
	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

protected:
	// Skip auto init and start
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done 
	uint8 bIsFinished : 1;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;


	// Mesh used to load all the mask materials and rendered on screen
	UPROPERTY() // Avoid GC
	AStaticMeshActor* MaskRenderActor;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY() // Avoid GC
	AStaticMeshActor* CameraPoseActor;

	// Material to apply to the render mesh
	UPROPERTY() // Avoid GC
	UMaterialInstanceDynamic* DynamicMaskMaterial;

	// Pointer to the render SMA mesh
	class UStaticMesh* MaskRenderMesh;

private:
	// Individuals to iterate through
	TArray<USLVisibleIndividual*> VisibleIndividuals;

	// Active index
	int32 IndividualIdx = INDEX_NONE;
};
