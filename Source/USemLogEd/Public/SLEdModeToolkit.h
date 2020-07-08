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
	SVerticalBox::FSlot& CreateOverwriteFlagSlot();
	SVerticalBox::FSlot& CreateOnlySelectedFlagSlot();
	SVerticalBox::FSlot& CreateIncludeChildrenFlagSlot();
	SVerticalBox::FSlot& CreateResetFlagSlot();
	SVerticalBox::FSlot& CreateTryImportFlagSlot();

	// Semantic map
	SVerticalBox::FSlot& CreateSemMapSlot();

	// Semantic data managers
	SVerticalBox::FSlot& CreateSemDataManagersTxtSlot();
	SVerticalBox::FSlot& CreateSemDataManagersSlot();

	// Semantic data components
	SVerticalBox::FSlot& CreateIndividualsTxtSlot();
	SVerticalBox::FSlot& CreateIndividualsSlot();
	SVerticalBox::FSlot& CreateIndividualsFuncSlot();

	// Semantic data visual info
	SVerticalBox::FSlot& CreateSemDataVisInfoTxtSlot();
	SVerticalBox::FSlot& CreateSemDataVisInfoSlot();
	SVerticalBox::FSlot& CreateSemDataVisInfoFuncSlot();

	// Semantic data
	SVerticalBox::FSlot& CreateSemDataTxtSlot();
	SVerticalBox::FSlot& CreateSemDataAllSlot();
	SVerticalBox::FSlot& CreateSemDataIdSlot();
	SVerticalBox::FSlot& CreateSemDataClassSlot();
	SVerticalBox::FSlot& CreateSemDataMaskSlot();
	
	// Tag import export 
	SVerticalBox::FSlot& CreateTagTxtSlot();
	SVerticalBox::FSlot& CreateTagDataSlot();

	// Misc
	SVerticalBox::FSlot& CreateUtilsTxtSlot();
	SVerticalBox::FSlot& CreateAddSemMonitorsSlot();	
	SVerticalBox::FSlot& CreateEnableOverlapsSlot();
	SVerticalBox::FSlot& CreateShowSemData();
	SVerticalBox::FSlot& CreateEnableInstacedMeshMaterialsSlot();
	SVerticalBox::FSlot& CreateTriggerGCSlot();

	// Info
	SVerticalBox::FSlot& CreateGenericButtonSlot();


	/* Checkbox callbacks */
	void OnCheckedOverwriteFlag(ECheckBoxState NewCheckedState);
	void OnCheckedOnlySelectedFlag(ECheckBoxState NewCheckedState);
	void OnCheckedPrioritizeChildrenFlag(ECheckBoxState NewCheckedState);
	void OnCheckedResetFlag(ECheckBoxState NewCheckedState);
	void OnCheckedTryImportFlag(ECheckBoxState NewCheckedState);


	/* Button callbacks */
	
	// Semantic map
	FReply OnWriteSemMap();

	// Semantic data managers
	FReply OnInitSemDataManagers();
	FReply OnReloadSemDataManagers();

	// Semantic data components
	FReply OnCreateIndividuals();
	FReply OnRemoveIndividuals();
	FReply OnInitIndividuals();
	FReply OnLoadIndividuals();
	FReply OnToggleIndividualVisualMaskVisiblity();

	// Semantic data visual info
	FReply OnCreateIndividualsInfo();
	FReply OnResetIndividualsInfo();
	FReply OnRemoveIndividualsInfo();
	FReply OnToggleIndividualsInfoVisiblity();
	FReply OnUpdateIndividualsInfoOrientation();
	FReply OnToggleIndividualsInfoLiveOrientationUpdate();

	// Semantic data
	FReply OnWriteSemDataAll();
	FReply OnRmSemDataAll();
	FReply OnWriteSemDataIds();
	FReply OnRmSemDataIds();
	FReply OnWriteClassNames();
	FReply OnRmClassNames();

	FReply OnWriteVisualMasks();
	FReply OnRmVisualMasks();
	
	FReply OnExportData();
	FReply OnImportData();
	FReply OnClearData();

	FReply OnAddSemMon();	
	FReply OnEnableOverlaps();
	FReply OnShowSemData();
	FReply OnEnableMaterialsForInstancedStaticMesh();

	FReply OnTriggerGC();

	FReply OnGenericButton();


	/* Managers */
	bool HasValidIndividualManager() const;
	bool SetIndividualManager();

	bool HasValidIndividualInfoManager() const;
	bool SetdIndividualInfoManager();

	/* Helper */
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

	bool bOverwriteFlag;
	bool bOnlySelectedFlag;
	bool bProritizeChildrenFlag;
	bool bResetFlag;
	bool bTryImportFlag;
};
