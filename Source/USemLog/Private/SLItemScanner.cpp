// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLItemScanner.h"
#include "ConstructorHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "SLEntitiesManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

// Ctor
USLItemScanner::USLItemScanner()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bIsTickable = false;
	CurrPoseIdx = INDEX_NONE;
	CurrItemIdx = INDEX_NONE;
}

// Dtor
USLItemScanner::~USLItemScanner()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();
	}
}

// Init scanner
void USLItemScanner::Init(UWorld* InWorld)
{
	if (!bIsInit)
	{
		// Cache the world, needed later to get the camera manager (can be done after init -- around BeginPlay)
		World = InWorld;

		// Spawn the scan box mesh actor
		if(!LoadScanBoxActor())
		{
			return;
		}

		// Spawn the target view dummy actor
		if(!LoadScanCameraPoseActor())
		{
			return;
		}

		// Load the scan points pattern
		if(!LoadScanPoints())
		{
			return;
		}
		
		// Load items which need scanning
		if(!LoadItemsToScan())
		{
			return;
		}	

		// Disable physics on skeletal actors (avoid any of them roaming through the scene)
		for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
		{
			SkMAItr->DisableComponentsSimulatePhysics();
		}

		CurrPoseIdx = 0;
		CurrItemIdx = 0;
		
		bIsInit = true;
	}
}

// Start scanning
void USLItemScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
		// Get the camera controller, not available yet in Init(), hence called here
		PCM = UGameplayStatics::GetPlayerCameraManager(World, 0);
		if(!PCM)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the CameraManager.."), *FString(__func__), __LINE__);
			return;
		}
		
		// Enable ticking (camera movement will happen every tick)
		bIsTickable = true;
		
		bIsStarted = true;
	}
}

// Finish scanning
void USLItemScanner::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Finish"),
			*FString(__func__), __LINE__);
		
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLItemScanner::Tick(float DeltaTime)
{
	if(CurrItemIdx < ScanItems.Num())
	{
		
	}
	
	if(CurrPoseIdx < ScanPoses.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] CurrCameraPoseIdx = %ld; CameraPose=%s; ActorPose=%s;"),
			*FString(__func__), __LINE__, World->GetTimeSeconds(),
			CurrPoseIdx,
			*ScanPoses[CurrPoseIdx].ToString(),
			*CameraPoseActor->GetTransform().ToString());
		
		CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
		PCM->SetViewTarget(CameraPoseActor);

		CurrPoseIdx++;
	}
	else
	{
		QuitEditor();
	}
}

// Return if object is ready to be ticked
bool USLItemScanner::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLItemScanner::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLWorldStateLogger, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */

// Load scan box actor
bool USLItemScanner::LoadScanBoxActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanBox");
	ScanBoxActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		FTransform(FVector(0.f, 0.f, ScanBoxOffsetZ)), SpawnParams);
	if(!ScanBoxActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn scan box actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	ScanBoxActor->SetActorLabel(FString(TEXT("SM_ScanBox")));
#endif // WITH_EDITOR
		
	UStaticMesh* ScanBoxMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/USemLog/Vision/ScanBox/SM_ScanBox.SM_ScanBox"));
	if(!ScanBoxMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find scan box mesh.."), *FString(__func__), __LINE__);
		ScanBoxActor->Destroy();
		return false;
	}
	ScanBoxActor->GetStaticMeshComponent()->SetStaticMesh(ScanBoxMesh);
	ScanBoxActor->SetMobility(EComponentMobility::Static);

	return true;
}

// Load scan camera convenience actor
bool USLItemScanner::LoadScanCameraPoseActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanCameraPoseDummy");
	CameraPoseActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnParams);
	if(!CameraPoseActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn convenience camera pose actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	CameraPoseActor->SetActorLabel(FString(TEXT("SM_ScanCameraPoseDummy")));
#endif // WITH_EDITOR

	UStaticMesh* CameraPoseDummyMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/USemLog/Vision/ScanCameraPoseDummy/SM_ScanCameraPoseDummy.SM_ScanCameraPoseDummy"));
	if(!CameraPoseDummyMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find camera pose dummy mesh.."), *FString(__func__), __LINE__);
		CameraPoseActor->Destroy();
		return false;
	}
	CameraPoseActor->GetStaticMeshComponent()->SetStaticMesh(CameraPoseDummyMesh);
	CameraPoseActor->SetMobility(EComponentMobility::Movable);

	return true;
}

