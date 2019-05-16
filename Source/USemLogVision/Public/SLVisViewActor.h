// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "SLVisViewActor.generated.h"

UCLASS(ClassGroup = (SL), DisplayName = "SL Vis View")
class USEMLOGVISION_API ASLVisViewActor : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisViewActor();

	// Set parameters from tags
	void Init();

	// True if it is successfully initialized
	bool IsInit() const { return bIsInit; };

	// Get the class of the view
	FString GetClass() const { return Class; };

	// Get the id of the view
	FString GetId() const { return Id; };

protected:
	// Only called during gameplay, used for attaching to components of parent (if requested)
	virtual void PostInitializeComponents() override;

	// Called when the games starts
	virtual void BeginPlay() override;

	// Called every update frame
	virtual void Tick(float DeltaSeconds) override;

private:
#if WITH_EDITORONLY_DATA
	// Camera used for visual localization
	UPROPERTY()
	class UCameraComponent* VisCamera;
#endif // WITH_EDITORONLY_DATA

	// If you want to attach the actor to a component of its attach parent actor,
	// use the ComponentTagStamp value as a tag value on the component you want to attach the actor to
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName ComponentTagStamp;

	// True if successfully initialized
	bool bIsInit;

	// Class of the view
	FString Class;

	// Unique id
	FString Id;

	// Reference to the component to follow
	USceneComponent* CompToFollow;
};
