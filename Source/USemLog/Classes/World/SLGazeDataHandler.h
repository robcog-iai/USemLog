// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLStructs.h"
#include "Camera/PlayerCameraManager.h"

/**
* Structure holding the eye tracking data
*/
struct FSLGazeData
{
	// Default ctor
	FSLGazeData() = default;
	
	// Init ctor
	FSLGazeData(const FVector& InOrigin, const FVector& InTarget, const FSLEntity& InEntity) :
		Origin(InOrigin), Target(InTarget), Entity(InEntity){};

	// Set the values of the data
	void SetData(const FVector& InOrigin, const FVector& InTarget, const FSLEntity& InEntity)
	{
		Origin = InOrigin;
		Target = InTarget;
		Entity = InEntity;
	}
	
	// Head position
	FVector Origin;
	
	// Line trace end location
	FVector Target;
	
	// The entities in focus (sphere trace
	FSLEntity Entity;

	// Quick check if there is data
	FORCEINLINE bool HasDataFast() const { return Entity.Obj != nullptr; };

	// In depth check for data
	FORCEINLINE bool HasData() const { return Entity.IsSet(); };

	// Check if the two gaze data is equal with a tolerance
	FORCEINLINE bool Equals(const FSLGazeData& Other, float Tolerance=KINDA_SMALL_NUMBER) const
	{
		return Entity.EqualsFast(Other.Entity) && Target.Equals(Other.Target, Tolerance) && Origin.Equals(Other.Origin, Tolerance);
	}
};

/**
* Handler for getting the eye tracking data
*/
class FSLGazeDataHandler
{
public:
	// Default constructor
	FSLGazeDataHandler();

	// Setup the eye tracking software
	void Init();

	// Setup the active camera
	void Start(UWorld* InWorld);
	
	// Stop the eye tracking software
	void Finish();

	// Check if the eye tracking software is set up
	bool IsInit() const {return bIsInit; };

	// True if the active camera is set
	bool IsStarted() const {return bIsStarted; };
	
	// Check if the eye tracking software is stopped
	bool IsFinished() const { return bIsFinished; };

	// Get the current gaze data, true if a raytrace hit occurs
	bool GetData(FSLGazeData& OutData);

private:
	// True if the eye tracking framework is successfully working
	bool bIsInit;

	// True when the active camera is set
	bool bIsStarted;

	// True when the eye tracking framework is stopped
	bool bIsFinished;
	
	// Used for trace calculation
	UWorld* World;

	// Used for getting the gaze origin point
	APlayerCameraManager* PlayerCameraRef;

	/* Constants */
	constexpr static float RayLength = 1000.f;
	constexpr static float RayRadius = 1.5f;
};
