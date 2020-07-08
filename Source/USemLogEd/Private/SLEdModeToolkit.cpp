// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdModeToolkit.h"
#include "SLEdMode.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"

#include "Engine/Selection.h"
#include "ScopedTransaction.h"
#include "UnrealEdGlobals.h" // for GUnrealEd
#include "Editor/UnrealEdEngine.h"


#include "EngineUtils.h"

// SL
#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualVisualManager.h"
#include "Individuals/SLIndividualUtils.h"

// UUtils
#include "SLEdUtils.h"

#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

// Ctor
FSLEdModeToolkit::FSLEdModeToolkit()
{
	IndividualManager = nullptr;
	IndividualInfoManager = nullptr;
	/* Checkbox states */
	bOverwriteFlag = false;
	bOnlySelectedFlag = false;
	bProritizeChildrenFlag = false;	
	bResetFlag = false;
	bTryImportFlag = true;
}

// Create the widget, bind the button callbacks
void FSLEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(15)
		[
			SNew(SVerticalBox)

			////
			+ CreateOverwriteFlagSlot()
			+ CreateOnlySelectedFlagSlot()
			+ CreateIncludeChildrenFlagSlot()
			+ CreateResetFlagSlot()
			+ CreateTryImportFlagSlot()

			////
			+ CreateSemMapSlot()

			////
			+ CreateSemDataManagersTxtSlot()
			+ CreateSemDataManagersSlot()

			////
			+ CreateIndividualsTxtSlot()
			+ CreateIndividualsSlot()
			+ CreateIndividualsFuncSlot()

			////
			+ CreateSemDataVisInfoTxtSlot()
			+ CreateSemDataVisInfoSlot()
			+ CreateSemDataVisInfoFuncSlot()

			////
			+ CreateSemDataTxtSlot()
			+ CreateSemDataAllSlot()
			+ CreateSemDataIdSlot()
			+ CreateSemDataClassSlot()
			+ CreateSemDataMaskSlot()

			////
			+ CreateTagTxtSlot()
			+ CreateTagDataSlot()

			////
			+ CreateUtilsTxtSlot()

			////
			+ CreateAddSemMonitorsSlot()

			////
			+ CreateEnableOverlapsSlot()

			////
			+ CreateShowSemData()

			////
			+ CreateEnableInstacedMeshMaterialsSlot()

			////
			+ CreateTriggerGCSlot()

			////
			+ CreateGenericButtonSlot()
		];

	FModeToolkit::Init(InitToolkitHost);
}


/** IToolkit interface */
FName FSLEdModeToolkit::GetToolkitFName() const
{
	return FName("Semantic Logger Editor Mode");
}

FText FSLEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("SemLogEdModeToolkit", "DisplayName", "Semantic Logger Editor Mode Toolkit");
}

class FEdMode* FSLEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FSLEdMode::EM_SLEdModeId);
}


/* Vertical slot entries */

/* Checkboxes */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateOverwriteFlagSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OverwriteTextLabel", "Global Overwrite Flag: "))
			]

				+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckBoxOverwrite", "Overwrites any existing data, use with caution"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteFlag)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateOnlySelectedFlagSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OnlySelectedTextLabel", "Only Selection Flag:  "))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckBoxOnlySelected", "Consider only selected actors"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOnlySelectedFlag)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIncludeChildrenFlagSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("IncludeChildrenTextLabel", "Prioritize Children Flag:  "))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckBoxIncludeChildren", "Prioritize children data (e.g. bones/links..)"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedPrioritizeChildrenFlag)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateResetFlagSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ResetFlagTextLabel", "Reset Flag:  "))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckboxResetFlag", "Apply reset flag to functions"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedResetFlag)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateTryImportFlagSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TryImportTextLabel", "Try Import Flag:  "))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckboxTryImport", "If available data will be imported first.."))
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedTryImportFlag)
			]
		];
}


/* Semantic map */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemMapSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SemMapTxt", "Semantic Map:  "))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemMapGen", "Generate"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemMapGenTip", "Exports the generated semantic map to an owl file"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteSemMap)
			]
		];
}


