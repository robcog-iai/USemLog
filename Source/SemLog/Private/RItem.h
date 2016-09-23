// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "REnums.h"
#include "RItem.generated.h"

/**
*
*/
UCLASS()
class SEMLOG_API ARItem : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// Constructor, sets default values for object
	ARItem();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Ontology class name
	UPROPERTY(EditAnywhere, Category = "Semlog|Semantic Map")
	FString ItemClassName;

	// Ontology class name
	UPROPERTY(EditAnywhere, Category = "Semlog|Semantic Logger")
	ERLogType ItemLogType;
};