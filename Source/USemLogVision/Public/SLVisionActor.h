// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVisionActor.generated.h"

UCLASS(ClassGroup = (SL), hidecategories = (HLOD, Mobile, Cooking, AssetUserData))
class USEMLOGVISION_API ASLVisionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Semantic vision logger component
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	class USLVisionComponent* SLVisionComponent;
	
};
