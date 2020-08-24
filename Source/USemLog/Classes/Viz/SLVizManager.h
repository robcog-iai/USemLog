// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "GameFramework/Actor.h"
#include "SLVizManager.generated.h"

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
    // Called every frame
    virtual void Tick( float DeltaSeconds ) override;

};