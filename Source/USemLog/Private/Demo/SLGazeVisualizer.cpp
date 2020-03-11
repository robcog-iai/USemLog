// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Demo/SLGazeVisualizer.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "SLSkeletalDataComponent.h"

// UUtils
#include "Tags.h"

// Sets default values
ASLGazeVisualizer::ASLGazeVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;
	// Disable tick by default, will be enabled if the eye tracking is succesfully init
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create the visual component
	GazeVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GazeVisual"));
	GazeVisual->SetMobility(EComponentMobility::Movable);
	GazeVisual->SetGenerateOverlapEvents(false);
	GazeVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = GazeVisual;
	
	// Create the text component
	GazeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GazeText"));	
	GazeText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	GazeText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	GazeText->SetText(FText::FromString("NONE"));
	GazeText->SetWorldSize(2.5f);
	GazeText->SetTextRenderColor(FColor::Red);
	GazeText->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>VisualAsset(TEXT("StaticMesh'/USemLog/Gaze/SM_GazeVisual.SM_GazeVisual'"));
	if (VisualAsset.Succeeded())
	{
		GazeVisual->SetStaticMesh(VisualAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialAsset(TEXT("StaticMesh'/USemLog/Gaze/M_GazeVisual.M_GazeVisual'"));
	if (MaterialAsset.Succeeded())
	{
		VisualMaterial = MaterialAsset.Object;		
	}	
	
	// Default process ptr to none
	GazeTraceFuncPtr = &ASLGazeVisualizer::GazeTrace_NONE;
}

// Called when the game starts or when spawned
void ASLGazeVisualizer::BeginPlay()
{
	Super::BeginPlay();

	/*SetActorEnableCollision(false);
	SetActorEnableCollision(ECollisionEnabled::NoCollision);*/

#if SL_WITH_EYE_TRACKING
	for (TActorIterator<ASLGazeProxy> GazeItr(GetWorld()); GazeItr; ++GazeItr)
	{
		GazeProxy = *GazeItr;
		break;
	}

	if (!GazeProxy || !GazeProxy->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No ASLGazeProxy found in the world, eye tracking will be disabled.."), *FString(__func__), __LINE__);
		return;
	}

	if (!GazeProxy->Start())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not start the eye tracking software.."), *FString(__func__), __LINE__);
		return;
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		CameraManager = PC->PlayerCameraManager;
		if (CameraManager)
		{
			TraceParams = FCollisionQueryParams(FName("EyeTraceParam"), true, CameraManager);
			SetActorTickEnabled(true);
		}
	}

	// Load the semantic data of all actors
	LoadSemanticData();

	// Bind trace function
	if (RayRadius <= KINDA_SMALL_NUMBER)
	{
		GazeTraceFuncPtr = &ASLGazeVisualizer::GazeTrace_Line;
	}
	else
	{
		SphereShape.SetSphere(RayRadius);
		GazeVisual->SetWorldScale3D(FVector(RayRadius * 2.f));
		GazeTraceFuncPtr = &ASLGazeVisualizer::GazeTrace_Sphere;
	}

#else	
	UE_LOG(LogTemp, Error, TEXT("%s::%d Eye tracking module not enabled.."), *FString(__func__), __LINE__);
#endif // SL_WITH_EYE_TRACKING	
}

// Called every frame
void ASLGazeVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if SL_WITH_EYE_TRACKING
	// Get the gaze direction
	FVector RelativeGazeDirection;
	if (GazeProxy->GetRelativeGazeDirection(RelativeGazeDirection))
	{
		const FVector OriginLoc = CameraManager->GetCameraLocation();
		const FVector TargetLoc = CameraManager->GetCameraRotation().RotateVector(OriginLoc + RelativeGazeDirection * RayLength);
		(this->*GazeTraceFuncPtr)(OriginLoc, TargetLoc);
	}
#endif // SL_WITH_EYE_TRACKING
}

