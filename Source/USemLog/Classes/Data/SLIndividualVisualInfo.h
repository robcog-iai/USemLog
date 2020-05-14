// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLIndividualVisualInfo.generated.h"

UCLASS()
class ASLIndividualVisualInfo : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLIndividualVisualInfo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Class text
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	class UTextRenderComponent* ClassText;

	// Id text
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	class UTextRenderComponent* IdText;

private:
	// Class text size, used for ofsseting location of other text
	float ClassTextSize;
};
