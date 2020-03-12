// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLDataComponent.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent) )
class USEMLOG_API USLDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLDataComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
