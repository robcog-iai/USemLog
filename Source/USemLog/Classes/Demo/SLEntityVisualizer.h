// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLEntityVisualizer.generated.h"

/*
* Struct holding the text info about the actors
*/
struct FSLEntityTextInfo
{
	// Class name
	FString ClassName;

	// Unique identifier
	FString Id;
};

/*
* Demo actor to attach text components with text data to the world entities
*/
UCLASS()
class ASLEntityVisualizer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLEntityVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// True if it should tick in the editor
	virtual bool ShouldTickIfViewportsOnly() const override;

private:
	// Update orientation

	// Update locations and values	

	// Create visual components

};