/* Semantic data managers */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataManagersTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IndividualManagersTxt", "Managers:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataManagersSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualManagersInit", "Init"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualManagersInitTip", "Loads (or creates) managers from (or to) the world, and initializes them.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitSemDataManagers)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualManagersReload", "Reload"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualManagersReloadTip", "Reloads the components from the world (clean + init).."))
				.OnClicked(this, &FSLEdModeToolkit::OnReloadSemDataManagers)
			]
		];
}


/* Individual components */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IndividualsTxt", "Individuals:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsCreate", "Create"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsCreateTip", "Create individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnCreateIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsRemove", "Remove"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsRmTip", "Remove individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnRemoveIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsInit", "Init"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsInitTip", "Call Init(bReset) on individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsLoad", "Load"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsLoadTip", "Call Load(bReset, bTryImport) on individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitIndividuals)
			]


		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsFuncSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsToggleVisualMaskVisiblitiy", "Toggle Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsToggleMaskTip", "Toggle between the visual mask and the original colors.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleIndividualVisualMaskVisiblity)
			]

		];
}


/* Semantic data visual info components */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataVisInfoTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SemDataVisInfoTxt", "Visual Components:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataVisInfoSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoCreate", "Create"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoCreateTip", "Create visual info components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnCreateIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoRefresh", "Reset"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoRefreshTip", "Force init + load .."))
				.OnClicked(this, &FSLEdModeToolkit::OnResetIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoRm", "Remove"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoRmTip", "Remove visual info components (make sure no related editor windows are open).."))
				.OnClicked(this, &FSLEdModeToolkit::OnRemoveIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoToggle", "Toggle Visibility"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoToggleTip", "Toggle visual info visibility.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleIndividualsInfoVisiblity)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataVisInfoFuncSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoUpdate", "Update"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoUpdateTip", "Point text towards camera.."))
				.OnClicked(this, &FSLEdModeToolkit::OnUpdateIndividualsInfoOrientation)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoLIveUpdate", "ToggleDynamicUpdate"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoLIveUpdateTip", "Toggle live update.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleIndividualsInfoLiveOrientationUpdate)
			]

		];
}


/* Semantic data */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SemDataTxt", "Semantic Data:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataAllSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("WriteAll", "Write All"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("WriteAllTip", "Generates all data"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteSemDataAll)
			]

		+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RmAll", "Remove All"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmAllTip", "Removes all generated data"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmSemDataAll)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataIdSlot()
{
	return 	SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("WriteIds", "Write Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("WriteIdsTip", "Generates unique ids for every semantic entity"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteSemDataIds)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RmIds", "Remove Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmIdsTip", "Removes all generated ids"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmSemDataIds)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataClassSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("WriteClassNames", "Write Class Names"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("WriteClassNames", "Writes known class names"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteClassNames)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RmClassNames", "Remove Class Names"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmClassNamesTip", "Removes all class names"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmClassNames)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataMaskSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("WriteVisualMasks", "Write Visual Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("WriteVisualMasksTip", "Writes unique visual masks for visual entities"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteVisualMasks)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RmVisualMasks", "Remove Visual Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmVisualMasksTip", "Removes all visual masks"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmVisualMasks)
			]
		];
}


/* Tag */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateTagTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TagTxt", "Tags:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateTagDataSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsSave", "Export"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsSaveTip", "Save data to tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnExportData)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("IndividualsLoad", "Import"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("IndividualsLoadTip", "Load data from tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnImportData)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RemoveTagData", "Clear"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RemoveTagDataTip", "Removes data stored in tags.."))
				.OnClicked(this, &FSLEdModeToolkit::OnClearData)
			]
		];
}


/* Misc */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateUtilsTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("UtilsTxt", "Utils:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateAddSemMonitorsSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("AddSemMon", "Add Semantic Monitors"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("AddSemMonTip", "Creates or updates semantic monitor components.."))
			.OnClicked(this, &FSLEdModeToolkit::OnAddSemMon)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateEnableOverlapsSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("EnableOverlaps", "Enable Overlaps"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("EnableOverlapsTip", "Enables overlap events on all actors.."))
			.OnClicked(this, &FSLEdModeToolkit::OnEnableOverlaps)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateShowSemData()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("ShowSemData", "Show Semantic Data"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("ShowSemDataTip", "Visualize semantic data.."))
			.OnClicked(this, &FSLEdModeToolkit::OnShowSemData)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateEnableInstacedMeshMaterialsSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("EnableMaterialsForInstancedStaticMesh", "Enable Materials for Instanced Static Mesh"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("EnableMaterialsForInstancedStaticMeshTip", "Make sure every material asset can be rendered as an instanced static mesh.."))
			.OnClicked(this, &FSLEdModeToolkit::OnEnableMaterialsForInstancedStaticMesh)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateTriggerGCSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("TriggerGC", "GC"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("TriggerGCTip", "This will triger the garbage collection"))
			.OnClicked(this, &FSLEdModeToolkit::OnTriggerGC)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateGenericButtonSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("GenericButton", "Generic Button"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("GenericButtonTip", "Test various things.."))
			.OnClicked(this, &FSLEdModeToolkit::OnGenericButton)
		];
}