// Load scanning points
bool USLItemScanner::LoadScanPoints()
{
	//// Load the scan poses
	//for(uint32 Idx = 0; Idx < 1200; Idx++)
	//{
	//	const float Z = Idx * 1.1f;
	//	const FTransform T = FTransform(FVector(0.f, 0.f, Idx));
	//	ScanPoses.Emplace(T);
	//}

	ScanPoses.Emplace(FTransform(FVector(-100.f, 0.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-100.f, 50.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-100.f, -50.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-50.f, 0.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-50.f, 50.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-50.f, -50.f, ScanBoxOffsetZ)));
		
	if(ScanPoses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No scan poses added.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Load items to scan
bool USLItemScanner::LoadItemsToScan()
{
	// Cache the classes that were already iterated
	TSet<FString> IteratedClasses;
	
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
	{
		const FSLEntity SemEntity = Pair.Value;

		// Skip if class was already checked
		if(IteratedClasses.Contains(SemEntity.Class))
		{
			continue;
		}
		IteratedClasses.Emplace(SemEntity.Class);
		
		// Check if the item has a visual
		if (AStaticMeshActor* ObjAsSMA = Cast<AStaticMeshActor>(SemEntity.Obj))
		{
			if(UStaticMeshComponent* SMC = ObjAsSMA->GetStaticMeshComponent())
			{
				if(ShouldBeScanned(SMC))
				{
					ScanItems.Emplace(SMC, SemEntity);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s is out of bounds/static, skipping scan.."),
						*FString(__func__), __LINE__, *SemEntity.Class);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no static mesh component, skipping scan.."),
					*FString(__func__), __LINE__, *SemEntity.Class);
			}
		}
		else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(SemEntity.Obj))
		{
			if(ShouldBeScanned(ObjAsSMC))
			{
				ScanItems.Emplace(ObjAsSMC, SemEntity);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is out of bounds, skipping scan.."),
					*FString(__func__), __LINE__, *SemEntity.Class);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual, skipping scan.."),
				*FString(__func__), __LINE__, *SemEntity.Class);
		}
	}

	if(ScanItems.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No items found to scan.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

//// Get an array of items to scan
//void USLItemScanner::ScanItems(const float InVolumeLimit, const float InLengthLimit)
//{
//	// Cache the classes that were already iterated
//	TSet<FString> IteratedClasses;
//	
//	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
//	{
//		const FSLEntity SemEntity = Pair.Value;
//
//		// Skip if class was already checked
//		if(IteratedClasses.Contains(SemEntity.Class))
//		{
//			continue;
//		}
//		IteratedClasses.Emplace(SemEntity.Class);
//		
//		// Check if the item has a visual
//		if (AStaticMeshActor* ObjAsSMA = Cast<AStaticMeshActor>(SemEntity.Obj))
//		{
//			if(UStaticMeshComponent* SMC = ObjAsSMA->GetStaticMeshComponent())
//			{
//				if(ShouldBeScanned(SMC, InVolumeLimit, InLengthLimit))
//				{
//					ScanItem(SMC->GetOwner());
//				}
//				else
//				{
//					UE_LOG(LogTemp, Error, TEXT("%s::%d %s is out of bounds/static, skipping scan.."),
//						*FString(__func__), __LINE__, *SemEntity.Class);
//				}
//			}
//			else
//			{
//				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no static mesh component, skipping scan.."),
//					*FString(__func__), __LINE__, *SemEntity.Class);
//			}
//		}
//		else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(SemEntity.Obj))
//		{
//			if(ShouldBeScanned(ObjAsSMC, InVolumeLimit, InLengthLimit))
//			{
//				ScanItem(ObjAsSMC->GetOwner());
//			}
//			else
//			{
//				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is out of bounds, skipping scan.."),
//					*FString(__func__), __LINE__, *SemEntity.Class);
//			}
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual, skipping scan.."),
//				*FString(__func__), __LINE__, *SemEntity.Class);
//		}
//	}
//}

// Check if the items meets the requirements to be scanned
bool USLItemScanner::ShouldBeScanned(UStaticMeshComponent* SMC) const
{
	return SMC->Mobility == EComponentMobility::Movable &&
		SMC->Bounds.GetBox().GetVolume() < VolumeLimit &&
		SMC->Bounds.SphereRadius * 2.f < LengthLimit;
}

// Scan the current item
void USLItemScanner::ScanItem(AActor* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Scanning %s.."),
			*FString(__func__), __LINE__, *Item->GetName());
}

// Quit the editor once the scanning is finished
void USLItemScanner::QuitEditor()
{
	//FGenericPlatformMisc::RequestExit(false);
	//
	//FGameDelegates::Get().GetExitCommandDelegate().Broadcast();
	//FPlatformMisc::RequestExit(0);

	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
	}
}