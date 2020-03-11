// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/PlayerCameraManager.h"
#if SL_WITH_EYE_TRACKING
#include "SLGazeProxy.h"
#endif // SL_WITH_EYE_TRACKING
#include "SLGazeVisualizer.generated.h"

/*
* Struct holding the text info about the actors
*/
struct FSLGazeSemanticData
{
	// Class name
	FString ClassName;

	// Unique identifier
	FString Id;

	// Color
	FColor MaskColor;
};

/*
* Demo actor used for appending semantic information text on the viewed actors
*/
UCLASS(ClassGroup = (SL),  DisplayName = "SL Gaze Visualizer")
class ASLGazeVisualizer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLGazeVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Load each actor's info from tags
	void LoadSemanticData();

	// Process the gaze trace
	void GazeTrace_NONE(const FVector& Origin, const FVector& Target);
	void GazeTrace_Line(const FVector& Origin, const FVector& Target);
	void GazeTrace_Sphere(const FVector& Origin, const FVector& Target);

	// Print out the gaze hit info
	void ProcessGazeHit(const FHitResult& HitResult);

private:
	// Screen text
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	class UTextRenderComponent* GazeText;

	// Visual component of the gaze target
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	class UStaticMeshComponent* GazeVisual;

	// Material for the gaze visual
	class UMaterial* VisualMaterial;

	// Raytrace limit
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float RayLength = 1000.f;

	// Sphere raytrace radius
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float RayRadius = 1.5f;

#if SL_WITH_EYE_TRACKING
	// Custom made sranipal proxy to avoid compilation issues
	class ASLGazeProxy* GazeProxy;
#endif // SL_WITH_EYE_TRACKING

	// Used for getting the gaze origin point
	APlayerCameraManager* CameraManager;

	// Process update function
	typedef void(ASLGazeVisualizer::* GazeTraceFuncPtrType)(const FVector& Origin, const FVector& Target);
	GazeTraceFuncPtrType GazeTraceFuncPtr;

	// Parameters used for the trace
	FCollisionQueryParams TraceParams;

	// Collision shape used for the sweep
	FCollisionShape SphereShape;

	// Map of the semantically annotated actors and their text info
	TMap<AStaticMeshActor*, FSLGazeSemanticData> SMActorSemanticData;

	// Skip reading the semantic data if we are looking at the same actor
	AActor* PrevActor;
};
