// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisSpectatorPC.h"
#include "EngineUtils.h"
#include "Engine/DemoNetDriver.h"
#include "SLVisManager.h"

// Ctor
ASLVisSpectatorPC::ASLVisSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;
	//bShowMouseCursor = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
}

// Called when the game starts or when spawned
void ASLVisSpectatorPC::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	for (TObjectIterator<USLVisManager> Itr; Itr; ++Itr)
	{
		SLVisCameras.Add(*Itr);
	}

	if (GetWorld()->DemoNetDriver)
	{
		GetWorld()->DemoNetDriver->OnDemoFinishPlaybackDelegate.AddUObject(this, &ASLVisSpectatorPC::PlaybackFinished);
		GetWorld()->DemoNetDriver->OnGotoTimeDelegate.AddUObject(this, &ASLVisSpectatorPC::Goto);
	}

	SetPause(true);
}

// Called to bind functionality to input
void ASLVisSpectatorPC::SetupInputComponent()
{
	Super::SetupInputComponent();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	InputComponent->BindAction("SLVisPause", IE_Pressed, this, &ASLVisSpectatorPC::PauseToggle).bExecuteWhenPaused = true;
	InputComponent->BindAction("SLVisStep", IE_Pressed, this, &ASLVisSpectatorPC::Step).bExecuteWhenPaused = true;
	InputComponent->BindAction("SLVisNextView", IE_Pressed, this, &ASLVisSpectatorPC::NextView).bExecuteWhenPaused = true;

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
 }


 void ASLVisSpectatorPC::PlaybackFinished()
 {
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 //if (GetWorld()->DemoNetDriver)
	 //{
		// GetWorld()->DemoNetDriver->GotoTimeInSeconds(0.f);
		// Pause();
	 //}
 }

 void ASLVisSpectatorPC::Goto()
 {
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
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

 void ASLVisSpectatorPC::SetPause(bool bPause)
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
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 CurrTime += 0.1;
	 GetWorld()->DemoNetDriver->GotoTimeInSeconds(CurrTime);
	 SetPause(false);
	 SetPause(true);
 }

 void ASLVisSpectatorPC::NextView()
 {
	 UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
	 if (SLVisCameras.Num() > 0)
	 {
		 FVector Loc = SLVisCameras[CameraIdx]->GetComponentLocation();
		 FQuat Quat = SLVisCameras[CameraIdx]->GetComponentQuat();

		 if (APawn* Pawn = GetPawnOrSpectator())
		 {
			 Pawn->SetActorLocationAndRotation(Loc, Quat);
			 UE_LOG(LogTemp, Warning, TEXT("\t%s::%d %f/%f -- %s"), TEXT(__FUNCTION__), __LINE__,
				 GetWorld()->DemoNetDriver->DemoCurrentTime, GetWorld()->DemoNetDriver->DemoTotalTime,
				 *GetWorld()->DemoNetDriver->GetActiveReplayName());
		 }

	 }
	 CameraIdx++;
	 if (CameraIdx > SLVisCameras.Num() - 1)
	 {
		 CameraIdx = 0;
	 }
 }