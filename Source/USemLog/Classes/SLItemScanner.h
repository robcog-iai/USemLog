// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLItemScanner.generated.h"

/**
 * Scans handheld items by taking images from unidistributed points form a sphere as a camera location
 */
UCLASS()
class USLItemScanner : public UObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLItemScanner();

	// Dtor
	~USLItemScanner();

	// Setup scanning room
	void Init(UWorld* World);

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

private:
	// Get the items to scan
	void ScanItems(const float InVolumeLimit, const float InLengthLimit);

	// Check if the item should be scanned
	bool ShouldBeScanned(UStaticMeshComponent* SMC, const float InVolumeLimit, const float InLengthLimit) const;

	// Scan the item
	void ScanItem(AActor *Item);
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;
	
	// Scanner box actor to spawn
	UPROPERTY() // Avoid GC
	AStaticMeshActor* ScanBoxActor;

	/* Constants */
	// Vertical offset to spawn the scanning room
	constexpr static const float ZOffset = 1000.f;

	// Volume limit in cubic centimeters (1000cm^3 = 1 Liter) of items to scan
	constexpr static const float VolumeLimit = 40000.f;

	// Length limit of its bounding box points (cm) 
	constexpr static const float LengthLimit = 75.f;
};
