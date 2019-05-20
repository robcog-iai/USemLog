// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

/**
 * Editor mode toolkit
 */
class FSLEdModeToolkit : public FModeToolkit
{
public:
	// Ctor
	FSLEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

private:
	/** Callbacks */
	// Return true if any actors are selected in the viewport
	bool AreActorsSelected();

	// Generate semantic map from editor world
	FReply GenerateSemanticMap();

	// Set flag attribute depending on the checkbox state
	void OnCheckedOverwriteSemanticMap(ECheckBoxState NewCheckedState);

	// Generate new semantic ids
	FReply GenerateNewSemanticIds();

	// Remove all semantic ids
	FReply RemoveAllSemanticIds();

	// Generate semantic ids for constraints
	FReply SemanticallyAnnotateConstraints();

	// Name semantic classes from asset name
	FReply SetClassNamesToDefault();

	// Set flag attribute depending on the checkbox state
	void OnCheckedOverwriteClassNames(ECheckBoxState NewCheckedState);

	// Set unique mask colors in hex for the entities (random or incremental)
	FReply GenerateVisualMasks();

	// Set unique mask colors in hex for the entities (random generator)
	FReply GenerateVisualMasksRand();

	// Set unique mask colors in hex for the entities (incremental generator)
	FReply GenerateVisualMasksInc();

	// Set flag attribute depending on the checkbox state
	void OnCheckedOverwriteVisualMasks(ECheckBoxState NewCheckedState);

	// Set flag attribute depending on the checkbox state
	void OnCheckedOverwriteGenerateRandomVisualMasks(ECheckBoxState NewCheckedState);

	// Update legacy namings from tags
	FReply UpdateLegacyNames();

	// Remove all tags
	FReply RemoveAllTags();

	// Update semantic visual shape visuals
	FReply UpdateSLContactOverlapShapeColors();

private:
	// Widget pointer
	TSharedPtr<SWidget> ToolkitWidget;

	// If true, overwrite existing semantic map
	bool bOverwriteSemanticMap;

	// If true, overwrite existing class names
	bool bOverwriteExistingClassNames;

	// If true, overwrite existing mask values
	bool bOverwriteVisualMaskValues;

	// If true, generate random colors
	bool bGenerateRandomVisualMasks;

	// Apply changed to selected actors only
	bool bOnlySelected;
};
