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

	SVerticalBox::FSlot& CreateSemMapSlot();

	SVerticalBox::FSlot& CreateSemDataCompTxtSlot();
	SVerticalBox::FSlot& CreateSemDataCompSlot();

	SVerticalBox::FSlot& CreateSemDataTxtSlot();
	SVerticalBox::FSlot& CreateSemDataAllSlot();
	SVerticalBox::FSlot& CreateSemDataIdSlot();
	SVerticalBox::FSlot& CreateSemDataClassSlot();
	SVerticalBox::FSlot& CreateSemDataMaskSlot();

	SVerticalBox::FSlot& CreateTagTxtSlot();
	SVerticalBox::FSlot& CreateTagDataSlot();

	SVerticalBox::FSlot& CreateUtilsTxtSlot();
	SVerticalBox::FSlot& CreateAddSemMonitorsSlot();	
	SVerticalBox::FSlot& CreateEnableOverlapsSlot();
	SVerticalBox::FSlot& CreateShowSemData();
	SVerticalBox::FSlot& CreateEnableInstacedMeshMaterialsSlot();

	SVerticalBox::FSlot& CreateGenericButtonSlot();


	/* Checkbox callbacks */
	void OnCheckedOverwrite(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelected(ECheckBoxState NewCheckedState);

	/* Button callbacks */
	FReply OnWriteSemMap();

	FReply OnCreateSemDataComp();
	FReply OnRefreshSemDataComp();
	FReply OnRmSemDataComp();


	FReply OnWriteSemDataAll();
	FReply OnRmSemDataAll();
	FReply OnWriteSemDataIds();
	FReply OnRmSemDataIds();
	FReply OnWriteClassNames();
	FReply OnRmClassNames();

	FReply OnWriteVisualMasks();
	FReply OnRmVisualMasks();
	
	FReply OnSaveTagData();
	FReply OnLoadTagData();
	FReply OnClearTagData();

	FReply OnAddSemMon();	
	FReply OnEnableOverlaps();
	FReply OnShowSemData();
	FReply OnEnableMaterialsForInstancedStaticMesh();

	FReply OnGenericButton();

	/* Helper */
	// Returns all the selected actors in the editor
	TArray<AActor*> GetSelectedActors() const;

	// Returns the actor that is selected, nullptr if no selection or multiple selections
	AActor* GetSingleSelectedActor() const;

	// Deselect components to avoid crash when deleting the sl data component
	void DeselectComponentsOnly() const;

private:
	// Widget pointer
	TSharedPtr<SWidget> ToolkitWidget;

	/** SCS editor, refresh view if an actor is selected when adding/removing components */
	TSharedPtr<class SSCSEditor> SCSEditor;

	bool bOverwrite;
	bool bOnlySelected;
	bool bUseTags;
};