// Load info from tags
void ASLGazeVisualizer::LoadSemanticData()
{
	/* Get static mesh data */
	for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
	{		
		FString Id = FTags::GetValue(*SMAItr, "SemLog", "Id");
		FString Class = FTags::GetValue(*SMAItr, "SemLog", "Class");
		FString ColorHex = FTags::GetValue(*SMAItr, "SemLog", "VisMask");

		if (!Id.IsEmpty() && !Class.IsEmpty() && !ColorHex.IsEmpty())
		{
			FSLGazeEntityData EntityData;
			EntityData.ClassName = Class;
			EntityData.Id = Id;
			EntityData.MaskColor = FColor::FromHex(ColorHex);

			EntitySemanticData.Emplace(*SMAItr, EntityData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has missing data in tags, will not be considered for gaze visualization.."),
				*FString(__FUNCTION__), __LINE__, *SMAItr->GetName());
		}
	}

	/* Get skeletal data */
	for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
	{
		FString Id = FTags::GetValue(*SkMAItr, "SemLog", "Id");
		FString Class = FTags::GetValue(*SkMAItr, "SemLog", "Class");

		if (!Id.IsEmpty() && !Class.IsEmpty())
		{
			if (UActorComponent* AC = SkMAItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
			{
				FSLGazeSkelData SkelData;
				SkelData.ClassName = Class;
				SkelData.Id = Id;
				
				// Get the bone (body parts) data from the component
				USLSkeletalDataComponent* SLSkelData = CastChecked<USLSkeletalDataComponent>(AC);
				SLSkelData->Init();				
				for (const auto& DataPair : SLSkelData->SemanticBonesData)
				{
					if (DataPair.Value.IsSet())
					{
						FSLGazeBoneData BoneData;
						BoneData.ClassName = DataPair.Value.Class;
						BoneData.Id = DataPair.Value.Id;
						BoneData.MaskColor = FColor::FromHex(DataPair.Value.VisualMask);

						SkelData.BonesData.Add(DataPair.Key, BoneData);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d %s-%s has missing semantic data, will not be considered for gaze visualization...."),
							*FString(__FUNCTION__), __LINE__, *SkMAItr->GetName(), *DataPair.Key.ToString());
					}
				}

				SkelSemanticData.Emplace(*SkMAItr, SkelData);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has missing semantic body parts data, will not be considered for gaze visualization...."),
					*FString(__FUNCTION__), __LINE__, *SkMAItr->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has missing data in tags, will not be considered for gaze visualization.."),
				*FString(__FUNCTION__), __LINE__, *SkMAItr->GetName());
		}
	}
}

// Process the gaze trace default
void ASLGazeVisualizer::GazeTrace_NONE(const FVector& Origin, const FVector& Target)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Gaze is not going to be processed.."), *FString(__FUNCTION__), __LINE__);
}

// Process the gaze trace default
void ASLGazeVisualizer::GazeTrace_Line(const FVector& Origin, const FVector& Target)
{
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Origin, Target, ECC_Pawn, TraceParams))
	{		
		ProcessGazeHit(HitResult);
	}
}

// Process the gaze trace as a line
void ASLGazeVisualizer::GazeTrace_Sphere(const FVector& Origin, const FVector& Target)
{
	FHitResult HitResult;
	if (GetWorld()->SweepSingleByChannel(HitResult, Origin, Target, FQuat::Identity, ECC_Pawn, SphereShape, TraceParams))
	{
		ProcessGazeHit(HitResult);
	}
}

// Print out the gaze hit info
void ASLGazeVisualizer::ProcessGazeHit(const FHitResult& HitResult)
{
	// Move visual to the hit location
	SetActorLocation(HitResult.Location);

	// Look at camera
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CameraManager->GetCameraLocation()));


	AActor* HitActor = HitResult.Actor.Get();
	if (PrevActor != HitActor)
	{		
		GazeText->SetText(FText::FromString(HitActor->GetName()));


		UE_LOG(LogTemp, Warning, TEXT("%s::%d Hit=%s; BoneName=%s;"),
			*FString(__FUNCTION__), __LINE__, *HitActor->GetName(), *HitResult.BoneName.ToString());

		PrevActor = HitActor;
	}
	else if(!PrevBoneName.IsEqual(HitResult.BoneName))
	{
		GazeText->SetText(FText::FromString(HitActor->GetName()));

		PrevBoneName = HitResult.BoneName;
	}
}

