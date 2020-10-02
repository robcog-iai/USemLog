// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "SLVizMarker.generated.h"

// Forward declarations
class USLVizAssets;

/*
* Active visual type
*/
UENUM()
enum class ESLVizVisualType : uint8
{
	NONE			UMETA(DisplayName = "NONE"),
	Static			UMETA(DisplayName = "Static"),
	Skeletal		UMETA(DisplayName = "Skeletal")
};

/*
* Marker primitive types
*/
UENUM()
enum class ESLVizMarkerType : uint8
{
	Box				UMETA(DisplayName = "Box"),
	Sphere			UMETA(DisplayName = "Sphere"),
	Cylinder		UMETA(DisplayName = "Cylinder"),
	Arrow			UMETA(DisplayName = "Arrow"),
	Axis			UMETA(DisplayName = "Axis"),
	Clone			UMETA(DisplayName = "Clone")
};


/**
 * Marker parameters
 */
USTRUCT()
struct FSLVizMarkerVisualParams
{
	GENERATED_BODY();

	// Color
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	// Material lit property
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUnlit = false;

	// Visual scale
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FVector Scale = FVector(0.1f);

	// Visual marker type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizMarkerType Type = ESLVizMarkerType::Box;

	// Clone maker static mesh visual
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	UStaticMeshComponent* SMC = nullptr;

	// Clone maker skeletal mesh visual
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USkeletalMeshComponent* SkelMC = nullptr;

	// Use the original color of the cloned mesh
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCloneOriginalMaterial = false;

	// The material instances to apply the materials to (use all slots if emtpy)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<int32> MaterialIndexes;
};

/**
 * Class capable of visualizing multiple types of markers as instanced static meshes
 */
UCLASS()
class USEMLOG_API USLVizMarker : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	// Constructor
	USLVizMarker();

	// Set the visual representation
	void SetVisual(const FSLVizMarkerVisualParams& VisualParams);


	// Set the visuals of the marker
	void Init(ESLVizMarkerType Type = ESLVizMarkerType::Box,
		const FVector& InScale = FVector(0.1f),
		const FLinearColor& Color = FLinearColor::Green,
		bool bUnlit = false);

	// Set the visuals of the marker from the static mesh component with its original colors
	void Init(UStaticMeshComponent* SMC);

	// Set the visuals of the marker from the static mesh component with custom color
	void Init(UStaticMeshComponent* SMC, const FLinearColor& Color, bool bUnlit = false);

	// Set the visuals of the marker from the skeletal mesh component with its original colors
	void Init(USkeletalMeshComponent* SkMC);

	// Set the visuals of the marker from the skeletal mesh component with custom color
	void Init(USkeletalMeshComponent* SkMC, const FLinearColor& Color, bool bUnlit = false);

	// Set the visuals of the marker from the bone (material index) skeletal mesh component with its original colors
	void Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex);

	// Set the visuals of the marker from the bone (material index) skeletal mesh component with custom color
	void Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FLinearColor& Color, bool bUnlit = false);

	// Set the visuals of the marker from the bone (material index) skeletal mesh component with its original colors
	void Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes);

	// Set the visuals of the marker from the bone (material index) skeletal mesh component with custom color
	void Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FLinearColor& Color, bool bUnlit = false);

	// Add instances at location
	void Add(const FVector& Location);

	// Add instances at pose
	void Add(const FTransform& Pose);

	// Add instances with the locations
	void Add(const TArray<FVector>& Locations);
	
	// Add instances with the poses
	void Add(const TArray<FTransform>& Poses);

	// Add skeletal pose
	void Add(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose);

	// Add skeletal poses
	void Add(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses);

	//~ Begin ActorComponent Interface
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	//~ End ActorComponent Interface

protected:
	// Clear any previously set related data (mesh / materials)
	void Reset();

	// Create and register a new poseable mesh component
	UPoseableMeshComponent* CreateNewSkeletalInstance();

	// Load marker mesh and material assets
	void LoadAssets();

	// Load assets container
	bool LoadAssetsContainer();

	// Get the marker static mesh from its type
	UStaticMesh* GetPrimitiveMarkerMesh(ESLVizMarkerType Type) const;

protected:
	// Scale of the visuals
	FVector Scale;

	// Enum showing the current active visual type
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ESLVizVisualType CurrentVisualType;

	/* Skeletal marker components helpers */
	// Skeletal mesh marker to render
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	class USkeletalMesh* SkeletalMesh;

	// Skeletal materials
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<int32, class UMaterialInterface*> SkeletalMaterials;

	// Instances of the skeletal marker
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<UPoseableMeshComponent*> SkeletalInstances;

	// Dynamic material
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	UMaterialInstanceDynamic* DynamicMaterial;

	/* Marker visual static meshes */
	UStaticMesh* MeshBox;	
	UStaticMesh* MeshSphere;
	UStaticMesh* MeshCylinder;
	UStaticMesh* MeshArrow;
	UStaticMesh* MeshAxis;

	/* Marker materials */
	UMaterial* MaterialLit;
	UMaterial* MaterialUnlit;
	UMaterial* MaterialInvisible;

	// Assets container
	USLVizAssets* VizAssetsContainer;

	/* Constants */
	static constexpr auto AssetsContainerPath = TEXT("SLVizAssets'/USemLog/Viz/SL_VizAssetsContainer.SL_VizAssetsContainer'");
};
