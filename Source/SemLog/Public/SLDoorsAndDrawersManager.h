// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLRuntimeManager.h"
#include "SLDoorsAndDrawersManager.generated.h"

UCLASS()
class SEMLOG_API ASLDoorsAndDrawersManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLDoorsAndDrawersManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Semantic events runtime manager
	ASLRuntimeManager* SemanticEventsRuntimeManager;
};
