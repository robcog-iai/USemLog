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
	SVerticalBox::FSlot& CreateWriteIdsSlot();
	SVerticalBox::FSlot& CreateRmIdsSlot();
	SVerticalBox::FSlot& CreateWriteClassNamesSlot();
	SVerticalBox::FSlot& CreateRmClassNamesSlot();
	SVerticalBox::FSlot& CreateWriteVisualMasksSlot();
	SVerticalBox::FSlot& CreateRmVisualMasksSlot();

	/* Button callbacks */
	FReply OnGenSemMap();
	FReply OnWriteSemIds();
	FReply OnRmSemIds();
	FReply OnWriteClassNames();
	FReply OnRmClassNames();
	FReply OnWriteVisualMasks();
	FReply OnRmVisualMasks();

	/* Checkbox callbacks */
	void OnCheckedOverwrite(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelected(ECheckBoxState NewCheckedState);

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
};
