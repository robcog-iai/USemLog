// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisSpectatorPC.h"
#include "EngineUtils.h"
#include "Engine/DemoNetDriver.h"
#include "SLVisManager.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "ImageUtils.h"
#include "UnrealClient.h"

// Ctor
ASLVisSpectatorPC::ASLVisSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;
	//bShowMouseCursor = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	PBFinished = false;
}

// Called when the game starts or when spawned
void ASLVisSpectatorPC::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	//for (TObjectIterator<USLVisManager> Itr; Itr; ++Itr)
	//{
	//	SLVisCameras.Add(*Itr);
	//}
	Path = FPaths::ProjectDir() + TEXT("SemLog/Screenshots/");
	

	for (TActorIterator<ASLVisCamera>Itr(GetWorld()); Itr; ++Itr)
	{
		SLVisCameras.Add(*Itr);
	}

	if (GetWorld()->DemoNetDriver)
	{
		GetWorld()->DemoNetDriver->OnDemoFinishPlaybackDelegate.AddUObject(this, &ASLVisSpectatorPC::PlaybackFinished);
		GetWorld()->DemoNetDriver->OnGotoTimeDelegate.AddUObject(this, &ASLVisSpectatorPC::GotoCbRecursive);
		GetWorld()->DemoNetDriver->GotoTimeInSeconds(0.f);
	}

	//SetPause2(true);
}

// Called to bind functionality to input
void ASLVisSpectatorPC::SetupInputComponent()
{
	Super::SetupInputComponent();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	InputComponent->BindAction("SLVisPause", IE_Pressed, this, &ASLVisSpectatorPC::PauseToggle).bExecuteWhenPaused = true;
	InputComponent->BindAction("SLVisStep", IE_Pressed, this, &ASLVisSpectatorPC::Step).bExecuteWhenPaused = true;
	InputComponent->BindAction("SLVisNextView", IE_Pressed, this, &ASLVisSpectatorPC::NextView).bExecuteWhenPaused = true;
	InputComponent->BindAction("SLVisStartCapture", IE_Pressed, this, &ASLVisSpectatorPC::StartCapture).bExecuteWhenPaused = true;

}

