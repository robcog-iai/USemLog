// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "CoreMinimal.h"

UENUM()
enum class ESLAssetAction : uint8
{
	NONE				UMETA(DisplayName = "NONE"),
	Download			UMETA(DisplayName = "Download Assets"),
	Move				UMETA(DisplayName = "Move Assets"),
	Upload				UMETA(DisplayName = "Upload Assets"),
	MoveAndUpload		UMETA(DisplayName = "Move then Upload Assets"),
};

/*
* Editor logger parameters
*/
struct FSLEditorLoggerParams
{
	// Default ctor
	FSLEditorLoggerParams() :
		bOverwrite(false),
		bWriteSemanticMap(false),
		bClearAllTags(false),
		bWriteClassTags(false),
		bWriteUniqueIdTags(false),
		bWriteUniqueMaskColors(false),
		MinColorManhattanDistance(17),
		bUseRandomColorGeneration(true)
	{};

	// Init ctor
	FSLEditorLoggerParams(bool bNewOverwrite,
		bool bNewWriteSemanticMap,
		bool bNewClearAllTags,
		const FString& InTagTypeToClear,
		const FString& InTagKeyToClear,
		bool bNewWriteClassTags,
		bool bNewWriteUniqueIdTags,
		bool bNewWriteUniqueMaskColors,
		uint8 NewMinColorManhattanDistance,
		bool bNewUseRandomColorGeneration)
		:
		bOverwrite(bNewOverwrite),
		bWriteSemanticMap(bNewWriteSemanticMap),
		bClearAllTags(bNewClearAllTags),
		TagTypeToClear(InTagTypeToClear),
		TagKeyToClear(InTagKeyToClear),
		bWriteClassTags(bNewWriteClassTags),
		bWriteUniqueIdTags(bNewWriteUniqueIdTags),
		bWriteUniqueMaskColors(bNewWriteUniqueMaskColors),
		MinColorManhattanDistance(NewMinColorManhattanDistance),
		bUseRandomColorGeneration(bNewUseRandomColorGeneration)
	{};

	// Overwrite the data (where applicable)
	bool bOverwrite;

	// Write the semantic map
	bool bWriteSemanticMap;

	/* Tag related */
	// Clear all tags in the world
	bool bClearAllTags;

	// Clear only the given tag type (e.g. SemLog)
	FString TagTypeToClear;

	// Clear only the given tag keys (e.g. Class)
	FString TagKeyToClear;

	// Write class properties to tags
	bool bWriteClassTags;

	// Write unique ids to tags
	bool bWriteUniqueIdTags;

	/* Mask color generation */
	// Write unique color masks
	bool bWriteUniqueMaskColors;

	// Guaranteed minimal manhattan distance between the generated mask colors
	uint8 MinColorManhattanDistance;

	// Algorithm to generate the colors (random color generation, or incrementally fill an array and pick random indexes)
	bool bUseRandomColorGeneration;
};