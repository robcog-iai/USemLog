// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#if SL_WITH_EYE_TRACKING
#include "SLGazeProxy.h"
#endif // SL_WITH_EYE_TRACKING

#include "SLGazeVisualizer.generated.h"

/*
* Struct holding the text info about the actors
*/
struct FSLGazeTextInfo
{
	// Class name
	FString ClassName;

	// Unique identifier
	FString Id;
};

/*
* Demo actor used for appending semantic information text on the viewed actors
*/
UCLASS()
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
#if SL_WITH_EYE_TRACKING
	// Custom made sranipal proxy to avoid compilation issues
	class ASLGazeProxy* GazeProxy;
#endif // SL_WITH_EYE_TRACKING

	// Map of the semantically annotated actors and their text info
	TMap<AActor*, FSLGazeTextInfo> ActorToGazeInfo;
};
