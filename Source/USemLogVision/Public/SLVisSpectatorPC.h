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
	
	TArray<ASLVisCamera*> SLVisCameras;

private:
	void PlaybackFinished();
	void GotoCbRecursive();

	void PauseToggle();

	void SetPause2(bool bPause);

	void Step();
	
	void NextView();

	float CurrTime;

	int32 CameraIdx;

	bool PBFinished;

	bool TakeNextScreenshot();

	void StartCapture();

	void OnScrCb(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

	FString CurrFilename;
	FString Path;

	//
	float CurrTime;
	float TotalTime;
	int32 CurrView;
	int32 TotalViews;

	// Go to next record step

	// Go to prev record step

	// Go to next view

	// Take screenshot (curr view)

};
