// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
	// Separator hack slot
	SVerticalBox::FSlot& CreateSeparatorHackSlot();

	// Flag checkboxes
	SVerticalBox::FSlot& CreateCompactCheckBoxSlot();
	SVerticalBox::FSlot& CreateOverwriteFlagSlot();
	SVerticalBox::FSlot& CreateOnlySelectedFlagSlot();
	SVerticalBox::FSlot& CreateResetFlagSlot();
	SVerticalBox::FSlot& CreateTryImportFlagSlot();
	SVerticalBox::FSlot& CreateIncludeChildrenFlagSlot();

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
	SVerticalBox::FSlot& CreateIndividualsInfoTxtSlot();
	SVerticalBox::FSlot& CreateIndividualsInfoSlot();
	SVerticalBox::FSlot& CreateIndividualsInfoFuncSlot();

	// Individual Managers
	SVerticalBox::FSlot& CreateIndividualsManagersTxtSlot();
	SVerticalBox::FSlot& CreateIndividualsManagersSlot();

	// Semantic Map
	SVerticalBox::FSlot& CreateSemMapSlot();

	// Misc
	SVerticalBox::FSlot& CreateUtilsTxtSlot();
	SVerticalBox::FSlot& CreateConvertToVizMapSlot();
	SVerticalBox::FSlot& CreateLogIdsSlot();
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
	void OnCheckedIncludeChildrenFlag(ECheckBoxState NewCheckedState);
	void OnCheckedResetFlag(ECheckBoxState NewCheckedState);
	void OnCheckedTryImportFlag(ECheckBoxState NewCheckedState);


	// Individual Components
	FReply OnCreateIndividuals();
	FReply OnClearIndividuals();
	FReply OnInitIndividuals();
	FReply OnLoadIndividuals();
	FReply OnConnectIndividuals();

	// Individual Components Funcs
	FReply OnToggleIndividualVisualMaskVisiblity();

	// Individual Values
	FReply OnWriteAllIndvidualValues();
	FReply OnClearAllIndividualValues();
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

	// Individual Info 
	FReply OnCreateIndividualsInfo();
	FReply OnClearIndividualsInfo();
	FReply OnInitIndividualsInfo();
	FReply OnLoadIndividualsInfo();
	FReply OnConnectIndividualsInfo();

	// Individual Info Funcs
	FReply OnToggleIndividualsInfoVisiblity();
	FReply OnUpdateIndividualsInfoOrientation();
	FReply OnToggleIndividualsInfoLiveOrientationUpdate();
		
	// Individual Managers
	FReply OnInitIndividualManagers();
	FReply OnLoadIndividualManagers();
	FReply OnConnectIndividualManagers();

	// Semantic map
	FReply OnWriteSemMap();
	FReply OnWriteOntology();


	FReply OnConvertToViz();
	FReply OnLogIds();
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
	bool SetIndividualInfoManager();

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
	class ASLIndividualInfoManager* IndividualInfoManager;

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
