// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLGazeDataHandler.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

//#if SL_WITH_EYE_TRACKING
//#include "SRanipal_Enums.h"
//#include "SRanipal_API.h"
//#include "SRanipal_Eye.h"
//#include "SRanipal_FunctionLibrary_Eye.h"
//#endif // SL_WITH_EYE_TRACKING

// Default ctor
FSLGazeDataHandler::FSLGazeDataHandler()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	
	World = nullptr;
	PlayerCameraRef = nullptr;
}

// Setup the eye tracking software
void FSLGazeDataHandler::Init()
{
	if(!bIsInit)
	{
//#if SL_WITH_EYE_TRACKING
//
//		if(ViveSR::anipal::Initial(ViveSR::anipal::Eye::ANIPAL_TYPE_EYE, nullptr) == ViveSR::Error::WORK)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Eye tracking framework init!"), *FString(__func__), __LINE__);
//			bIsInit = true;
//		}
//#endif // SL_WITH_EYE_TRACKING
	}
}

	// Setup the active camera
void FSLGazeDataHandler::Start(UWorld* InWorld)
{
	if(!bIsStarted && bIsInit)
	{
#if SL_WITH_EYE_TRACKING
		World = InWorld;
		if(World)
		{
			if(UGameplayStatics::GetPlayerController(World, 0))
			{
				PlayerCameraRef = UGameplayStatics::GetPlayerController(World, 0)->PlayerCameraManager;
				if(PlayerCameraRef)
				{
					bIsStarted = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d GetPlayerController failed, this should be called after BeginPlay!"), *FString(__func__), __LINE__);
			}
		}
#endif // SL_WITH_EYE_TRACKING
	}
}

void FSLGazeDataHandler::Finish()
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
//#if SL_WITH_EYE_TRACKING
//		if(ViveSR::anipal::Release(ViveSR::anipal::Eye::ANIPAL_TYPE_EYE) != ViveSR::Error::WORK)
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d Release eye tracking framework FAILED!"), *FString(__func__), __LINE__);
//		}
//		else
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Released eye tracking framework!"), *FString(__func__), __LINE__);
//		}
//		
//		bIsStarted = false;
//		bIsInit = false;
//		bIsFinished = true;
//	#endif // SL_WITH_EYE_TRACKING
	}
}

// Get the current gaze data, true if a raytrace hit occurs
bool FSLGazeDataHandler::GetData(FSLGazeData& OutData)
{
	if(!bIsStarted)
	{
		return false;
	}
	return true;
}

void FSLGazeDataHandler::TestGazeData()
{
	if(bIsStarted)
	{
//#if SL_WITH_EYE_TRACKING
//		float RayLength = 1000.0f;
//		float RayRadius = 0.5f;
//			
//		FVector CameraGazeOrigin, CameraGazeDirection;
//		FVector RayCastOrigin, RayCastDirection;
//
//		FVector PlayerMainCameraLocation;
//		FRotator PlayerMainCameraRotation;
//
//		bool valid = USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, CameraGazeOrigin, CameraGazeDirection);
//		if (valid) 
//		{
//			// Find the ray cast origin and target positon.
//			PlayerMainCameraLocation = PlayerCameraRef->GetCameraLocation();
//			PlayerMainCameraRotation = PlayerCameraRef->GetCameraRotation();
//			RayCastOrigin = PlayerMainCameraLocation + FVector(0.f, 0.f, -4.f);
//			RayCastDirection = PlayerMainCameraRotation.RotateVector(
//				PlayerMainCameraLocation + CameraGazeDirection * RayLength
//			);
//
//			DrawDebugLine(
//				World,
//				RayCastOrigin, RayCastDirection,
//				FColor::Emerald,
//				false, -1, 0,
//				RayRadius
//			);
//
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s Loc %s"), *FString(__func__), __LINE__, 
//				*PlayerCameraRef->GetName(), *PlayerCameraRef->GetCameraLocation().ToString());
//
//		}
//		else
//		{
//			UE_LOG(LogTemp, Log, TEXT("[SRanipal] Notvalid gaze ray"));
//		}
//#endif // SL_WITH_EYE_TRACKING
	}
}
