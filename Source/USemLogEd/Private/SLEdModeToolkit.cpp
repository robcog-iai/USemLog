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

// SL
#include "Data/SLIndividualManager.h"
#include "Data/SLIndividualVisualInfoManager.h"

// UUtils
#include "SLEdUtils.h"

#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

// Ctor
FSLEdModeToolkit::FSLEdModeToolkit()
{
	IndividualManager = nullptr;
	VisualInfoMananger = nullptr;
	/* Checkbox states */
	bOverwrite = false;
	bOnlySelected = false;
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
			+ CreateOverwriteSlot()
			+ CreateOnlySelectedSlot()

			////
			+ CreateSemMapSlot()

			////
			+ CreateSemDataManagersTxtSlot()
			+ CreateSemDataManagersSlot()

			////
			+ CreateSemDataCompTxtSlot()
			+ CreateSemDataCompSlot()

			////
			+ CreateSemDataVisInfoTxt()
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
SVerticalBox::FSlot& FSLEdModeToolkit::CreateOverwriteSlot()
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
				.Text(LOCTEXT("OverwriteTextLabel", "Overwrite existing data? "))
			]

				+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckBoxOverwrite", "Overwrites any existing data, use with caution"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwrite)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateOnlySelectedSlot()
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
				.Text(LOCTEXT("OnlySelectedTextLabel", "Consider only selected actors? "))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("CheckBoxOnlySelected", "Consider only selected actors"))
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOnlySelected)
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
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SemDataManagersTxt", "Semantic Data Managers:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataManagersSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataManagersInit", "Init"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataManagersInitTip", "Loads (or addsn new) managers from (or to) the world, and initializes them.."))
				.OnClicked(this, &FSLEdModeToolkit::OnInitSemDataManagers)
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataManagersReLoad", "ReLoad"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataManagersReLoadTip", "Re-loads the components from the world (clean + init).."))
				.OnClicked(this, &FSLEdModeToolkit::OnReLoadSemDataManagers)
			]
		];
}


/* Semantic data components */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataCompTxtSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SemDataCompTxt", "Semantic Data Components:"))
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataCompSlot()
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
				.Text(LOCTEXT("SemDataCompCreate", "Create"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompCreateTip", "Create semantic data components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnCreateSemDataComp)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataCompReLoad", "Re-Load"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompReLoadTip", "Re-Load semantic data components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnReLoadSemDataComp)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataCompRm", "Remove"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompRmTip", "Remove semantic data components (make sure no related editor windows are open).."))
				.OnClicked(this, &FSLEdModeToolkit::OnRmSemDataComp)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataCompToggleMask", "Toggle Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompToggleMaskTip", "Toggle between the visual mask and the original colors.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleMaskSemDataComp)
			]
		];
}


