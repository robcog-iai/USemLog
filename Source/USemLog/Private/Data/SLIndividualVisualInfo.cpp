// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfo.h"
#include "Components/TextRenderComponent.h"

// Sets default values
ASLIndividualVisualInfo::ASLIndividualVisualInfo()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ClassTextSize = 50.f;

	ClassText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ClassTxt"));
	ClassText->SetHorizontalAlignment(EHTA_Center);
	ClassText->SetWorldSize(ClassTextSize);
	ClassText->SetText(FText::FromString(TEXT("ClassVal")));
	RootComponent = ClassText;

	IdText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IdTxt"));
	IdText->SetHorizontalAlignment(EHTA_Center);
	IdText->SetWorldSize(50.f);
	IdText->SetText(FText::FromString(TEXT("IdVal")));
	IdText->SetupAttachment(ClassText);
	IdText->SetRelativeLocation(FVector(0.f, 0.f, -ClassTextSize));
}

// Called when the game starts or when spawned
void ASLIndividualVisualInfo::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLIndividualVisualInfo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

