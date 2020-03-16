// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "Widgets/SBoxPanel.h"


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
	/* Create vertical slot entries */	
	SVerticalBox::FSlot& CreateOverwriteSlot();
	SVerticalBox::FSlot& CreateOnlySelectedSlot();
	SVerticalBox::FSlot& CreateGenSemMapSlot();
	SVerticalBox::FSlot& CreateGenIdsSlot();
	SVerticalBox::FSlot& CreateRmIdsSlot();
	SVerticalBox::FSlot& CreateWriteClassNamesSlot();

	/* Button callbacks */
	FReply OnGenSemMap();
	FReply OnGenSemIds();
	FReply OnRmSemIds();
	FReply OnWriteClassNames();

	/* Checkbox callbacks */
	void OnCheckedOverwrite(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelected(ECheckBoxState NewCheckedState);




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


	void OnCheckedOverwriteVisualMasks(ECheckBoxState NewCheckedState);

	// Set flag attribute depending on the checkbox state
	void OnCheckedOverwriteGenerateRandomVisualMasks(ECheckBoxState NewCheckedState);

	// Update legacy namings from tags
	FReply UpdateLegacyNames();

	// Remove all tags
	FReply RemoveAllTags();

	// Add contact overlap shapes
	FReply AddSLContactBoxes();

	// Update semantic visual shape visuals
	FReply UpdateSLContactBoxColors();

	// Enable all overlaps
	FReply EnableAllOverlaps();

	// Tag selected meshes as containers
	FReply TagSelectedAsContainers();

	// Iterate all materials and enable for instanced static mesh
	FReply EnableMaterialsForInstancedStaticMesh();

	// Generate semantic components for each actor
	FReply GenerateSemanticComponents();

	// Show semantic data in the editor
	FReply ShowSemanticData();

	// Return true if any actors are selected in the viewport
	bool AreActorsSelected();


	/* Checkbox callbacks */


private:
	// Widget pointer
	TSharedPtr<SWidget> ToolkitWidget;

	/* Checkbox states */

	bool bOverwrite;
	bool bOnlySelected;

	bool bOverwriteExistingClassNames;
	bool bOverwriteVisualMaskValues;
	bool bGenerateRandomVisualMasks;
};