/* Checkbox callbacks */
void FSLEdModeToolkit::OnCheckedOverwriteFlag(ECheckBoxState NewCheckedState)
{
	bOverwriteFlag = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedOnlySelectedFlag(ECheckBoxState NewCheckedState)
{
	bOnlySelectedFlag = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedPrioritizeChildrenFlag(ECheckBoxState NewCheckedState)
{
	bProritizeChildrenFlag = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedResetFlag(ECheckBoxState NewCheckedState)
{
	bResetFlag = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedTryImportFlag(ECheckBoxState NewCheckedState)
{
	bTryImportFlag = (NewCheckedState == ECheckBoxState::Checked);
}


/* Button callbacks */
////
FReply FSLEdModeToolkit::OnWriteSemMap()
{
	FSLEdUtils::WriteSemanticMap(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	return FReply::Handled();
}

//// Managers
FReply FSLEdModeToolkit::OnInitSemDataManagers()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataManagerInit", "Init semantic data managers"));

	int32 NumIndividualComponents = 0;
	if (HasValidIndividualManager() || SetIndividualManager())
	{
		NumIndividualComponents = IndividualManager->Init();
	}

	UE_LOG(LogTemp, Log, TEXT("%s::%d Loaded %ld individual components.."),
		*FString(__FUNCTION__), __LINE__, NumIndividualComponents);


	if (HasValidIndividualInfoManager())
	{
		IndividualInfoManager->Init();
	}
	else
	{
		IndividualInfoManager = FSLEdUtils::GetOrCreateNewVisualInfoManager(GEditor->GetEditorWorldContext().World());
		if (HasValidIndividualInfoManager())
		{
			IndividualInfoManager->Init();
		}
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnReloadSemDataManagers()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataManagerReload", "Reload semantic data managers"));
	const bool bReset = true;

	if (HasValidIndividualManager())
	{
		IndividualManager->Reload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (HasValidIndividualInfoManager())
	{
		IndividualInfoManager->Init(bReset);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual visual manager not set, init first.."), *FString(__FUNCTION__), __LINE__);
	}

	return FReply::Handled();
}

//// Individual Components
FReply FSLEdModeToolkit::OnCreateIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsCreateST", "Create individual components"));
	int32 NumComp = 0;

	if (bOnlySelectedFlag)
	{
		NumComp = FSLIndividualUtils::CreateIndividualComponents(GetSelectedActors());
	}
	else
	{
		NumComp = FSLIndividualUtils::CreateIndividualComponents(GEditor->GetEditorWorldContext().World());
	}

	if (HasValidIndividualManager())
	{
		// TODO reload individuals
	}

	//if (HasValidIndividualManager())
	//{
	//	if (bOnlySelectedFlag)
	//	{
	//		NumComp = IndividualManager->AddIndividualComponents(GetSelectedActors());
	//	}
	//	else
	//	{
	//		NumComp = IndividualManager->AddIndividualComponents();
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
	//		*FString(__FUNCTION__), __LINE__);
	//}

	if (NumComp)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Created %ld new individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRemoveIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsRmST", "Remove semantic data components"));

	DeselectComponentSelection();

	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->DestroyIndividualComponents(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualManager->DestroyIndividualComponents();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Removed %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnInitIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsLoadST", "Reload semantic data components"));
	int32 NumComp = 0;

	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->ResetIndividualComponents(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualManager->ResetIndividualComponents();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Init(bReset=%d) on %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp, bResetFlag);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsLoadST", "Reload semantic data components"));
	int32 NumComp = 0;

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Load(bReset=%d, bTryImport=%d) on  %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp, bResetFlag, bTryImportFlag);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleIndividualVisualMaskVisiblity()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsToggleMaskST", "Toggle individuals visual maks visiblity"));
	
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->ToggleVisualMaskVisibility(GetSelectedActors(), bProritizeChildrenFlag);
		}
		else
		{
			NumComp = IndividualManager->ToggleVisualMaskVisibility(bProritizeChildrenFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Mask visibility toggled for %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}


//// Individual Info Components
FReply FSLEdModeToolkit::OnCreateIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoCreateST", "Create individuals info.."));
	int32 NumComp = 0;

	if (HasValidIndividualInfoManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualInfoManager->AddIndividualInfoComponents(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualInfoManager->AddIndividualInfoComponents();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Created %ld new visual info components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnResetIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoRefreshST", "Reset individuals info components.."));
	int32 NumComp = 0;

	if (HasValidIndividualInfoManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualInfoManager->ResetIndividualInfoComponents(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualInfoManager->ResetIndividualInfoComponents();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Refreshed %ld new visual info components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRemoveIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoRmST", "Remove individual info components"));
	
	DeselectComponentSelection();

	int32 NumComp = 0;

	if (HasValidIndividualInfoManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualInfoManager->DestroyIndividualInfoComponents(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualInfoManager->DestroyIndividualInfoComponents();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Refreshed %ld new visual info components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleIndividualsInfoVisiblity()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoToggleST", "Toggle visual info components visibility"));
	int32 NumComp = 0;

	if (HasValidIndividualInfoManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualInfoManager->ToggleInfoVisibility(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualInfoManager->ToggleInfoVisibility();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Toggled %ld new visual info components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnUpdateIndividualsInfoOrientation()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoUpdateST", "Update visual info orientation"));
	int32 NumComp = 0;

	if (HasValidIndividualInfoManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualInfoManager->LookAtCamera(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualInfoManager->LookAtCamera();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Toggled %ld new visual info components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleIndividualsInfoLiveOrientationUpdate()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoLiveUpdateST", "Toggle live visual info orientation"));

	if (HasValidIndividualInfoManager())
	{
		IndividualInfoManager->ToggleTickUpdate();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}
	
	return FReply::Handled();
}


////
FReply FSLEdModeToolkit::OnWriteSemDataAll()
{
	FScopedTransaction Transaction(LOCTEXT("WriteAllSemDataST", "Write all semantic data"));
	OnWriteSemDataIds();
	OnWriteClassNames();
	OnWriteVisualMasks();
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataAll()
{
	FScopedTransaction Transaction(LOCTEXT("RmAllSemDataST", "Remove all semantic data"));
	OnRmSemDataIds();
	OnRmClassNames();
	OnRmVisualMasks();
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteSemDataIds()
{
	FScopedTransaction Transaction(LOCTEXT("GenSemIdsST", "Generate new semantic Ids"));	
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->WriteUniqueIds(GetSelectedActors(), bOverwriteFlag);
		}
		else
		{
			NumComp = IndividualManager->WriteUniqueIds(bOverwriteFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		//if (VisualInfoManager && VisualInfoManager->IsValidLowLevel() && !VisualInfoManager->IsPendingKill())
		//{
		//	bOnlySelected ? VisualInfoManager->ReloadVisualInfoComponents(GetSelectedActors()) : VisualInfoManager->ReloadVisualInfoComponents();
		//}
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Generated new ids for %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataIds()
{
	FScopedTransaction Transaction(LOCTEXT("RmSemIdsST", "Remove all semantic Ids"));
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->RemoveUniqueIds(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualManager->RemoveUniqueIds();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Removed ids from %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("WriteClassNamesST", "Write class names"));
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->WriteClassNames(GetSelectedActors(), bOverwriteFlag);
		}
		else
		{
			NumComp = IndividualManager->WriteClassNames(bOverwriteFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		//if (VisualInfoManager && VisualInfoManager->IsValidLowLevel() && !VisualInfoManager->IsPendingKill())
		//{
		//	bOnlySelected ? VisualInfoManager->ReloadVisualInfoComponents(GetSelectedActors()) : VisualInfoManager->ReloadVisualInfoComponents();
		//}
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Wrote classes for %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("RmClassNamesST", "Remove all class names"));
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->RemoveClassNames(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualManager->RemoveClassNames();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Removed classes from %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}
	
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("WriteVisualMasksST", "Write visual masks"));	
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->WriteVisualMasks(GetSelectedActors(), bOverwriteFlag);
		}
		else
		{
			NumComp = IndividualManager->WriteVisualMasks(bOverwriteFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Wrote visual masks for %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("RmVisualMasksST", "Remove all visual masks names"));
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->RemoveVisualMasks(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualManager->RemoveVisualMasks();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Removed visual masks from %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}
	
	return FReply::Handled();
}


////
FReply FSLEdModeToolkit::OnExportData()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsSaveST", "Export semantic data to tag"));
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->ExportValues(GetSelectedActors(), bOverwriteFlag);
		}
		else
		{
			NumComp = IndividualManager->ExportValues(bOverwriteFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Exported %ld individual components to tag.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnImportData()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsLoadST", "Import data from tag"));
	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->ImportValues(GetSelectedActors(), bOverwriteFlag);
		}
		else
		{
			NumComp = IndividualManager->ImportValues(bOverwriteFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Exported %ld individual components to tag.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearData()
{
	FScopedTransaction Transaction(LOCTEXT("RmTagData", "Remove all SemLog tags"));
	//bool bMarkDirty = false;

	//if (bOnlySelected)
	//{
	//	bMarkDirty = FSLEdUtils::RemoveTagType(GetSelectedActors(), "SemLog");
	//}
	//else
	//{
	//	bMarkDirty = FSLEdUtils::RemoveTagType(GEditor->GetEditorWorldContext().World(), "SemLog");
	//}

	//if (bMarkDirty)
	//{
	//	GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	//}

	int32 NumComp = 0;
	if (HasValidIndividualManager())
	{
		if (bOnlySelectedFlag)
		{
			NumComp = IndividualManager->ClearExportedValues(GetSelectedActors());
		}
		else
		{
			NumComp = IndividualManager->ClearExportedValues();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, init first.."),
			*FString(__FUNCTION__), __LINE__);
	}

	if (NumComp)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Exported %ld individual components to tag.."),
			*FString(__FUNCTION__), __LINE__, NumComp);
	}

	return FReply::Handled();
}


////
FReply FSLEdModeToolkit::OnAddSemMon()
{
	FScopedTransaction Transaction(LOCTEXT("AddSemMonitorsST", "Add semantic monitor components"));
	bool bMarkDirty = false;

	if (bOnlySelectedFlag)
	{
		bMarkDirty = FSLEdUtils::AddSemanticMonitorComponents(GetSelectedActors(), bOverwriteFlag);
	}
	else
	{
		bMarkDirty = FSLEdUtils::AddSemanticMonitorComponents(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnEnableOverlaps()
{
	FScopedTransaction Transaction(LOCTEXT("EnableOverlapsST", "Enable overlaps"));
	bool bMarkDirty = false;

	if (bOnlySelectedFlag)
	{
		bMarkDirty = FSLEdUtils::EnableOverlaps(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::EnableOverlaps(GEditor->GetEditorWorldContext().World());
	}
	
	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnShowSemData()
{
	FScopedTransaction Transaction(LOCTEXT("ShowSemDataST", "Show semantic data"));
	bool bMarkDirty = false;

	if (bOnlySelectedFlag)
	{
		//FSLEdUtils::ShowSemanticData(GetSelectedActors());
	}
	else
	{
		//FSLEdUtils::ShowSemanticData(GEditor->GetEditorWorldContext().World());
	}
	
	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnEnableMaterialsForInstancedStaticMesh()
{
	FScopedTransaction Transaction(LOCTEXT("AllMatForInstancedStaticMeshST", "Enable all materials for instanced static mesh rendering"));
	FSLEdUtils::EnableAllMaterialsForInstancedStaticMesh();
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnTriggerGC()
{
	//FScopedTransaction Transaction(LOCTEXT("TriggerGCST", "TriggerGC"));
	//if (GEngine)
	//{		
	//	//GEngine->ForceGarbageCollection();
	//	//GEngine->ForceGarbageCollection(true);
	//}
	if (GEditor)
	{
		//GEditor->ForceGarbageCollection();
		//GEditor->ForceGarbageCollection(true);
		GEditor->PerformGarbageCollectionAndCleanupActors();
		//GEditor->GetEditorWorldContext().World()->CleanupWorld();
		UE_LOG(LogTemp, Warning, TEXT("%s::%d GC + Cleanup requested.."), *FString(__FUNCTION__), __LINE__);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnGenericButton()
{
	FScopedTransaction Transaction(LOCTEXT("GenericST", "Generic button.."));
	UWorld* CurrWorld = GEditor->GetEditorWorldContext().World();
	//UE_LOG(LogTemp, Error, TEXT("%s::%d *** -BEGIN-  GenericButton ***"), *FString(__FUNCTION__), __LINE__);

	///* WORLD */
	//UE_LOG(LogTemp, Log, TEXT("%s::%d CurrWorld:"), *FString(__FUNCTION__), __LINE__);
	//UE_LOG(LogTemp, Log, TEXT("\t\t\t%s"), *CurrWorld->GetName());
	//UE_LOG(LogTemp, Log, TEXT("***"));

	///* LEVELS */
	//UE_LOG(LogTemp, Log, TEXT("%s::%d Levels:"), *FString(__FUNCTION__), __LINE__);
	//for (const auto& Level : CurrWorld->GetLevels())
	//{
	//	UE_LOG(LogTemp, Log, TEXT("\t\t\t%s"), *Level->GetName());
	//}
	//UE_LOG(LogTemp, Log, TEXT("***"));

	///* STREAMING LEVELS */
	//UE_LOG(LogTemp, Log, TEXT("%s::%d Streaming levels:"), *FString(__FUNCTION__), __LINE__);
	//for (const auto& StreamingLevel : CurrWorld->GetStreamingLevels())
	//{
	//	UE_LOG(LogTemp, Log, TEXT("\t\t\t%s"), *StreamingLevel->GetName());
	//}
	//UE_LOG(LogTemp, Log, TEXT("***"));


	/* UOBJECTS INFO */
	//UE_LOG(LogTemp, Log, TEXT("%s::%d UObject Infos:"), *FString(__FUNCTION__), __LINE__);
	LogObjectInfo(CurrWorld);
	UE_LOG(LogTemp, Log, TEXT("***"));

	///* SELECTED ACTORS */
	//UE_LOG(LogTemp, Log, TEXT("%s::%d Selected actors: "), *FString(__func__), __LINE__);
	//for (const auto Act : GetSelectedActors())
	//{
	//	UE_LOG(LogTemp, Error, TEXT("\t\t\t%s"), *Act->GetName());
	//}
	//UE_LOG(LogTemp, Log, TEXT("***"));

	//UE_LOG(LogTemp, Error, TEXT("%s::%d *** -END- GenericButton ***"), *FString(__FUNCTION__), __LINE__);
	return FReply::Handled();
}


/* Managers */
// Check if the individual manager is set
bool FSLEdModeToolkit::HasValidIndividualManager() const
{
	return IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKill();
}

// Set the individual manager
bool FSLEdModeToolkit::SetIndividualManager()
{
	IndividualManager = FSLEdUtils::GetOrCreateNewIndividualManager(GEditor->GetEditorWorldContext().World());
	return HasValidIndividualManager();
}

// Check if the individual info manager is set
bool FSLEdModeToolkit::HasValidIndividualInfoManager() const
{
	return IndividualInfoManager && IndividualInfoManager->IsValidLowLevel() && !IndividualInfoManager->IsPendingKill();;
}

// Set the individual info manager
bool FSLEdModeToolkit::SetdIndividualInfoManager()
{
	IndividualInfoManager = FSLEdUtils::GetOrCreateNewVisualInfoManager(GEditor->GetEditorWorldContext().World());
	return HasValidIndividualInfoManager();
}


/* Helper functions */
TArray<AActor*> FSLEdModeToolkit::GetSelectedActors() const
{
	TArray<AActor*> Actors;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		Actors.Add(CastChecked<AActor>(*It));
	}

	//USelection* SelectedActors = GEditor->GetSelectedActors();
	//for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	//{
	//	if (AActor* Actor = Cast<AActor>(*Iter))
	//	{
	//		Actors.Add(Actor);
	//	}
	//}

	return Actors;
}

// Return the actor that is selected, return nullptr is no or more than one actors are selected
AActor* FSLEdModeToolkit::GetSingleSelectedActor() const
{
	//GEditor->GetSelectedActors()->GetTop<AActor>();

	AActor* SelectedActor = nullptr;
	// Counter to abort if there are more than one selections
	int8 NumSelection = 0;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		SelectedActor = CastChecked<AActor>(*It);
		++NumSelection;

		// Return nullptr if more than one item is selected
		if (NumSelection >= 2)
		{
			return nullptr;
		}
	}
	return SelectedActor;
}

// Deselect components to avoid crash when deleting the sl data component
void FSLEdModeToolkit::DeselectComponentSelection() const
{
	UActorComponent* SelectedActorComp = nullptr;
	for (FSelectionIterator It(GEditor->GetSelectedComponentIterator()); It; ++It)
	{
		GEditor->SelectComponent(CastChecked<UActorComponent>(*It), false, true);
	}
}

// Print out info about uobjects in editor
void FSLEdModeToolkit::LogObjectInfo(UWorld* World) const
{
	/* Iterate all objects */
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d **START** UObjects list:"), *FString(__FUNCTION__), __LINE__);
	//for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("\t\t %s \t [%s]"), *ObjectItr->GetName(), *ObjectItr->StaticClass()->GetName());
	//}
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d **END** UObjects list.."), *FString(__FUNCTION__), __LINE__);

	/* Iterate all actors */
	UE_LOG(LogTemp, Warning, TEXT("%s::%d **START** World actor list:"), *FString(__FUNCTION__), __LINE__);
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		UE_LOG(LogTemp, Log, TEXT("\t\t %s \t [%s]"), *ActItr->GetName(), *ActItr->StaticClass()->GetName());
		if (ActItr->IsInBlueprint())
		{
			UE_LOG(LogTemp, Log, TEXT("\t\t\t IN BP %s \t [%s]"), *ActItr->GetName(), *ActItr->StaticClass()->GetName());
		}

		if (UBlueprint* InBP = Cast<UBlueprint>(*ActItr))
		{
			UE_LOG(LogTemp, Log, TEXT("\t\t BP %s \t [%s]"), *ActItr->GetName(), *InBP->StaticClass()->GetName());
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s::%d **END** World actor list.."), *FString(__FUNCTION__), __LINE__);


	/* Iterate all blueprints */
	UE_LOG(LogTemp, Warning, TEXT("%s::%d **START** BP objects list:"), *FString(__FUNCTION__), __LINE__);
	for (TObjectIterator<UBlueprint> BpItr; BpItr; ++BpItr)
	{
		if (BpItr->GetWorld() == World)
		{
			UE_LOG(LogTemp, Log, TEXT("\t\t %s \t [%s]"), *BpItr->GetName(), *BpItr->GetArchetype()->StaticClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("\t\t NOT IN THE WORLD %s \t [%s]"), *BpItr->GetName(), *BpItr->GetArchetype()->StaticClass()->GetName());
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s::%d **END** BP objects list.."), *FString(__FUNCTION__), __LINE__);


	int32 InWorldNum = 0;
	int32 IsActorInWorldNum = 0;

	int32 IsPendingKillNum = 0;
	int32 IsActorPendingKillNum = 0;

	int32 TotalNum = 0;
	int32 IsActorTotalNum = 0;

	for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr)
	{
		/* In world */
		if (ObjectItr->GetWorld() == World)
		{
			InWorldNum++;
			if (ObjectItr->IsA(AActor::StaticClass()))
			{
				IsActorInWorldNum++;
			}
		}

		/* Pending kill */
		if (ObjectItr->IsPendingKill())
		{
			IsPendingKillNum++;
			if (ObjectItr->IsA(AActor::StaticClass()))
			{
				IsActorPendingKillNum++;
			}
		}

		/* Total */
		TotalNum++;
		if (ObjectItr->IsA(AActor::StaticClass()))
		{
			IsActorTotalNum++;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("\t\t\tInWorld:%ld(A)/%ld(O); \tPendingKill:%ld(A)/%ld(O); \tTotal:%ld(A)/%ld(O);"),
		IsActorInWorldNum, InWorldNum,
		IsActorPendingKillNum, IsPendingKillNum,
		IsActorTotalNum, TotalNum);
}

#undef LOCTEXT_NAMESPACE
