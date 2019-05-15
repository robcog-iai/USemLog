// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisViewActor.h"
#include "SLVisViewComponent.h"
#include "Tags.h"
#if WITH_EDITOR
#include "Camera/CameraComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLVisViewActor::ASLVisViewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;

#if WITH_EDITORONLY_DATA
	VisCamera = CreateEditorOnlyDefaultSubobject<UCameraComponent>(TEXT("SLVisViewCamera"));
	if (VisCamera)
	{
		VisCamera->SetupAttachment(GetRootComponent());
		VisCamera->SetWorldScale3D(FVector(0.25));
	}
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLVisViewActor::Init()
{
	if (!bIsInit)
	{
		Class = FTags::GetValue(Tags, "SemLog", "Class");
		Id = FTags::GetValue(Tags, "SemLog", "Id");

		if (!Class.IsEmpty() && !Id.IsEmpty())
		{
			// Init even if component attachment might fail
			bIsInit = true;
		}
	}
}

// Only called during gameplay, used for attaching to components of parent (if requested)
void ASLVisViewActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Check if the actor should be attached to a component of its attached parent actor
	if (ComponentTagStamp != NAME_None)
	{
		if (GetAttachParentActor())
		{
			TArray<UActorComponent*> ComponentsWithTagStamp =
				GetAttachParentActor()->GetComponentsByTag(USceneComponent::StaticClass(), ComponentTagStamp);
			if (ComponentsWithTagStamp.Num() == 1)
			{
				USceneComponent* CompToAttachTo = CastChecked<USceneComponent>(ComponentsWithTagStamp[0]);
				AttachToComponent(CompToAttachTo, FAttachmentTransformRules::SnapToTargetIncludingScale);
			}
			else if (ComponentsWithTagStamp.Num() > 1)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Multiple components found to attach to with tag stamp=%s, attaching to first one"),
					*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
				USceneComponent* CompToAttachTo = CastChecked<USceneComponent>(ComponentsWithTagStamp[0]);
				AttachToComponent(CompToAttachTo, FAttachmentTransformRules::SnapToTargetIncludingScale);
			}
			else if (ComponentsWithTagStamp.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d No components found to attach to with tag stamp=%s"),
					*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Attached parent is not set, cannot search for the component with the tag stamp=%s"),
				*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
		}
	}
}