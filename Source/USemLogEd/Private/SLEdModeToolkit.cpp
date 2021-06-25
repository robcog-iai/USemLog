// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
#include "Individuals/SLIndividualInfoManager.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/SLIndividualInfoUtils.h"

#include "Viz/SLVizEpisodeUtils.h"

#include "Owl/SLOwlSemMapDocUtils.h"
#include "Owl/SLOwlOntologyDocUtils.h"


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
	bProritizeChildrenFlag = true;	
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

			// Flag checkboxes
			+ CreateCompactCheckBoxSlot()
			//+ CreateOverwriteFlagSlot()
			//+ CreateOnlySelectedFlagSlot()
			//+ CreateIncludeChildrenFlagSlot()
			//+ CreateResetFlagSlot()
			//+ CreateTryImportFlagSlot()

			// Individual Components
			+ CreateSeparatorHackSlot()
			+ CreateIndividualsTxtSlot()
			+ CreateIndividualsSlot()
			+ CreateIndividualsFuncSlot()

			// Individual Values
			+ CreateIndividualValuesTxtSlot()
			+ CreateIndividualValuesAllSlot()
			+ CreateIndivualValuesIdSlot()
			+ CreateIndividualValuesClassSlot()
			+ CreateIndividualValuesVisualMaskSlot()

			// Import / Export
			+ CreateImportExportTxtSlot()
			+ CreateImportExportSlot()

			// Individual Visual Info
			+ CreateSeparatorHackSlot()
			+ CreateIndividualsInfoTxtSlot()
			+ CreateIndividualsInfoSlot()
			+ CreateIndividualsInfoFuncSlot()

			// Individual Managers
			+ CreateSeparatorHackSlot()
			+ CreateIndividualsManagersTxtSlot()
			+ CreateIndividualsManagersSlot()

			// Semantic Map
			+ CreateSeparatorHackSlot()
			+ CreateSemMapSlot()

			// Misc
			+ CreateSeparatorHackSlot()
			+ CreateUtilsTxtSlot()
			+ CreateConvertToVizMapSlot()
			+ CreateLogIdsSlot()
			+ CreateAddSemMonitorsSlot()
			+ CreateEnableOverlapsSlot()
			+ CreateShowSemData()
			+ CreateEnableInstacedMeshMaterialsSlot()
			+ CreateTriggerGCSlot()
			+ CreateGenericButtonSlot()
		];

	FModeToolkit::Init(InitToolkitHost);
}


/** Start IToolkit interface */
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
/** End IToolkit interface */

/* -Start- Vertical Slot Entries */
// Separator hack slot
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSeparatorHackSlot()
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
				.Text(LOCTEXT("SeparatorHack", "------------------------------------------"))
			]
		];
}

// Create the checkbox slots as a bundle
SVerticalBox::FSlot& FSLEdModeToolkit::CreateCompactCheckBoxSlot()
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
				.Text(LOCTEXT("OverwriteCB", "Overwrite:"))
			]
				+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("OverwriteCBTip", "Overwrites any existing data, use with caution"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteFlag)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OnlySelectedCB", " ||  Selected:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("OnlySelectedCBTip", "Consider only selected actors.."))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOnlySelectedFlag)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ResetCB", " ||  Reset:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("ResetCBTip", "Apply reset flag to any related functions."))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedResetFlag)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TryImportCB", " ||  Import:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("TryImportCBTip", "If available data will be imported or newly generated first.."))
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedTryImportFlag)
			]

			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ChildrenCB", " ||  Children:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("IncludeChildrenCB", "Include children data (e.g. bones/links..)"))
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedIncludeChildrenFlag)
			]
		];
}


