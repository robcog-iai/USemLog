// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisViewActor.h"
#include "Tags.h"
#if WITH_EDITOR
#include "Components/ArrowComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLVisViewActor::ASLVisViewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bInit = false;

#if WITH_EDITORONLY_DATA
	ArrowVis = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("SLVisViewArrowComponent"));
	if (ArrowVis)
	{
		ArrowVis->SetupAttachment(RootComponent);
		ArrowVis->ArrowColor = FColor::Blue;
		ArrowVis->bTreatAsASprite = true;
		ArrowVis->bLightAttachment = true;
		ArrowVis->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLVisViewActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called when the game starts or when spawned
bool ASLVisViewActor::Init()
{
	if (bInit && !Class.IsEmpty() && !Id.IsEmpty())
	{
		return true;
	}
	else
	{
		Class = FTags::GetValue(Tags, "SemLog", "Class");
		Id = FTags::GetValue(Tags, "SemLog", "Id");

		if (!Class.IsEmpty() && !Id.IsEmpty())
		{
			bInit = true;
			return true;
		}
		else
		{
			bInit = false;
			return false;
		}
	}
}

