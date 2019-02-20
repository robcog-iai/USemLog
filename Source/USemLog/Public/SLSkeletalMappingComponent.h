// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstance.h"
#include "SLSkeletalMapDataAsset.h"
#include "SLSkeletalMappingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USEMLOG_API USLSkeletalMappingComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLSkeletalMappingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Map of bones to their class names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, FString> BoneClassMap;

	// Map of bones to their mask colors
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, FString> BoneMaskColorMap;

	// Map of bones to their material instances
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, UMaterialInstance*> BoneMaterialInstanceMap;

private:
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USLSkeletalMapDataAsset* LoadFromSkeletalMapDataAsset;
};