// Checkboxes
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
				.Text(LOCTEXT("OverwriteCB", "Overwrite:"))
			]
				+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("OverwriteCBTip", "Overwrites any existing data, use with caution.."))
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
				.Text(LOCTEXT("OnlySelectedCB", "Selection:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("OnlySelectedCBTip", "Consider only selected actors"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOnlySelectedFlag)
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
				.Text(LOCTEXT("ResetCB", "Reset:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("ResetCBTip", "Apply reset flag to any related functions."))
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
				.Text(LOCTEXT("IncludeChildrenCB", "Children:"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("IncludeChildrenCBTip", "Include children data (e.g. bones/links..)"))
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedIncludeChildrenFlag)
			]
		];
}


// Individual Components
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
				.Text(LOCTEXT("CreateIndividuals", "Create"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("CreateIndividualsTip", "Create individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnCreateIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearIndividuals", "Clear"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ClearIndividualsTip", "Clear individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnClearIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("InitIndividuals", "Init"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("InitIndividualsTip", "Call Init(bReset) on individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadIndividuals", "Load"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("LoadIndividualsTip", "Call Load(bReset, bTryImport) on individuals.."))
				.OnClicked(this, &FSLEdModeToolkit::OnLoadIndividuals)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ConnectIndividuals", "Connect"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ConnectIndividualsTip", "Connect delegates between objects, components, managers, etc.."))
				.OnClicked(this, &FSLEdModeToolkit::OnConnectIndividuals)
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
				.ToolTipText(LOCTEXT("IndividualsToggleMaskTip", "Toggle between the visual mask and the original material visualization.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleIndividualVisualMaskVisiblity)
			]

		];
}


// Individual Values
SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualValuesTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IndividualValuesTxt", "Individual values:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualValuesAllSlot()
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
				.ToolTipText(LOCTEXT("WriteAllTip", "Write all individual values.."))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteAllIndvidualValues)
			]

		+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearAll", "Clear All"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ClearAllTip", "Clear all individual values.."))
				.OnClicked(this, &FSLEdModeToolkit::OnClearAllIndividualValues)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndivualValuesIdSlot()
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
				.ToolTipText(LOCTEXT("WriteIdsTip", "Generates unique ids for every individual.."))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteIndividualIds)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearIds", "Clear Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ClearIdsTip", "Clears all generated ids.."))
				.OnClicked(this, &FSLEdModeToolkit::OnClearIndividualIds)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualValuesClassSlot()
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
				.OnClicked(this, &FSLEdModeToolkit::OnWriteIndividualClasses)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RmClassNames", "Remove Class Names"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmClassNamesTip", "Removes all class names"))
				.OnClicked(this, &FSLEdModeToolkit::OnClearIndividualClasses)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualValuesVisualMaskSlot()
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
				.OnClicked(this, &FSLEdModeToolkit::OnWriteIndividualVisualMasks)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearVisualMasks", "Clear Visual Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ClearVisualMasksTip", "Removes all visual masks"))
				.OnClicked(this, &FSLEdModeToolkit::OnClearIndividualVisualMasks)
			]
		];
}


// Import / Export
SVerticalBox::FSlot& FSLEdModeToolkit::CreateImportExportTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ImportExportTxt", "Import/export individual values:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateImportExportSlot()
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
				.Text(LOCTEXT("ExportValues", "Export"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ExportValuesTip", "Export individual values.."))
				.OnClicked(this, &FSLEdModeToolkit::OnExportValues)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ImportValues", "Import"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ImportValuesTip", "Import individual values.."))
				.OnClicked(this, &FSLEdModeToolkit::OnImportValues)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearExportedValues", "Clear"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ClearExportedValuesTip", "Clear exported individual values.."))
				.OnClicked(this, &FSLEdModeToolkit::OnClearExportedValues)
			]
		];
}


