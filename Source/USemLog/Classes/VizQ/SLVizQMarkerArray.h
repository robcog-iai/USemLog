// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "Viz/SLVizStructs.h"
#include "SLVizQMarkerArray.generated.h"

// Forward declaration
class ASLKnowrobManager;

/**
 *
 */
UENUM()
enum class ESLVizQMarkerArrayMeshType : uint8
{
	Primitive			UMETA(DisplayName = "Primitive"),
	StaticMesh			UMETA(DisplayName = "Static Mesh"),
	SkeletalMesh		UMETA(DisplayName = "Skeletal Mesh"),
};

/**
 *
 */
UENUM()
enum class ESLVizQMarkerArrayType : uint8
{
	Pose				UMETA(DisplayName = "Pose"),
	Trajectory			UMETA(DisplayName = "Trajectory"),
	Timeline			UMETA(DisplayName = "Timeline"),
};

/**
 * 
 */
UCLASS()
class USLVizQMarkerArray : public USLVizQBase
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
	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit")
	FString MarkerIdPrefix = "";

	UPROPERTY(EditAnywhere, Category = "MarkerArray")
	TArray<FString> MarkerIds;

	UPROPERTY(EditAnywhere, Category = "MarkerArray")
	ESLVizQMarkerArrayType Type = ESLVizQMarkerArrayType::Pose;


	/* Data */
	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data")
	FString Task;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data")
	FString Episode;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data")
	TArray<FString> Individuals;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data", meta = (editcondition = "Type==ESLVizQMarkerArrayType::Trajectory || Type==ESLVizQMarkerArrayType::Timeline"))
	float EndTime = -1.f;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data", meta = (editcondition = "Type==ESLVizQMarkerArrayType::Trajectory || Type==ESLVizQMarkerArrayType::Timeline"))
	float DeltaT = -1.f;


	/* Timeline */
	UPROPERTY(EditAnywhere, Category = "MarkerArray|Data", meta = (editcondition = "Type==ESLVizQMarkerArrayType::Timeline"))
	FSLVizTimelineParams TimelineParams;


	/* Visual */
	UPROPERTY(EditAnywhere, Category = "MarkerArray|Visual")
	ESLVizQMarkerArrayMeshType MeshType = ESLVizQMarkerArrayMeshType::Primitive;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Visual", meta = (editcondition = "MeshType!=ESLVizQMarkerArrayMeshType::Primitive"))
	bool bUseOriginalColor = false;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Visual", meta = (editcondition = "!bUseOriginalColor"))
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Visual", meta = (editcondition = "!bUseOriginalColor"))
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;


	/* Visual params */
	UPROPERTY(EditAnywhere, Category = "MarkerArray|Visual|Primitive", meta = (editcondition = "MeshType==ESLVizQMarkerArrayMeshType::Primitive"))
	ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Visual|Primitive", meta = (editcondition = "MeshType==ESLVizQMarkerArrayMeshType::Primitive"))
	float Size = 0.05f;

protected:
	/* Manual interaction */
	UPROPERTY(EditAnywhere, Category = "Manual Interaction|MarkerArray")
	bool bRemoveButton = false;

	UPROPERTY(EditAnywhere, Category = "Manual Interaction|MarkerArray")
	bool bRemoveAllButton = false;


	/* Editor interaction */
	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit")
	bool bAddSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit", meta = (editcondition = "Type==ESLVizQMarkerType::Timeline"))
	bool bCalcTimelineDurationButton = false;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit")
	bool bRemoveSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit")
	bool bSyncMarkerAndIndividualsButton = false;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit")
	bool bOverwrite = false;

	UPROPERTY(EditAnywhere, Category = "MarkerArray|Edit")
	bool bEnsureUniqueness = true;

	UPROPERTY(EditAnywhere, Category = "Children|Edit")
	bool bSyncWithChildrenButton = false;
};
