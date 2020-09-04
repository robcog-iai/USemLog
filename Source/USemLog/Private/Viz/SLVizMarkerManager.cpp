// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizMarkerManager.h"
#include "Viz/SLVizHighlightMarker.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
ASLVizMarkerManager::ASLVizMarkerManager()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SL_VizMarkerManagerRoot"));

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Called when actor removed from game or game ended
void ASLVizMarkerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearAllMarkers();
}

// Clear marker
void ASLVizMarkerManager::ClearMarker(USLVizMarker* Marker)
{	
	Marker->DestroyComponent();
	Markers.Remove(Marker);	
}

// Clear all markers
void ASLVizMarkerManager::ClearAllMarkers()
{
	for (const auto& M : Markers)
	{
		if (M->IsValidLowLevel())
		{
			M->DestroyComponent();
		}
	}
	Markers.Empty();
}


/* Primitive static mesh visual markers */
// Create marker at location
USLVizMarker* ASLVizMarkerManager::CreateMarker(const FVector& Location,
	ESLVizMarkerType Type, const FVector& Scale, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->Init(Type, Scale, Color, bUnlit);
	Marker->Add(Location);
	return Marker;
}

// Create marker with orientation
USLVizMarker* ASLVizMarkerManager::CreateMarker(const FTransform& Pose,
	ESLVizMarkerType Type, const FVector& Scale, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->Init(Type, Scale, Color, bUnlit);
	Marker->Add(Pose);	
	return Marker;
}

// Create a marker array at the given locations
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<FVector>& Locations,
	ESLVizMarkerType Type, const FVector& Scale, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->Init(Type, Scale, Color, bUnlit);
	Marker->Add(Locations);
	return Marker;
}

// Create a marker array with orientations
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<FTransform>& Poses,
	ESLVizMarkerType Type, const FVector& Scale, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->Init(Type, Scale, Color, bUnlit);
	Marker->Add(Poses);
	return Marker;
}


/* Static mesh visual markers */
// Create marker at location
USLVizMarker* ASLVizMarkerManager::CreateMarker(const FVector& Location, UStaticMeshComponent* SMC,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SMC);
	}
	else
	{
		Marker->Init(SMC, Color, bUnlit);
	}	
	Marker->Add(Location);
	return Marker;
}

// Create marker with orientation
USLVizMarker* ASLVizMarkerManager::CreateMarker(const FTransform& Pose, UStaticMeshComponent* SMC,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SMC);
	}
	else
	{
		Marker->Init(SMC, Color, bUnlit);
	}
	Marker->Add(Pose);
	return Marker;
}

// Create a marker array at the given locations
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<FVector>& Locations, UStaticMeshComponent* SMC,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SMC);
	}
	else
	{
		Marker->Init(SMC, Color, bUnlit);
	}
	Marker->Add(Locations);
	return Marker;
}

// Create a marker array with orientations
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<FTransform>& Poses, UStaticMeshComponent* SMC,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SMC);
	}
	else
	{
		Marker->Init(SMC, Color, bUnlit);
	}
	Marker->Add(Poses);
	return Marker;
}


/* Skeletal mesh visual markers */
// Create skeletal marker 
USLVizMarker* ASLVizMarkerManager::CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose, USkeletalMeshComponent* SkMC,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SkMC);
	}
	else
	{
		Marker->Init(SkMC, Color, bUnlit);
	}
	Marker->Add(SkeletalPose);
	return Marker;
}

// Create skeletal marker array at the given poses
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses, USkeletalMeshComponent* SkMC,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SkMC);
	}
	else
	{
		Marker->Init(SkMC, Color, bUnlit);
	}
	Marker->Add(SkeletalPoses);
	return Marker;
}

// Create skeletal marker array for the given bone (material index) at the given pose
USLVizMarker* ASLVizMarkerManager::CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose, USkeletalMeshComponent* SkMC, int32 MaterialIndex,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SkMC, MaterialIndex);
	}
	else
	{
		Marker->Init(SkMC, MaterialIndex, Color, bUnlit);
	}
	Marker->Add(SkeletalPose);
	return Marker;
}

// Create skeletal marker array for the given bone (material index) at the given poses
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses, USkeletalMeshComponent* SkMC, int32 MaterialIndex,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SkMC, MaterialIndex);
	}
	else
	{
		Marker->Init(SkMC, MaterialIndex, Color, bUnlit);
	}
	Marker->Add(SkeletalPoses);
	return Marker;
}

// Create skeletal marker array for the given bones (material indexes) at the given pose
USLVizMarker* ASLVizMarkerManager::CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose, USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SkMC, MaterialIndexes);
	}
	else
	{
		Marker->Init(SkMC, MaterialIndexes, Color, bUnlit);
	}
	Marker->Add(SkeletalPose);
	return Marker;
}

// Create skeletal marker array for the given bones (material indexes) at the given poses
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses, USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
	bool bUseOriginalMaterials, const FLinearColor& Color, bool bUnlit)
{
	USLVizMarker* Marker = CreateNewMarker();
	if (bUseOriginalMaterials)
	{
		Marker->Init(SkMC, MaterialIndexes);
	}
	else
	{
		Marker->Init(SkMC, MaterialIndexes, Color, bUnlit);
	}
	Marker->Add(SkeletalPoses);
	return Marker;
}

// Create and register the marker
USLVizMarker* ASLVizMarkerManager::CreateNewMarker()
{
	USLVizMarker* Marker = NewObject<USLVizMarker>(this);
	Marker->RegisterComponent();
	AddInstanceComponent(Marker); // Makes it appear in the editor
	//AddOwnedComponent(Marker);
	Marker->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	Markers.Emplace(Marker);
	return Marker;
}
