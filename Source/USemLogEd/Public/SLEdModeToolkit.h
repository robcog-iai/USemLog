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

	SVerticalBox::FSlot& CreateIdsSlot();
	SVerticalBox::FSlot& CreateClassNamesSlot();
	SVerticalBox::FSlot& CreateWriteVisualMasksSlot();

	SVerticalBox::FSlot& CreateRmAllSlot();
	SVerticalBox::FSlot& CreateAddSemMonSlot();
	SVerticalBox::FSlot& CreateAddSemDataSlot();
	SVerticalBox::FSlot& CreateEnableOverlapsSlot();
	SVerticalBox::FSlot& CreateShowSemData();
	SVerticalBox::FSlot& CreateEnableInstacedMeshMaterialsSlot();

	/* Button callbacks */
	FReply OnGenSemMap();
	FReply OnWriteSemIds();
	FReply OnRmSemIds();
	FReply OnWriteClassNames();
	FReply OnRmClassNames();
	FReply OnWriteVisualMasks();
	FReply OnRmVisualMasks();
	FReply OnRmAll();
	FReply OnAddSemMon();
	FReply OnAddSemData();
	FReply OnEnableOverlaps();
	FReply OnShowSemData();
	FReply OnEnableMaterialsForInstancedStaticMesh();

	/* Checkbox callbacks */
	void OnCheckedOverwrite(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelected(ECheckBoxState NewCheckedState);

	// Return true if any actors are selected in the viewport
	bool AreActorsSelected();

private:
	// Widget pointer
	TSharedPtr<SWidget> ToolkitWidget;

	bool bOverwrite;
	bool bOnlySelected;
};
