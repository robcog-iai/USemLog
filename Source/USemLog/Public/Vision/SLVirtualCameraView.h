// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "SLVirtualCameraView.generated.h"

/**
 * Virtual camera logging the transformations of the cameras that need to re-render the scene
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Virtual Camera")
class USEMLOG_API ASLVirtualCameraView : public ACameraActor
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	ASLVirtualCameraView();

protected:	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Get the semantic class name of the virtual camera
	FString GetClassName();

	// Get the unique id of the virtual camera
	FString GetId();

#if WITH_EDITOR
private:
	// UObject interface
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UObject interface
#endif // WITH_EDITOR

private:
	// Semantic class name of the virtual vision camera
	FString ClassName;

	// Semantic uinque id of the virtual vision camera
	FString Id;

#if WITH_EDITORONLY_DATA
	// Mimics a button to set its relative tranform to 0
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bResetTransformButtton;
#endif // WITH_EDITORONLY_DATA
};