// Individual Visual Info
SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsInfoTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IndividualsInfoTxt", "Individuals info components:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsInfoSlot()
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
				.Text(LOCTEXT("CreateIndividualsInfo", "Create"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("CreateIndividualsInfoTip", "Create individuals info components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnCreateIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearIndividualsInfo", "Clear"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ClearIndividualsInfo", "Clear individual info components"))
				.OnClicked(this, &FSLEdModeToolkit::OnClearIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("InitIndividualsInfo", "Init"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("InitIndividualsInfoTip", "Call Init(bReset) on the individuals info components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadIndividualsInfo", "Load"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("LoadIndividualsInfoTip", "Call Load(bReset) on the individuals info components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnLoadIndividualsInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ConnectIndividualsInfo", "Connect"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ConnectIndividualsInfoTip", "Call Load(bReset) on the individuals info components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnConnectIndividualsInfo)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsInfoFuncSlot()
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
				.Text(LOCTEXT("ToggleIndividualsInfo", "Toggle Visibility"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoToggleTip", "Toggle individuals info visibility.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleIndividualsInfoVisiblity)
			]

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



// Individual Managers
SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsManagersTxtSlot()
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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIndividualsManagersSlot()
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
				.Text(LOCTEXT("InitIndividualManagers", "Init"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("InitIndividualManagersTip", "Init managers.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitIndividualManagers)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadIndividualManagers", "Load"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("LoadIndividualManagersTip", "Load managers.."))
				.OnClicked(this, &FSLEdModeToolkit::OnLoadIndividualManagers)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ConnectIndividualManagers", "Connect"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("ConnectIndividualManagersTip", "Connect managers.."))
				.OnClicked(this, &FSLEdModeToolkit::OnConnectIndividualManagers)
			]
		];
}


// Semantic Map
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
			.Text(LOCTEXT("SemMapTxt", "Semantic Map: "))
		]

	+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.Text(LOCTEXT("SemMapWrite", "Write"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("SemMapWriteTip", "Exports the generated semantic map to an owl file"))
			.OnClicked(this, &FSLEdModeToolkit::OnWriteSemMap)
		]
		];
}


// Misc
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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateConvertToVizMapSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("ConvertToViz", "Convert Map to Viz"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("ConvertToVizTip", "Removes physics creates poseable mesh components.."))
			.OnClicked(this, &FSLEdModeToolkit::OnConvertToViz)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateLogIdsSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("LogIds", "Log Ids"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("LogIdsTip", "write the individual ids in the as a log string.."))
			.OnClicked(this, &FSLEdModeToolkit::OnLogIds)
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

// Info
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
/* -End- Vertical Slot Entries */



/* -Start- Callbacks */
// Flag checkboxes
void FSLEdModeToolkit::OnCheckedOverwriteFlag(ECheckBoxState NewCheckedState)
{
	bOverwriteFlag = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedOnlySelectedFlag(ECheckBoxState NewCheckedState)
{
	bOnlySelectedFlag = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedIncludeChildrenFlag(ECheckBoxState NewCheckedState)
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


// Individual Components
FReply FSLEdModeToolkit::OnCreateIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("IndividualsCreateST", "Create individual components"));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::CreateIndividualComponents(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::CreateIndividualComponents(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Created %ld new individual components.."),
			*FString(__FUNCTION__), __LINE__, Num);

		if (HasValidIndividualManager() && IndividualManager->Load(true))
		{
			IndividualManager->Connect();
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual manager successfully reloaded.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("ClearIndividualComponentsST", "Clear individual components"));
	DeselectComponentSelection();
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ClearIndividualComponents(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::ClearIndividualComponents(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cleared %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, Num);

		if (HasValidIndividualManager() && IndividualManager->Load(true))
		{
			IndividualManager->Connect();
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual manager successfully reloaded.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnInitIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("InitIndividualComponentsST", "Init individual components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::InitIndividualComponents(GetSelectedActors(), bResetFlag);
	}
	else
	{
		Num = FSLIndividualUtils::InitIndividualComponents(GEditor->GetEditorWorldContext().World(), bResetFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Init(bReset=%d) on %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, bResetFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("LoadIndividualComponentsST", "Load individual components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::LoadIndividualComponents(GetSelectedActors(), bResetFlag, bTryImportFlag);
	}
	else
	{
		Num = FSLIndividualUtils::LoadIndividualComponents(GEditor->GetEditorWorldContext().World(), bResetFlag, bTryImportFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Load(bReset=%d, bTryImport=%d) on %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, bResetFlag, bTryImportFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnConnectIndividuals()
{
	FScopedTransaction Transaction(LOCTEXT("ConnectIndividualComponentsST", "Connect individual components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ConnectIndividualComponents(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::ConnectIndividualComponents(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully connected %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, Num);
	}
	return FReply::Handled();
}

// Individual Components Funcs
FReply FSLEdModeToolkit::OnToggleIndividualVisualMaskVisiblity()
{
	FScopedTransaction Transaction(LOCTEXT("ToggleMaskIndividualComponentsST", "Toggle individuals visual maks visiblity.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ToggleVisualMaskVisibility(GetSelectedActors(), bProritizeChildrenFlag);
	}
	else
	{
		Num = FSLIndividualUtils::ToggleVisualMaskVisibility(GEditor->GetEditorWorldContext().World(), bProritizeChildrenFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called ToggleVisualMaskVisibility(bProritizeChildrenFlag=%d) on %ld individual components.."),
			*FString(__FUNCTION__), __LINE__, bProritizeChildrenFlag, Num);
	}
	return FReply::Handled();
}


// Individual Values
FReply FSLEdModeToolkit::OnWriteAllIndvidualValues()
{
	FScopedTransaction Transaction(LOCTEXT("WriteAllIndividualValuesST", "Writing all individual values.."));
	OnWriteIndividualIds();
	OnWriteIndividualClasses();
	OnWriteIndividualVisualMasks();
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearAllIndividualValues()
{
	FScopedTransaction Transaction(LOCTEXT("ClearAllIndividualValuesST", "Clearing all individual values.."));
	OnClearIndividualIds();
	OnClearIndividualClasses();
	OnClearIndividualVisualMasks();
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteIndividualIds()
{
	FScopedTransaction Transaction(LOCTEXT("WriteIdsST", "Write individual ids.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::WriteIds(GetSelectedActors(), bOverwriteFlag);
	}
	else
	{
		Num = FSLIndividualUtils::WriteIds(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Wrote (bOverwrite=%d) %ld new individual ids.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearIndividualIds()
{
	FScopedTransaction Transaction(LOCTEXT("ClearIdsST", "Clear individual ids.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ClearIds(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::ClearIds(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cleared %ld individual ids.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteIndividualClasses()
{
	FScopedTransaction Transaction(LOCTEXT("WriteClassNamesST", "Write individual class names.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::WriteClasses(GetSelectedActors(), bOverwriteFlag);
	}
	else
	{
		Num = FSLIndividualUtils::WriteClasses(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Wrote (bOverwrite=%d) %ld new individual classes.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);

		if (HasValidIndividualManager())
		{
			// TODO reload individuals
			//IndividualInfoManager->Refresh();
			/*UE_LOG(LogTemp, Log, TEXT("%s::%d Individual manager refreshed.."),
				*FString(__FUNCTION__), __LINE__);*/
		}
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearIndividualClasses()
{
	FScopedTransaction Transaction(LOCTEXT("ClearClassNamesST", "Clear individual class names.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ClearClasses(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::ClearClasses(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cleared %ld individual classes.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteIndividualVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("WriteIndividualVisualMasksST", "Write unique individual visual masks.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::WriteUniqueVisualMasks(GetSelectedActors(), bOverwriteFlag);
	}
	else
	{
		Num = FSLIndividualUtils::WriteUniqueVisualMasks(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Wrote (bOverwrite=%d) %ld new (parent) individual unique visual masks.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearIndividualVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("ClearIndividualVisualMaskST", "Clear individual visual mask values.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ClearVisualMasks(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::ClearVisualMasks(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cleared %ld individual's (parent) visual masks.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}


// Import / Export
FReply FSLEdModeToolkit::OnExportValues()
{
	FScopedTransaction Transaction(LOCTEXT("ExportValuesST", "Exporting individual values.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ExportValues(GetSelectedActors(), bOverwriteFlag);
	}
	else
	{
		Num = FSLIndividualUtils::ExportValues(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Exported (bOverwrite=%d) %ld individual values.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnImportValues()
{
	FScopedTransaction Transaction(LOCTEXT("ImportValuesST", "Importing individual values.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ImportValues(GetSelectedActors(), bOverwriteFlag);
	}
	else
	{
		Num = FSLIndividualUtils::ImportValues(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Imported (bOverwrite=%d) %ld individual values.."),
			*FString(__FUNCTION__), __LINE__, bOverwriteFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearExportedValues()
{
	FScopedTransaction Transaction(LOCTEXT("ClearExportedValuesST", "Clearing exported individual values.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualUtils::ClearExportedValues(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualUtils::ClearExportedValues(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cleared %ld individual exported values.."),
			*FString(__FUNCTION__), __LINE__, Num);
	}
	return FReply::Handled();
}


// Individual Visual Info
FReply FSLEdModeToolkit::OnCreateIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("CreateIndividualsInfoST", "Create individual info components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualInfoUtils::CreateIndividualInfoComponents(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualInfoUtils::CreateIndividualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Created %ld new individual info components.."),
			*FString(__FUNCTION__), __LINE__, Num);

		if (HasValidIndividualInfoManager() && IndividualInfoManager->Load(true))
		{
			IndividualInfoManager->Connect();
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual info manager successfully reloaded.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("ClearIndividualsInfoST", "Clear individual info components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualInfoUtils::ClearIndividualInfoComponents(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualInfoUtils::ClearIndividualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cleared %ld individual info components.."),
			*FString(__FUNCTION__), __LINE__, Num);

		if (HasValidIndividualInfoManager() && IndividualInfoManager->Load(true))
		{
			IndividualInfoManager->Connect();
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual info manager successfully reloaded.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnInitIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("InitIndividualsInfoST", "Init individual info components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualInfoUtils::InitIndividualInfoComponents(GetSelectedActors(), bResetFlag);
	}
	else
	{
		Num = FSLIndividualInfoUtils::InitIndividualInfoComponents(GEditor->GetEditorWorldContext().World(), bResetFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Init(bReset=%d) on %ld individual info components.."),
			*FString(__FUNCTION__), __LINE__, bResetFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("LoadIndividualsInfoST", "Load individual info components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualInfoUtils::LoadIndividualInfoComponents(GetSelectedActors(), bResetFlag);
	}
	else
	{
		Num = FSLIndividualInfoUtils::LoadIndividualInfoComponents(GEditor->GetEditorWorldContext().World(), bResetFlag);
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Load(bReset=%d) on %ld individual info components.."),
			*FString(__FUNCTION__), __LINE__, bResetFlag, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnConnectIndividualsInfo()
{
	FScopedTransaction Transaction(LOCTEXT("ConnectIndividualsInfoST", "Connect individual info components.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualInfoUtils::ConnectIndividualInfoComponents(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualInfoUtils::ConnectIndividualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Connect() on %ld individual info components.."),
			*FString(__FUNCTION__), __LINE__, Num);
	}
	return FReply::Handled();
}


// Individual Visual Info Funcs
FReply FSLEdModeToolkit::OnToggleIndividualsInfoVisiblity()
{
	FScopedTransaction Transaction(LOCTEXT("ToggleIndividualsInfoVisibilityTS", "Toggle individuals info visibility.."));
	int32 Num = 0;
	if (bOnlySelectedFlag)
	{
		Num = FSLIndividualInfoUtils::ToggleIndividualInfoComponentsVisibilty(GetSelectedActors());
	}
	else
	{
		Num = FSLIndividualInfoUtils::ToggleIndividualInfoComponentsVisibilty(GEditor->GetEditorWorldContext().World());
	}

	if (Num)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully changed the visibility of %ld individual info components.."),
			*FString(__FUNCTION__), __LINE__, Num);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnUpdateIndividualsInfoOrientation()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoUpdateST", "Update visual info orientation"));
	int32 NumComp = 0;

	if (HasValidIndividualInfoManager())
	{
		//if (bOnlySelectedFlag)
		//{
		//	NumComp = IndividualInfoManager->LookAtCamera(GetSelectedActors());
		//}
		//else
		//{
		//	NumComp = IndividualInfoManager->LookAtCamera();
		//}
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


// Individual Managers
FReply FSLEdModeToolkit::OnInitIndividualManagers()
{
	FScopedTransaction Transaction(LOCTEXT("InitIndividualManagersST", "Init individual managers.."));

	if (HasValidIndividualManager() || SetIndividualManager())
	{
		if (IndividualManager->Init(bResetFlag))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Init(bResetFlag=%d) on the individual manager.."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Init(bResetFlag=%d) failed on the individual manager .."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No individual manager found to init.."), *FString(__FUNCTION__), __LINE__);
	}


	if (HasValidIndividualInfoManager() || SetIndividualInfoManager())
	{
		if (IndividualManager->Init(bResetFlag))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Init(bResetFlag=%d) on the individual info manager.."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Init(bResetFlag=%d) failed on the individual info manager .."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No individual info manager found to init.."), *FString(__FUNCTION__), __LINE__);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadIndividualManagers()
{
	FScopedTransaction Transaction(LOCTEXT("LoadIndividualManagersST", "Load semantic data managers"));

	if (HasValidIndividualManager() || SetIndividualManager())
	{
		if (IndividualManager->Load(bResetFlag))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Load(bResetFlag=%d) on the individual manager.."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Load(bResetFlag=%d) failed on the individual manager .."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No individual manager found to load.."), *FString(__FUNCTION__), __LINE__);
	}

	if (HasValidIndividualInfoManager() || SetIndividualInfoManager())
	{
		if (IndividualInfoManager->Load(bResetFlag))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully called Load(bResetFlag=%d) on the individual info manager.."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Load(bResetFlag=%d) failed on the individual info manager .."),
				*FString(__FUNCTION__), __LINE__, bResetFlag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No individual info manager found to load.."), *FString(__FUNCTION__), __LINE__);
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnConnectIndividualManagers()
{
	FScopedTransaction Transaction(LOCTEXT("ConnectIndividualManagersST", "Connect semantic data managers"));
	const bool bReset = true;

	if (HasValidIndividualManager() || SetIndividualManager())
	{
		if (IndividualManager->Connect())
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual manager connected.."), *FString(__FUNCTION__), __LINE__);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to connect individual manager .."), *FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No individual manager found to connect.."), *FString(__FUNCTION__), __LINE__);
	}

	if (HasValidIndividualInfoManager() || SetIndividualInfoManager())
	{
		if (IndividualInfoManager->Connect())
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual info manager connected.."),
				*FString(__FUNCTION__), __LINE__);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to connect the individual info manager .."),
				*FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No individual info manager found to connect.."), *FString(__FUNCTION__), __LINE__);
	}

	return FReply::Handled();
}

// Semantic map
FReply FSLEdModeToolkit::OnWriteSemMap()
{
	FSLEdUtils::WriteSemanticMap(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	FSLOwlSemMapDocUtils::CreateAndPrintDoc(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	return FReply::Handled();
}

// Semantic map
FReply FSLEdModeToolkit::OnWriteOntology()
{
	FSLOwlOntologyDocUtils::CreateAndPrintDoc(GEditor->GetEditorWorldContext().World(), bOverwriteFlag);
	return FReply::Handled();
}

////
// Convert map to vizual only
FReply FSLEdModeToolkit::OnConvertToViz()
{
	FScopedTransaction Transaction(LOCTEXT("ConvertToVizST", "Converting map to VIZ.."));
	
	bool bMarkDirty = true;

	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	FSLVizEpisodeUtils::SetActorsAsVisualsOnly(GEditor->GetEditorWorldContext().World());

	// Add a poseable mesh component clone to the skeletal actors
	FSLVizEpisodeUtils::AddPoseablMeshComponentsToSkeletalActors(GEditor->GetEditorWorldContext().World());

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}


FReply FSLEdModeToolkit::OnLogIds()
{
	FScopedTransaction Transaction(LOCTEXT("LogIdsST", "Writing individual ids.."));

	if (bOnlySelectedFlag)
	{
		FSLEdUtils::LogIds(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::LogIds(GEditor->GetEditorWorldContext().World());
	}

	return FReply::Handled();
}


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

	for (const auto Act : GetSelectedActors())
	{
		if (!Act->IsValidLowLevel() || Act->IsPendingKillOrUnreachable())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Actor with issues found.."),
				*FString(__FUNCTION__), __LINE__, *Act->GetName());
		}
		else
		{
			for (const auto C : Act->GetComponents())
			{
				if (!C->IsValidLowLevel() || !C->IsRegistered() || C->IsPendingKillOrUnreachable())
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s: component with issues found.."),
						*FString(__FUNCTION__), __LINE__, *Act->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("%s::%d %s::%s all good.."),
						*FString(__FUNCTION__), __LINE__, *Act->GetName(), *C->GetName());
				}
			}
		}
	}


	////UE_LOG(LogTemp, Error, TEXT("%s::%d *** -BEGIN-  GenericButton ***"), *FString(__FUNCTION__), __LINE__);

	/////* WORLD */
	////UE_LOG(LogTemp, Log, TEXT("%s::%d CurrWorld:"), *FString(__FUNCTION__), __LINE__);
	////UE_LOG(LogTemp, Log, TEXT("\t\t\t%s"), *CurrWorld->GetName());
	////UE_LOG(LogTemp, Log, TEXT("***"));

	/////* LEVELS */
	////UE_LOG(LogTemp, Log, TEXT("%s::%d Levels:"), *FString(__FUNCTION__), __LINE__);
	////for (const auto& Level : CurrWorld->GetLevels())
	////{
	////	UE_LOG(LogTemp, Log, TEXT("\t\t\t%s"), *Level->GetName());
	////}
	////UE_LOG(LogTemp, Log, TEXT("***"));

	/////* STREAMING LEVELS */
	////UE_LOG(LogTemp, Log, TEXT("%s::%d Streaming levels:"), *FString(__FUNCTION__), __LINE__);
	////for (const auto& StreamingLevel : CurrWorld->GetStreamingLevels())
	////{
	////	UE_LOG(LogTemp, Log, TEXT("\t\t\t%s"), *StreamingLevel->GetName());
	////}
	////UE_LOG(LogTemp, Log, TEXT("***"));


	///* UOBJECTS INFO */
	////UE_LOG(LogTemp, Log, TEXT("%s::%d UObject Infos:"), *FString(__FUNCTION__), __LINE__);
	//LogObjectInfo(CurrWorld);
	//UE_LOG(LogTemp, Log, TEXT("***"));

	/////* SELECTED ACTORS */
	////UE_LOG(LogTemp, Log, TEXT("%s::%d Selected actors: "), *FString(__func__), __LINE__);
	////for (const auto Act : GetSelectedActors())
	////{
	////	UE_LOG(LogTemp, Error, TEXT("\t\t\t%s"), *Act->GetName());
	////}
	////UE_LOG(LogTemp, Log, TEXT("***"));

	////UE_LOG(LogTemp, Error, TEXT("%s::%d *** -END- GenericButton ***"), *FString(__FUNCTION__), __LINE__);
	return FReply::Handled();
}


/* Helpers */
// Managers
// Check if the individual manager is set
bool FSLEdModeToolkit::HasValidIndividualManager() const
{
	return IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKill();
}

// Set the individual manager
bool FSLEdModeToolkit::SetIndividualManager()
{
	IndividualManager = FSLIndividualUtils::GetOrCreateNewIndividualManager(GEditor->GetEditorWorldContext().World());
	return HasValidIndividualManager();
}

// Check if the individual info manager is set
bool FSLEdModeToolkit::HasValidIndividualInfoManager() const
{
	return IndividualInfoManager && IndividualInfoManager->IsValidLowLevel() && !IndividualInfoManager->IsPendingKill();;
}

// Set the individual info manager
bool FSLEdModeToolkit::SetIndividualInfoManager()
{
	IndividualInfoManager = FSLIndividualInfoUtils::GetOrCreateNewIndividualInfoManager(GEditor->GetEditorWorldContext().World());
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
