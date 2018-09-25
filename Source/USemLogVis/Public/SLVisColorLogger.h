// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLVisColorLogger.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USEMLOGVIS_API USLVisColorLogger : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLVisColorLogger();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