// Called every frame
 void ASLVisSpectatorPC::Tick(float DeltaTime)
 {
	 Super::Tick(DeltaTime);
	 //UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 //if (GetWorld()->DemoNetDriver)
	 //{
		// if (GetWorld()->DemoNetDriver->IsPlaying())
		// {
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
		//	
		//	for (auto C : SLVisCameras)
		//	{
		//		FVector Loc = C->GetComponentLocation();
		//		FQuat Quat = C->GetComponentQuat();

		//		if (APawn* Pawn = GetPawnOrSpectator())
		//		{
		//			Pawn->SetActorLocationAndRotation(Loc, Quat);
		//			UE_LOG(LogTemp, Warning, TEXT("\t%s::%d %f/%f -- %s"), TEXT(__FUNCTION__), __LINE__,
		//				GetWorld()->DemoNetDriver->DemoCurrentTime, GetWorld()->DemoNetDriver->DemoTotalTime,
		//				*GetWorld()->DemoNetDriver->GetActiveReplayName());

		//		}
		//	}
		// }
	 //}

	 //if (SLVisCameras.Num() > 0)
	 //{
		// FVector Loc = SLVisCameras[CameraIdx]->GetComponentLocation();
		// FQuat Quat = SLVisCameras[CameraIdx]->GetComponentQuat();

		// if (APawn* Pawn = GetPawnOrSpectator())
		// {
		//	 Pawn->SetActorLocationAndRotation(Loc, Quat);
		//	 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d %f/%f -- %s"), TEXT(__FUNCTION__), __LINE__,
		//		 GetWorld()->DemoNetDriver->DemoCurrentTime, GetWorld()->DemoNetDriver->DemoTotalTime,
		//		 *GetWorld()->DemoNetDriver->GetActiveReplayName());
		// }

	 //}
	 //if (PBFinished)
	 //{
		// GetWorld()->Exec(GetWorld(), TEXT("demostop"));

		// //if (GetWorld()->GetGameInstance())
		// //{
		//	// GetWorld()->GetGameInstance()->StopRecordingReplay();
		// //}
	 //}
 }


 void ASLVisSpectatorPC::PlaybackFinished()
 {
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 //if (GetWorld()->DemoNetDriver)
	 //{
		// GetWorld()->DemoNetDriver->GotoTimeInSeconds(0.f);
		// Pause();
	 //}
	 PBFinished = true;

	 //GetWorld()->Exec(GetWorld(), TEXT("demostop"));
	 //GetWorld()->TimeSeconds = 0.f;

	 //if (PBFinished)
	 //{
		// GetWorld()->Exec(GetWorld(), TEXT("demostop"));

	 //ConsoleCommand(TEXT("demostop"));
		 //if (GetWorld()->GetGameInstance())
		 //{
			// UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
			// GetWorld()->GetGameInstance()->StopRecordingReplay();
		 //}
	 //}
	 //FGenericPlatformMisc::RequestExit(true);
 }

 void ASLVisSpectatorPC::GotoCbRecursive()
 {
	 if (CurrTime > GetWorld()->DemoNetDriver->DemoTotalTime)
	 {
		 UE_LOG(LogTemp, Error, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
		 return;
	 }

	 // Pause the replay
	 SetPause2(true);
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	 int32 CamNr = 0;
	 // Save the images
	 for (auto C : SLVisCameras)
	 {
		 SetViewTarget(C);
		 UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Loc: %s"), TEXT(__FUNCTION__), __LINE__,
			 *GetViewTarget()->GetActorLocation().ToString());

		CurrFilename = FString::Printf(TEXT("Screenshot_%f_%d.png"), CurrTime, CamNr);
		UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
		//ViewportClient->OnScreenshotCaptured().Clear();
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &ASLVisSpectatorPC::OnScrCb);

		FString FilePath = Path + CurrFilename;
		FScreenshotRequest::RequestScreenshot(FilePath, false, false);
		ViewportClient->Viewport->TakeHighResScreenShot();

		CamNr++;
	 }


	 // Unpause the replay and goto next position
	 SetPause2(false);
	 CurrTime += 0.02;
	 GetWorld()->DemoNetDriver->GotoTimeInSeconds(CurrTime);

	 //SetPause2(true);
	 //UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 //if (CurrTime < GetWorld()->DemoNetDriver->DemoTotalTime)
	 //{
		// for (auto C : SLVisCameras)
		// {
		//	 //FVector Loc = C->GetComponentLocation();
		//	 //FQuat Quat = C->GetComponentQuat();

		//	 //if (APawn* Pawn = GetPawnOrSpectator())
		//	 //{
		//		// Pawn->SetActorLocationAndRotation(Loc, Quat);
		//		// UE_LOG(LogTemp, Warning, TEXT("\t%s::%d %f/%f -- %s"), TEXT(__FUNCTION__), __LINE__,
		//		//	 GetWorld()->DemoNetDriver->DemoCurrentTime, GetWorld()->DemoNetDriver->DemoTotalTime,
		//		//	 *GetWorld()->DemoNetDriver->GetActiveReplayName());

		//		 SetViewTarget(C);

		//		 UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
		//		 ViewportClient->OnScreenshotCaptured().Clear();
		//	 //}
		// }
		// //SetPause2(false);
		// //CurrTime += 0.1;
		// //GetWorld()->DemoNetDriver->GotoTimeInSeconds(CurrTime);
	 //}
 }

 void ASLVisSpectatorPC::PauseToggle()
 {
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	 FString TimeString;

	 AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
	 check(WorldSettings != nullptr);

	 if (WorldSettings->Pauser == nullptr)
	 {
		 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
		 if (GetWorld()->DemoNetDriver != nullptr && GetWorld()->DemoNetDriver->ServerConnection != nullptr && GetWorld()->DemoNetDriver->ServerConnection->OwningActor != nullptr)
		 {
			 UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
			 APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->DemoNetDriver->ServerConnection->OwningActor);
			 if (PlayerController != nullptr)
			 {
				 WorldSettings->Pauser = PlayerController->PlayerState;
				 UE_LOG(LogTemp, Warning, TEXT("\t\t\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
			 }
		 }
	 }
	 else
	 {
		 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
		 WorldSettings->Pauser = nullptr;
	 }
 }

 void ASLVisSpectatorPC::SetPause2(bool bPause)
 {
	 if (bPause)
	 {
		 if (GetWorld()->GetWorldSettings()->Pauser == nullptr)
		 {
			 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
			 if (GetWorld()->DemoNetDriver != nullptr &&
				 GetWorld()->DemoNetDriver->ServerConnection != nullptr && 
				 GetWorld()->DemoNetDriver->ServerConnection->OwningActor != nullptr)
			 {
				 GetWorld()->GetWorldSettings()->Pauser = PlayerState;
				 UE_LOG(LogTemp, Warning, TEXT("\t\t\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
			 }
		 }
	 }
	 else
	 {
		 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d"), TEXT(__FUNCTION__), __LINE__);

		 GetWorld()->GetWorldSettings()->Pauser = nullptr;
	 }
 }

 void ASLVisSpectatorPC::Step()
 {
	 SetPause2(false);
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 CurrTime += 0.1;
	 GetWorld()->DemoNetDriver->GotoTimeInSeconds(CurrTime);

 }

 void ASLVisSpectatorPC::NextView()
 {
	 //UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 //if (SLVisCameras.Num() > 0)
	 //{
		// FVector Loc = SLVisCameras[CameraIdx]->GetComponentLocation();
		// FQuat Quat = SLVisCameras[CameraIdx]->GetComponentQuat();

		// if (APawn* Pawn = GetPawnOrSpectator())
		// {
		//	 Pawn->SetActorLocationAndRotation(Loc, Quat);
		//	 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d %f/%f -- %s"), TEXT(__FUNCTION__), __LINE__,
		//		 GetWorld()->DemoNetDriver->DemoCurrentTime, GetWorld()->DemoNetDriver->DemoTotalTime,
		//		 *GetWorld()->DemoNetDriver->GetActiveReplayName());
		// }

	 //}
	 //CameraIdx++;
	 //if (CameraIdx > SLVisCameras.Num() - 1)
	 //{
		// CameraIdx = 0;
	 //}
 }

 bool ASLVisSpectatorPC::TakeNextScreenshot()
 {
	 return true;
 }

 void ASLVisSpectatorPC::StartCapture()
 {
	 GetWorld()->DemoNetDriver->GotoTimeInSeconds(0.f);
 }

 void ASLVisSpectatorPC::OnScrCb(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
 {
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	 // Make sure that all alpha values are opaque.
	 TArray<FColor>& RefBitmap = const_cast<TArray<FColor>&>(Bitmap);
	 for (auto& Color : RefBitmap)
		 Color.A = 255;

	 TArray<uint8> CompressedBitmap;
	 FImageUtils::CompressImageArray(SizeX, SizeY, RefBitmap, CompressedBitmap);
	 FString FullPath = Path + CurrFilename;
	 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Path=%s"), TEXT(__FUNCTION__), __LINE__, *FullPath);
	 FFileHelper::SaveArrayToFile(CompressedBitmap, *FullPath);
 }