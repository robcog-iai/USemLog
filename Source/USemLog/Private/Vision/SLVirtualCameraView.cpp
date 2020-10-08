// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVirtualCameraView.h"
#include "Camera/CameraComponent.h"

// Ctor
ASLVirtualCameraView::ASLVirtualCameraView()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// Scale the camera mesh size for visual purposes
	GetCameraComponent()->SetWorldScale3D(FVector(0.1));
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
	// Mimics a button
	bResetTransformButtton = false;
#endif // WITH_EDITOR
}

// Called when the game starts or when spawned
void ASLVirtualCameraView::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASLVirtualCameraView::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] [%s] Loc= \t %s"),
	//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName(), *GetActorLocation().ToCompactString());
}

// Get the semantic class name of the virtual camera
FString ASLVirtualCameraView::GetClassName()
{
	//if(ClassName.IsEmpty())
	//{
	//	ClassName =  FTags::GetValue(this, "SemLog", "Class");
	//}
	return ClassName;
}

// Get the unique id of the virtual camera
FString ASLVirtualCameraView::GetId()
{
	//if (Id.IsEmpty())
	//{
	//	Id = FTags::GetValue(this, "SemLog", "Id");
	//}
	return Id;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLVirtualCameraView::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ASLVirtualCameraView, bResetTransformButtton))
	{
		SetActorRelativeTransform(FTransform::Identity);
		bResetTransformButtton = false;
	}
}
#endif // WITH_EDITOR