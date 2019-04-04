// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVisViewActor.generated.h"

UCLASS()
class USEMLOGVISION_API ASLVisViewActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisViewActor();

	// Set parameters from tags
	bool Init();

	// True if it is successfully initialized
	bool IsInit() const { return bIsInit; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Class of the view
	FString Class;

	// Unique id
	FString Id;

private:
#if WITH_EDITORONLY_DATA
	// Location and orientation visualization of the component
	UPROPERTY()
	class UArrowComponent* ArrowVis;
#endif // WITH_EDITORONLY_DATA

	// True if successfully initialized
	bool bIsInit;
};
