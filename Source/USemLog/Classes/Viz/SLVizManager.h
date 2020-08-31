// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "SLVizManager.generated.h"

/*
* Trajectory marker primitive types
*/
UENUM()
enum class ESLVizMeshType : uint8
{
	Box				UMETA(DisplayName = "Box"),
	Sphere			UMETA(DisplayName = "Sphere"),
	Cylinder		UMETA(DisplayName = "Cylinder"),
	Arrow			UMETA(DisplayName = "Arrow"),
	Axis			UMETA(DisplayName = "Axis"),
};


/*
* Highlight material types
*/
UENUM()
enum class ESLVizHighlightType : uint8
{
	Additive			UMETA(DisplayName = "Additive"),
	Translucent			UMETA(DisplayName = "Translucent"),
};

USTRUCT()
struct FSLVizMaterialList
{
	GENERATED_USTRUCT_BODY()
	FSLVizMaterialList() {}

	UPROPERTY()
	TArray<UMaterialInterface*> Materials;
};



/*
* Class for visualizing various markers (mesh, skeletal mesh, basic primitives), as points or trajectories
*/
UCLASS()
class USEMLOG_API ASLVizManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLVizManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Init the mappings between the unique ids and the entities
	void Init();

	// Clears all markers, reset key counter
	void Clear();

	
	/* Markers */
	// Create a single pose marker with default orientation
	int32 CreateMarker(const FVector& Location, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit = false);
	
	// Create a single pose marker
	int32 CreateMarker(const FTransform& Pose, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit = false);

	// Create a trajectory marker with default orientation
	int32 CreateMarker(const TArray<FVector>& Locations, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit = false);
	
	// Create a trajectory marker
	int32 CreateMarker(const TArray<FTransform>& Poses, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit = false);

	// Append a pose to the marker
	bool AppendMarker(int32 Key, const FTransform& Pose, FVector Scale);

	// Append an array to the marker
	bool AppendMarker(int32 Key, const TArray<FTransform>& Poses, FVector Scale);
	
	// Remove marker
	// todo
	bool RemoveMarker(int32 Key);

	// Remove all markers
	// todo
	void ClearMarkers();

	// see virtual bool UpdateInstanceTransform
	// UpdateMarkerInstance(int32 Key, int32 InstanceIndex, const FTransform& Pose, FVectorScale)
	// NumMarkerInstance()
	
	
	/* Clones */
	// Create clone copy of the given entity id
	int32 CreateEntityClone(const FString& Id, const FTransform& Transform);

	// Get the entity clone
	AStaticMeshActor* GetEntityClone(int32 Key);

	// Remove the entity clone marker
	bool RemoveEntityClone(int32 Key);

	// Remove all entity clones
	void ClearEntityClones();

	// Create a skeletal clone
	int32 CreateSkeletalClone(const FString& Id, const FTransform& Transform);

	// Get access to the skeletal clone
	ASkeletalMeshActor* GetSkeletalClone(int32 Key);

	// Remove skeletal clone
	bool RemoveSkeletalClone(int32 Key);

	// Remove all skeletal clones
	void ClearSkeletalClones();
	
	// Remove all clones
	void ClearClones();

	/*Highlight*/
	// Highlight entity
	void Highlight(const FString& Id, FLinearColor Color, ESLVizHighlightType HighlightType);

	// Remove highlight of entity
	void RemoveHighlight(const FString& Id);

	// Remove all highlight of entity
	void ClearHighlights();

	/*Get Actor in the world*/
	// Get Skeletal Mesh Actor
	ASkeletalMeshActor* GetSkeletalMeshActor(const FString& Id);

	// Get Static Mesh Actor
	AStaticMeshActor* GetStaticMeshActor(const FString& Id);
	
private:
	// Load marker meshes and materials
	bool LoadMarkerAssets();
	
	// Load the mappings of the unique ids to the entities
	void LoadEntityMappings();

	// Get mesh instance from enum type
	UStaticMesh* GetMeshFromType(ESLVizMeshType Type) const;
	
private:
	// True if the manager is initialized
	bool bIsInit;

	// Key used for accessing the viz marker
	int32 VizKey;

	
	/* Markers */
	// Map of the existing pose and trajectory markers
	UPROPERTY()
	TMap<int32, UInstancedStaticMeshComponent*> Markers;

	// Pointers to the mesh type assets
	UPROPERTY()
	UStaticMesh* MeshBox;
	UPROPERTY()
	UStaticMesh* MeshSphere;
	UPROPERTY()
	UStaticMesh* MeshCylinder;
	UPROPERTY()
	UStaticMesh* MeshArrow;
	UPROPERTY()
	UStaticMesh* MeshAxis;

	// Pointer to the dynamic material
	UPROPERTY()
	UMaterial* MaterialLit;
	UPROPERTY()
	UMaterial* MaterialUnlit;
	UPROPERTY()
	UMaterial* MaterialHighlightAdditive;
	UPROPERTY()
	UMaterial* MaterialHighlightTranslucent;
	
	/* Clones */
	// Mapping between the unique keys and the cloned static mesh entities
	UPROPERTY()
	TMap<int32, AStaticMeshActor*> EntityClones;

	// Mapping between the unique keys and the cloned skeletal mesh entities
	UPROPERTY()
	TMap<int32, ASkeletalMeshActor*> SkeletalClones;

	// Id to static mesh actor
	UPROPERTY()
	TMap<FString, AStaticMeshActor*> IdToSMA;

	// Id to skeletal mesh actor
	UPROPERTY()
	TMap<FString, ASkeletalMeshActor*> IdToSkMA;

	// Id to the original material of the mesh
	UPROPERTY()
	TMap<FString, FSLVizMaterialList> IdToOriginalMaterials;
};
