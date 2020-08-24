// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "SLVizHighlightMarker.generated.h"

/*
* Highlight material types
*/
UENUM()
enum class ESLVizHighlightMarkerType : uint8
{
	Additive			UMETA(DisplayName = "Additive"),
	Translucent			UMETA(DisplayName = "Translucent"),
};

/**
 * Highlights a static or skeletal mesh by creating a clone in the same position
 */
UCLASS()
class USEMLOG_API USLVizHighlightMarker : public USceneComponent
{
	GENERATED_BODY()
	
public:
	// Constructor
	USLVizHighlightMarker();

	// Highlight the given static mesh by creating a clone
	void Init(UStaticMeshComponent* SMC,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Highlight the given skeletal mesh by creating a clone
	void Init(USkeletalMeshComponent* SkMC,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Highlight the given bone (material index) by creating a clone
	void Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Highlight the given bones (material indexes) by creating a clone
	void Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Call this if you want to notify the owner (manager) of the destruction
	bool DestroyThroughManager();

	//~ Begin ActorComponent Interface
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	//~ End ActorComponent Interface

protected:
	// Load highligh material assets
	void LoadAssets();

private:
	// Used as a clone if a static mesh component will be highlighted
	UStaticMeshComponent* HighlightSMC;

	// Used as a clone if a skeletal mesh component will be highlighted
	UPoseableMeshComponent* HighlightSkMC;

	/* Highligh dynamic materials */
	UMaterial* MaterialHighlightAdditive;
	UMaterial* MaterialHighlightTranslucent;
};
