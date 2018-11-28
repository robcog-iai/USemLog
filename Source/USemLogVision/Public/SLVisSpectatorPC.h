// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SLVisSpectatorPC.generated.h"

/**
 * 
 */
UCLASS()
class USEMLOGVISION_API ASLVisSpectatorPC : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Ctor
	ASLVisSpectatorPC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	TArray<USLVisManager*> SLVisCameras;

private:
	void PlaybackFinished();
	void Goto();

	void PauseToggle();

	void SetPause(bool bPause);

	void Step();
	
	void NextView();

	float CurrTime;

	int32 CameraIdx;

};
