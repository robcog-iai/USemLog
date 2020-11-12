// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class ResponseType
{
	None,
	TEXT,
	FILE
};

struct FSLKRResponse
{
	ResponseType Type;
	FString Text;
	FString FileName;
	TArray<uint8> FileData;
};
