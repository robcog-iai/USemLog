// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "Viz/SLVizStructs.h"
#include "SLVizQMarker.generated.h"

// Forward declaration
class ASLKnowrobManager;

/**
 *
 */
UENUM()
enum class ESLVizQMarkerMeshType : uint8
{
	Primitive			UMETA(DisplayName = "Primitive"),
	StaticMesh			UMETA(DisplayName = "Static Mesh"),
	SkeletalMesh		UMETA(DisplayName = "Skeletal Mesh"),
};

/**
 *
 */
UENUM()
enum class ESLVizQMarkerType : uint8
{
	Pose				UMETA(DisplayName = "Pose"),
	Trajectory			UMETA(DisplayName = "Trajectory"),
	Timeline			UMETA(DisplayName = "Timeline"),
};

/**
 * 
 */
UCLASS()
class USLVizQMarker : public USLVizQBase
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Virtual implementation of the execute function
	virtual void ExecuteImpl(ASLKnowrobManager* KRManager) override;

public:	
	UPROPERTY(EditAnywhere, Category = "Marker|Edit")
	FString MarkerIdPrefix = "";

	UPROPERTY(EditAnywhere, Category = "Marker")
	FString MarkerId = "";

	UPROPERTY(EditAnywhere, Category = "Marker")
	ESLVizQMarkerType Type = ESLVizQMarkerType::Pose;


	/* Data */
	UPROPERTY(EditAnywhere, Category = "Marker|Data")
	FString Task;

	UPROPERTY(EditAnywhere, Category = "Marker|Data")
	FString Episode;

	UPROPERTY(EditAnywhere, Category = "Marker|Data")
	FString Individual;

	UPROPERTY(EditAnywhere, Category = "Marker|Data")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Marker|Data", meta = (editcondition = "Type==ESLVizQMarkerType::Trajectory || Type==ESLVizQMarkerType::Timeline"))
	float EndTime = -1.f;

	UPROPERTY(EditAnywhere, Category = "Marker|Data", meta = (editcondition = "Type==ESLVizQMarkerType::Trajectory || Type==ESLVizQMarkerType::Timeline"))
	float DeltaT = -1.f;


	/* Timeline */
	UPROPERTY(EditAnywhere, Category = "Marker|Data", meta = (editcondition = "Type==ESLVizQMarkerType::Timeline"))
	FSLVizTimelineParams TimelineParams;


	/* Visual */
	UPROPERTY(EditAnywhere, Category = "Marker|Visual")
	ESLVizQMarkerMeshType MeshType = ESLVizQMarkerMeshType::Primitive;

	UPROPERTY(EditAnywhere, Category = "Marker|Visual", meta = (editcondition = "MeshType!=ESLVizQMarkerMeshType::Primitive"))
	bool bUseOriginalColor = false;

	UPROPERTY(EditAnywhere, Category = "Marker|Visual", meta = (editcondition = "!bUseOriginalColor"))
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Marker|Visual", meta = (editcondition = "!bUseOriginalColor"))
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;


	/* Visual params */
	UPROPERTY(EditAnywhere, Category = "Marker|Visual|Primitive", meta = (editcondition = "MeshType==ESLVizQMarkerMeshType::Primitive"))
	ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box;

	UPROPERTY(EditAnywhere, Category = "Marker|Visual|Primitive", meta = (editcondition = "MeshType==ESLVizQMarkerMeshType::Primitive"))
	float Size = 0.05f;


protected:
	/* Manual interaction */
	UPROPERTY(EditAnywhere, Category = "Manual Interaction|Marker")
	bool bRemoveButton = false;

	UPROPERTY(EditAnywhere, Category = "Manual Interaction|Marker")
	bool bRemoveAllButton = false;


	/* Editor interaction */
	UPROPERTY(EditAnywhere, Category = "Marker|Edit")
	bool bAddSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Marker|Edit", meta = (editcondition = "Type==ESLVizQMarkerType::Timeline"))
	bool bCalcTimelineDurationButton = false;

	UPROPERTY(EditAnywhere, Category = "Children|Edit")
	bool bSyncWithChildrenButton = false;
};