/* Semantic data visual info components */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataVisInfoTxt()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SemDataVisInfoTxt", "Semantic Data Visual Info:"))
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
				.OnClicked(this, &FSLEdModeToolkit::OnCreateSemDataVisInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoRefresh", "Refresh"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoRefreshTip", "Refresh visual values.."))
				.OnClicked(this, &FSLEdModeToolkit::OnRefreshSemDataVisInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoRm", "Remove"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoRmTip", "Remove visual info components (make sure no related editor windows are open).."))
				.OnClicked(this, &FSLEdModeToolkit::OnRmSemDataVisInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoToggle", "Toggle Visibility"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoToggleTip", "Toggle visual info visibility.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleSemDataVisInfo)
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
				.Text(LOCTEXT("SemDataVisInfoUpdate", "UpdateOrientation"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoUpdateTip", "Point text towards camera.."))
				.OnClicked(this, &FSLEdModeToolkit::OnUpdateSemDataVisInfo)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataVisInfoLIveUpdate", "AutoUpdateToggle"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataVisInfoLIveUpdateTip", "Toggle live update.."))
				.OnClicked(this, &FSLEdModeToolkit::OnLiveUpdateSemDataVisInfo)
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
				.Text(LOCTEXT("SemDataCompSave", "Export"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompSaveTip", "Save data to tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnExportToTag)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataCompLoad", "Import"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompLoadTip", "Load data from tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnImportFromTag)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RemoveTagData", "Clear"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RemoveTagDataTip", "Removes data stored in tags.."))
				.OnClicked(this, &FSLEdModeToolkit::OnClearTagData)
			]
		];
}

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
void FSLEdModeToolkit::OnCheckedOverwrite(ECheckBoxState NewCheckedState)
{
	bOverwrite = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedOnlySelected(ECheckBoxState NewCheckedState)
{
	bOnlySelected = (NewCheckedState == ECheckBoxState::Checked);
}


/* Button callbacks */
////
FReply FSLEdModeToolkit::OnWriteSemMap()
{
	FSLEdUtils::WriteSemanticMap(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

////
FReply FSLEdModeToolkit::OnInitSemDataManagers()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataManagerInit", "Init/Create+Init semantic data managers"));

	if (IndividualManager && IndividualManager->IsValidLowLevel())
	{
		IndividualManager->Init();
	}
	else
	{
		IndividualManager = FSLEdUtils::GetIndividualManager(GEditor->GetEditorWorldContext().World());
		if (IndividualManager && IndividualManager->IsValidLowLevel())
		{
			IndividualManager->Init();
		}
	}

	if (VisualInfoMananger && VisualInfoMananger->IsValidLowLevel())
	{
		VisualInfoMananger->Init();
	}
	else
	{
		VisualInfoMananger = FSLEdUtils::GetVisualInfoManager(GEditor->GetEditorWorldContext().World());
		if (VisualInfoMananger && VisualInfoMananger->IsValidLowLevel())
		{
			VisualInfoMananger->Init();
		}
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnReLoadSemDataManagers()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataManagerReLoad", "Reload semantic data managers"));
	const bool bReset = true;

	if (IndividualManager && IndividualManager->IsValidLowLevel())
	{
		IndividualManager->Init(bReset);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager not set, call init first.."), *FString(__FUNCTION__), __LINE__);
	}

	if (VisualInfoMananger && VisualInfoMananger->IsValidLowLevel())
	{
		VisualInfoMananger->Init(bReset);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual visual manager not set, call init first.."), *FString(__FUNCTION__), __LINE__);
	}

	return FReply::Handled();
}

////
FReply FSLEdModeToolkit::OnCreateSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompCreateST", "Create semantic data components"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::CreateSemanticDataComponents(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::CreateSemanticDataComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnReLoadSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompLoadST", "Re-Load semantic data components"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::ReLoadSemanticDataComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::ReLoadSemanticDataComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompRmST", "Remove semantic data components"));
	bool bMarkDirty = false;

	DeselectComponentSelection();

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveSemanticDataComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveSemanticDataComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleMaskSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompToggleMaskST", "Toggle semantic components visual maks visiblity"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::ToggleMasks(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::ToggleMasks(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}


////
FReply FSLEdModeToolkit::OnCreateSemDataVisInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoCreateST", "Create visual info components"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::CreateVisualInfoComponents(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::CreateVisualInfoComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRefreshSemDataVisInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoRefreshST", "Refresh visual info components"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RefreshVisualInfoComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::RefreshVisualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataVisInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoRmST", "Remove visual info components"));
	bool bMarkDirty = false;

	DeselectComponentSelection();

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveVisualInfoComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveVisualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GUnrealEd->UpdateFloatingPropertyWindows();
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleSemDataVisInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoToggleST", "Toggle visual info components visibility"));
	bool bMarkDirty = false;
	
	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::ToggleVisualInfoComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::ToggleVisualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnUpdateSemDataVisInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoUpdateST", "Update visual info orientation"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::UpdateVisualInfoComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::UpdateVisualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLiveUpdateSemDataVisInfo()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataVisInfoLiveUpdateST", "Toggle live visual info orientation"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::ToggleLiveUpdateVisualInfoComponents(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::ToggleLiveUpdateVisualInfoComponents(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}



////
FReply FSLEdModeToolkit::OnWriteSemDataAll()
{
	FScopedTransaction Transaction(LOCTEXT("WriteAllSemDataST", "Write all semantic data"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::WriteUniqueIds(GetSelectedActors(), bOverwrite) || bMarkDirty;
		bMarkDirty = FSLEdUtils::WriteClassNames(GetSelectedActors(), bOverwrite) || bMarkDirty;
		bMarkDirty = FSLEdUtils::WriteVisualMasks(GetSelectedActors(), GEditor->GetEditorWorldContext().World(), bOverwrite) || bMarkDirty;
	}
	else
	{
		bMarkDirty = FSLEdUtils::WriteUniqueIds(GEditor->GetEditorWorldContext().World(), bOverwrite) || bMarkDirty;
		bMarkDirty = FSLEdUtils::WriteClassNames(GEditor->GetEditorWorldContext().World(), bOverwrite) || bMarkDirty;
		bMarkDirty = FSLEdUtils::WriteVisualMasks(GEditor->GetEditorWorldContext().World(), bOverwrite) || bMarkDirty;
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataAll()
{
	FScopedTransaction Transaction(LOCTEXT("RmAllSemDataST", "Remove all semantic data"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveUniqueIds(GetSelectedActors()) || bMarkDirty;
		bMarkDirty = FSLEdUtils::RemoveClassNames(GetSelectedActors()) || bMarkDirty;
		bMarkDirty = FSLEdUtils::RemoveVisualMasks(GetSelectedActors()) || bMarkDirty;
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveUniqueIds(GEditor->GetEditorWorldContext().World()) || bMarkDirty;
		bMarkDirty = FSLEdUtils::RemoveClassNames(GEditor->GetEditorWorldContext().World()) || bMarkDirty;
		bMarkDirty = FSLEdUtils::RemoveVisualMasks(GEditor->GetEditorWorldContext().World()) || bMarkDirty;
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteSemDataIds()
{
	bool bMarkDirty = false;

	FScopedTransaction Transaction(LOCTEXT("GenSemIdsST", "Generate new semantic Ids"));	
	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::WriteUniqueIds(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::WriteUniqueIds(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataIds()
{
	FScopedTransaction Transaction(LOCTEXT("RmSemIdsST", "Remove all semantic Ids"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveUniqueIds(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveUniqueIds(GEditor->GetEditorWorldContext().World());
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("WriteClassNamesST", "Write class names"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::WriteClassNames(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::WriteClassNames(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("RmClassNamesST", "Remove all class names"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveClassNames(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveClassNames(GEditor->GetEditorWorldContext().World());
	}
	
	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}
	
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("WriteVisualMasksST", "Write visual masks"));	
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::WriteVisualMasks(GetSelectedActors(), GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::WriteVisualMasks(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("RmVisualMasksST", "Remove all visual masks names"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveVisualMasks(GetSelectedActors());
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveVisualMasks(GEditor->GetEditorWorldContext().World());
	}
	
	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}
	
	return FReply::Handled();
}

////
FReply FSLEdModeToolkit::OnExportToTag()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompSaveST", "Export semantic data to tag"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::ExportToTag(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::ExportToTag(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnImportFromTag()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompLoadST", "Import data from tag"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::ImportFromTag(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::ImportFromTag(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearTagData()
{
	FScopedTransaction Transaction(LOCTEXT("RmTagData", "Remove all SemLog tags"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::RemoveTagType(GetSelectedActors(), "SemLog");
	}
	else
	{
		bMarkDirty = FSLEdUtils::RemoveTagType(GEditor->GetEditorWorldContext().World(), "SemLog");
	}

	if (bMarkDirty)
	{
		GEditor->GetEditorWorldContext().World()->MarkPackageDirty();
	}

	return FReply::Handled();
}


////
FReply FSLEdModeToolkit::OnAddSemMon()
{
	FScopedTransaction Transaction(LOCTEXT("AddSemMonitorsST", "Add semantic monitor components"));
	bool bMarkDirty = false;

	if (bOnlySelected)
	{
		bMarkDirty = FSLEdUtils::AddSemanticMonitorComponents(GetSelectedActors(), bOverwrite);
	}
	else
	{
		bMarkDirty = FSLEdUtils::AddSemanticMonitorComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
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

	if (bOnlySelected)
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

	if (bOnlySelected)
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
