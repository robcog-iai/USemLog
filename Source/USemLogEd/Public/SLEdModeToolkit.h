// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "Widgets/SBoxPanel.h"


/**
 * SL editor mode toolkit
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
	/* Vertical slot entries */
	// Create checkbox entry slot to set overwrite flag
	SVerticalBox::FSlot& CreateOverwriteSlot();
	SVerticalBox::FSlot& CreateOnlySelectedSlot();
	SVerticalBox::FSlot& CreateSemDataComponentsSlot();
	SVerticalBox::FSlot& CreateGenSemMapSlot();
	SVerticalBox::FSlot& CreateIdsSlot();
	SVerticalBox::FSlot& CreateClassNamesSlot();
	SVerticalBox::FSlot& CreateVisualMasksSlot();
	SVerticalBox::FSlot& CreateRmAllSlot();
	SVerticalBox::FSlot& CreateAddSemMonSlot();	
	SVerticalBox::FSlot& CreateEnableOverlapsSlot();
	SVerticalBox::FSlot& CreateShowSemData();
	SVerticalBox::FSlot& CreateEnableInstacedMeshMaterialsSlot();

	/* Button callbacks */
	FReply OnGenSemMap();
	FReply OnAddSemDataComp();
	FReply OnRmSemDataComp();
	FReply OnSaveSemDataComp();
	FReply OnLoadSemDataComp();
	FReply OnWriteSemIds();
	FReply OnRmSemIds();
	FReply OnWriteClassNames();
	FReply OnRmClassNames();
	FReply OnWriteVisualMasks();
	FReply OnRmVisualMasks();
	FReply OnRmAll();
	FReply OnAddSemMon();	
	FReply OnEnableOverlaps();
	FReply OnShowSemData();
	FReply OnEnableMaterialsForInstancedStaticMesh();

	/* Checkbox callbacks */
	void OnCheckedOverwrite(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelected(ECheckBoxState NewCheckedState);

	/* Helper */
	TArray<AActor*> GetSelectedActors() const;

private:
	// Widget pointer
	TSharedPtr<SWidget> ToolkitWidget;

	bool bOverwrite;
	bool bOnlySelected;
	bool bUseTags;
};
