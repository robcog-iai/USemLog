// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "SLEnums.h"
#include "SLItem.generated.h"

/**
*
*/
UCLASS()
class SEMLOG_API ARItem : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// Constructor, sets default values for object
	ASLItem();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Ontology class name
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString ItemClassName;

	// Ontology class name
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESemLogType ItemLogType;
};