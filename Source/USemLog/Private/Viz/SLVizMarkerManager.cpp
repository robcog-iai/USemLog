// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizMarkerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
ASLVizMarkerManager::ASLVizMarkerManager()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VizRoot"));
	PrimaryActorTick.bCanEverTick = false;
}

// Clear marker
void ASLVizMarkerManager::ClearMarker(USLVizMarker* Marker)
{	
	Marker->DestroyComponent();
	Markers.Remove(Marker);	
}

// Clear hihlight marker
void ASLVizMarkerManager::ClearMarker(USLVizHighlightMarker* HighlightMarker)
{
	HighlightMarker->DestroyComponent();
	HighlightMarkers.Remove(HighlightMarker);
}

// Clear all markers
void ASLVizMarkerManager::ClearAllMarkers()
{
	for (auto& M : Markers)
	{
		M->DestroyComponent();
	}
	Markers.Empty();

	for (auto& HM : HighlightMarkers)
	{
		HM->DestroyComponent();
	}
	HighlightMarkers.Empty();
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


/* Highlight markers */
// Create a highlight marker for the given static mesh component
USLVizHighlightMarker* ASLVizMarkerManager::CreateHighlightMarker(UStaticMeshComponent* SMC, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	USLVizHighlightMarker* HighlightMarker = CreateNewHighlightMarker();
	HighlightMarker->Init(SMC, Color, Type);
	return HighlightMarker;
}

// Create a highlight marker for the given skeletal mesh component
USLVizHighlightMarker* ASLVizMarkerManager::CreateHighlightMarker(USkeletalMeshComponent* SkMC, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	USLVizHighlightMarker* HighlightMarker = CreateNewHighlightMarker();
	HighlightMarker->Init(SkMC, Color, Type);
	return HighlightMarker;
}

// Create a highlight marker for the given bone (material index) skeletal mesh component
USLVizHighlightMarker* ASLVizMarkerManager::CreateHighlightMarker(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	USLVizHighlightMarker* HighlightMarker = CreateNewHighlightMarker();
	HighlightMarker->Init(SkMC, MaterialIndex, Color, Type);
	return HighlightMarker;
}

// Create a highlight marker for the given bones (material indexes) skeletal mesh component
USLVizHighlightMarker * ASLVizMarkerManager::CreateHighlightMarker(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	USLVizHighlightMarker* HighlightMarker = CreateNewHighlightMarker();
	HighlightMarker->Init(SkMC, MaterialIndexes, Color, Type);
	return HighlightMarker;
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

// Create and register the highlight marker
USLVizHighlightMarker* ASLVizMarkerManager::CreateNewHighlightMarker()
{
	USLVizHighlightMarker* HighlightMarker = NewObject<USLVizHighlightMarker>(this);
	HighlightMarker->RegisterComponent();
	AddInstanceComponent(HighlightMarker); // Makes it appear in the editor
	//AddOwnedComponent(Marker);
	HighlightMarker->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	HighlightMarkers.Emplace(HighlightMarker);
	return HighlightMarker;
}