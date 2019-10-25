// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLItemScanner.h"
#include "ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "SLEntitiesManager.h"
#include "Kismet/GameplayStatics.h"

// Ctor
USLItemScanner::USLItemScanner()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
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
void USLItemScanner::Init(UWorld* World)
{
	if (!bIsInit)
	{
		// Spawn a the scan box SMA
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("SM_ScanBox");
		ScanBoxActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
			FTransform(FVector(0.f, 0.f, ZOffset)), SpawnParams);
		if(!ScanBoxActor)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn scan box actor.."), *FString(__func__), __LINE__);
			return;
		}

		// Load the scan box static mesh
		UStaticMesh* ScanBoxMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/USemLog/Vision/SM_SLVisScanBox.SM_SLVisScanBox"));
		if(!ScanBoxMesh)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find scan box mesh.."), *FString(__func__), __LINE__);
			return;
		}
		ScanBoxActor->GetStaticMeshComponent()->SetStaticMesh(ScanBoxMesh);


		//UGameplayStatics::GetPlayerController(World, 0)->SetViewTarget(ScanBoxActor);
		UGameplayStatics::GetPlayerCameraManager(World,0)->SetViewTarget(ScanBoxActor);
		
		bIsInit = true;
	}
}

// Start scanning
void USLItemScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
		ScanItems(VolumeLimit, LengthLimit);
		bIsStarted = true;
	}
}

// Finish scanning
void USLItemScanner::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Get an array of items to scan
void USLItemScanner::ScanItems(const float InVolumeLimit, const float InLengthLimit)
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
				if(ShouldBeScanned(SMC, InVolumeLimit, InLengthLimit))
				{
					ScanItem(SMC->GetOwner());
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
			if(ShouldBeScanned(ObjAsSMC, InVolumeLimit, InLengthLimit))
			{
				ScanItem(ObjAsSMC->GetOwner());
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

}

// Check if the items meets the requirements to be scanned
bool USLItemScanner::ShouldBeScanned(UStaticMeshComponent* SMC, const float InVolumeLimit, const float InLengthLimit) const
{
	return SMC->Mobility == EComponentMobility::Movable &&
		SMC->Bounds.GetBox().GetVolume() < InVolumeLimit &&
		SMC->Bounds.SphereRadius * 2.f < InLengthLimit;
}

// Scan the current item
void USLItemScanner::ScanItem(AActor* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Scanning %s.."),
			*FString(__func__), __LINE__, *Item->GetName());
}
