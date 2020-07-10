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
	/* **Start** Vertical Slot Entries */
	// Flag checkboxes
	SVerticalBox::FSlot& CreateOverwriteFlagSlot();
	SVerticalBox::FSlot& CreateOnlySelectedFlagSlot();
	SVerticalBox::FSlot& CreateIncludeChildrenFlagSlot();
	SVerticalBox::FSlot& CreateResetFlagSlot();
	SVerticalBox::FSlot& CreateTryImportFlagSlot();

	// Individual Managers
	SVerticalBox::FSlot& CreateIndividualsManagersTxtSlot();
	SVerticalBox::FSlot& CreateIndividualsManagersSlot();

	// Individual Components
	SVerticalBox::FSlot& CreateIndividualsTxtSlot();
	SVerticalBox::FSlot& CreateIndividualsSlot();
	SVerticalBox::FSlot& CreateIndividualsFuncSlot();

	// Individual Values
	SVerticalBox::FSlot& CreateIndividualValuesTxtSlot();
	SVerticalBox::FSlot& CreateIndividualValuesAllSlot();
	SVerticalBox::FSlot& CreateIndivualValuesIdSlot();
	SVerticalBox::FSlot& CreateIndividualValuesClassSlot();
	SVerticalBox::FSlot& CreateIndividualValuesVisualMaskSlot();
	
	// Import / Export
	SVerticalBox::FSlot& CreateImportExportTxtSlot();
	SVerticalBox::FSlot& CreateImportExportSlot();

	// Individual Visual Info
	SVerticalBox::FSlot& CreateSemDataVisInfoTxtSlot();
	SVerticalBox::FSlot& CreateSemDataVisInfoSlot();
	SVerticalBox::FSlot& CreateSemDataVisInfoFuncSlot();

	// Semantic Map
	SVerticalBox::FSlot& CreateSemMapSlot();

	// Misc
	SVerticalBox::FSlot& CreateUtilsTxtSlot();
	SVerticalBox::FSlot& CreateAddSemMonitorsSlot();	
	SVerticalBox::FSlot& CreateEnableOverlapsSlot();
	SVerticalBox::FSlot& CreateShowSemData();
	SVerticalBox::FSlot& CreateEnableInstacedMeshMaterialsSlot();
	SVerticalBox::FSlot& CreateTriggerGCSlot();

	// Info
	SVerticalBox::FSlot& CreateGenericButtonSlot();
	/* **End** Vertical Slot Entries */


	/* **Start** Callbacks */
	// Flag checkboxes
	void OnCheckedOverwriteFlag(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelectedFlag(ECheckBoxState NewCheckedState);
	void OnCheckedPrioritizeChildrenFlag(ECheckBoxState NewCheckedState);
	void OnCheckedResetFlag(ECheckBoxState NewCheckedState);
	void OnCheckedTryImportFlag(ECheckBoxState NewCheckedState);

	// Individual Managers
	FReply OnInitIndividualManagers();
	FReply OnReloadSemDataManagers();//todo

	// Individual Components
	FReply OnCreateIndividuals();
	FReply OnDestroyIndividuals();
	FReply OnInitIndividuals();
	FReply OnLoadIndividuals();
	FReply OnToggleIndividualVisualMaskVisiblity();

	// Individual Values
	FReply OnWriteSemDataAll();
	FReply OnRmSemDataAll();
	FReply OnWriteIndividualIds();
	FReply OnClearIndividualIds();
	FReply OnWriteIndividualClasses();
	FReply OnClearIndividualClasses();
	FReply OnWriteIndividualVisualMasks();
	FReply OnClearIndividualVisualMasks();

	// Import / Export
	FReply OnExportValues();
	FReply OnImportValues();
	FReply OnClearExportedValues();

	// Individual Visual Info
	FReply OnCreateIndividualsInfo();
	FReply OnResetIndividualsInfo();
	FReply OnRemoveIndividualsInfo();
	FReply OnToggleIndividualsInfoVisiblity();
	FReply OnUpdateIndividualsInfoOrientation();
	FReply OnToggleIndividualsInfoLiveOrientationUpdate();



	

	// Semantic map
	FReply OnWriteSemMap();

	FReply OnAddSemMon();	
	FReply OnEnableOverlaps();
	FReply OnShowSemData();
	FReply OnEnableMaterialsForInstancedStaticMesh();

	FReply OnTriggerGC();

	FReply OnGenericButton();
	/* **End** Callbacks */


	/* Helpers */
	// Managers
	bool HasValidIndividualManager() const;
	bool SetIndividualManager();
	bool HasValidIndividualInfoManager() const;
	bool SetdIndividualInfoManager();

	// Returns all the selected actors in the editor
	TArray<AActor*> GetSelectedActors() const;

	// Returns the actor that is selected, nullptr if no selection or multiple selections
	AActor* GetSingleSelectedActor() const;

	// Deselect components to avoid crash when deleting the sl data component
	void DeselectComponentSelection() const;

	/* Info */
	// Print out info about uobjects in editor
	void LogObjectInfo(UWorld* World) const;

private:
	// Provides functionalities over the individuals in the world
	class ASLIndividualManager* IndividualManager;

	// Provides visual functionalities over the individuals in the world
	class ASLIndividualVisualManager* IndividualInfoManager;

	// Widget pointer
	TSharedPtr<SWidget> ToolkitWidget;

	/** SCS editor, refresh view if an actor is selected when adding/removing components */
	TSharedPtr<class SSCSEditor> SCSEditor;

	// Flags
	bool bOverwriteFlag;
	bool bOnlySelectedFlag;
	bool bProritizeChildrenFlag;
	bool bResetFlag;
	bool bTryImportFlag;
};
