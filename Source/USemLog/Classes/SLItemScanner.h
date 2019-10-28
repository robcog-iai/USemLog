// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "SLStructs.h" // FSLEntity
#include "SLItemScanner.generated.h"

// Forward declarations
class AStaticMeshActor;
class APlayerCameraManager;

/**
 * Scans handheld items by taking images from unidistributed points form a sphere as a camera location
 */
UCLASS()
class USLItemScanner : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLItemScanner();

	// Dtor
	~USLItemScanner();

	// Setup scanning room
	void Init(UWorld* InWorld);

	// Start scanning
	void Start();

	// Finish scanning
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	/** Begin FTickableGameObject interface */
	// Called after ticking all actors, DeltaTime is the time passed since the last call.
	virtual void Tick(float DeltaTime) override;

	// Return if object is ready to be ticked
	virtual bool IsTickable() const override;

	// Return the stat id to use for this tickable
	virtual TStatId GetStatId() const override;
	/** End FTickableGameObject interface */

private:
	// Load scan box actor
	bool LoadScanBoxActor();

	// Load scan camera convenience actor
	bool LoadScanCameraPoseActor();

	// Load scanning points
	bool LoadScanPoints();

	// Load items to scan
	bool LoadItemsToScan();
	
	
	// Get the items to scan
	//void ScanItems(const float InVolumeLimit, const float InLengthLimit);

	// Check if the item should be scanned
	bool ShouldBeScanned(UStaticMeshComponent* SMC) const;

	// Scan the item
	void ScanItem(AActor *Item);

	// Quit the editor once the scanning is finished
	void QuitEditor();
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// True if the object can be ticked (used by FTickableGameObject)
	bool bIsTickable;

	// Pointer to the world
	UWorld* World;

	// Used for setting the camera pose (SetViewTarget())
	APlayerCameraManager* PCM;

	// Scan camera poses
	TArray<FTransform> ScanPoses;

	// Scan items with semantic data
	TArray<TPair<UStaticMeshComponent*, FSLEntity>> ScanItems;

	// Currently active camera pose scan index
	int32 CurrPoseIdx;

	// Currently scanned item index in map
	int32 CurrItemIdx;

	
	// Scanner box actor to spawn
	UPROPERTY() // Avoid GC
	AStaticMeshActor* ScanBoxActor;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY() // Avoid GC
	AStaticMeshActor* CameraPoseActor;



	/* Constants */
	// Vertical offset to spawn the scanning room
	constexpr static const float ScanBoxOffsetZ = 1000.f;

	// Volume limit in cubic centimeters (1000cm^3 = 1 Liter) of items to scan
	constexpr static const float VolumeLimit = 40000.f;

	// Length limit of its bounding box points (cm) 
	constexpr static const float LengthLimit = 75.f;
};
