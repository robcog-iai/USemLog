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
//#include "SSCSEditor.h"
#include "UnrealEdGlobals.h" // for GUnrealEd
#include "Editor/UnrealEdEngine.h"

// UUtils
#include "SLEdUtils.h"


#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

// Ctor
FSLEdModeToolkit::FSLEdModeToolkit()
{
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
			+ CreateSemDataCompTxtSlot()
			+ CreateSemDataCompSlot()
			+ CreateSemDataCompFuncSlot()

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
				.ToolTipText(LOCTEXT("SemDataCompCreateTip", "Creates semantic data components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnCreateSemDataComp)
			]

			+ SHorizontalBox::Slot()
				.Padding(2)
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("SemDataCompLoad", "Load"))
					.IsEnabled(true)
					.ToolTipText(LOCTEXT("SemDataCompLoadTip", "Load semantic data components.."))
					.OnClicked(this, &FSLEdModeToolkit::OnLoadSemDataComp)
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
		];
}

/* Semantic data components functionalities */
SVerticalBox::FSlot& FSLEdModeToolkit::CreateSemDataCompFuncSlot()
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
				.Text(LOCTEXT("SemDataCompToggleMask", "Toggle Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompToggleMaskTip", "Toggle between the visual mask and the original colors.."))
				.OnClicked(this, &FSLEdModeToolkit::OnToggleMaskSemDataComp)
			]

			+ SHorizontalBox::Slot()
				.Padding(2)
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("SemDataCompShowText", "Toggle Text"))
					.IsEnabled(true)
					.ToolTipText(LOCTEXT("SemDataCompShowTextTip", "Show the semantic data of the components.."))
					.OnClicked(this, &FSLEdModeToolkit::OnToggleTextSemDataComp)
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
				.OnClicked(this, &FSLEdModeToolkit::OnSaveTagData)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SemDataCompLoad", "Import"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SemDataCompLoadTip", "Load data from tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnLoadTagData)
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
FReply FSLEdModeToolkit::OnCreateSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompCreateST", "Create semantic data components"));
	if (bOnlySelected)
	{
		FSLEdUtils::CreateSemanticDataComponents(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::CreateSemanticDataComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}

	GUnrealEd->UpdateFloatingPropertyWindows();

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompLoadST", "Load semantic data components"));
	if (bOnlySelected)
	{
		FSLEdUtils::LoadSemanticDataComponents(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::LoadSemanticDataComponents(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataComp()
{
	DeselectComponentsOnly();

	FScopedTransaction Transaction(LOCTEXT("SemDataCompRmST", "Remove semantic data components"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveSemanticDataComponents(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::RemoveSemanticDataComponents(GEditor->GetEditorWorldContext().World());
	}

	GUnrealEd->UpdateFloatingPropertyWindows();

	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleMaskSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompLoadST", "Load semantic data components"));
	if (bOnlySelected)
	{
		FSLEdUtils::ToggleMasks(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::ToggleMasks(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnToggleTextSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompLoadST", "Load semantic data components"));
	if (bOnlySelected)
	{
		FSLEdUtils::ToggleText(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::ToggleText(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}



////
FReply FSLEdModeToolkit::OnWriteSemDataAll()
{
	FScopedTransaction Transaction(LOCTEXT("WriteAllSemDataST", "Write all semantic data"));
	if (bOnlySelected)
	{
		FSLEdUtils::WriteUniqueIds(GetSelectedActors(), bOverwrite);
		FSLEdUtils::WriteClassNames(GetSelectedActors(), bOverwrite);
		FSLEdUtils::WriteVisualMasks(GetSelectedActors(), GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	else
	{
		FSLEdUtils::WriteUniqueIds(GEditor->GetEditorWorldContext().World(), bOverwrite);
		FSLEdUtils::WriteClassNames(GEditor->GetEditorWorldContext().World(), bOverwrite);
		FSLEdUtils::WriteVisualMasks(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataAll()
{
	FScopedTransaction Transaction(LOCTEXT("RmAllSemDataST", "Remove all semantic data"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveUniqueIds(GetSelectedActors());
		FSLEdUtils::RemoveClassNames(GetSelectedActors());
		FSLEdUtils::RemoveVisualMasks(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::RemoveUniqueIds(GEditor->GetEditorWorldContext().World());
		FSLEdUtils::RemoveClassNames(GEditor->GetEditorWorldContext().World());
		FSLEdUtils::RemoveVisualMasks(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteSemDataIds()
{
	FScopedTransaction Transaction(LOCTEXT("GenSemIdsST", "Generate new semantic Ids"));	
	if (bOnlySelected)
	{
		FSLEdUtils::WriteUniqueIds(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::WriteUniqueIds(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemDataIds()
{
	FScopedTransaction Transaction(LOCTEXT("RmSemIdsST", "Remove all semantic Ids"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveUniqueIds(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::RemoveUniqueIds(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("WriteClassNamesST", "Write class names"));
	if (bOnlySelected)
	{
		FSLEdUtils::WriteClassNames(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::WriteClassNames(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}	
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("RmClassNamesST", "Remove all class names"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveClassNames(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::RemoveClassNames(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("WriteVisualMasksST", "Write visual masks"));	
	if (bOnlySelected)
	{
		FSLEdUtils::WriteVisualMasks(GetSelectedActors(), GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	else
	{
		FSLEdUtils::WriteVisualMasks(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("RmVisualMasksST", "Remove all visual masks names"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveVisualMasks(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::RemoveVisualMasks(GEditor->GetEditorWorldContext().World());
	}	
	return FReply::Handled();
}

////
FReply FSLEdModeToolkit::OnSaveTagData()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompSaveST", "Save semantic data tag"));
	if (bOnlySelected)
	{
		FSLEdUtils::ExportToTag(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::ExportToTag(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadTagData()
{
	FScopedTransaction Transaction(LOCTEXT("SemDataCompLoadST", "Load semantic data from tag"));
	if (bOnlySelected)
	{
		FSLEdUtils::ImportFromTag(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::ImportFromTag(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnClearTagData()
{
	FScopedTransaction Transaction(LOCTEXT("RmTagData", "Remove all SemLog tags"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveTagType(GetSelectedActors(), "SemLog");
	}
	else
	{
		FSLEdUtils::RemoveTagType(GEditor->GetEditorWorldContext().World(), "SemLog");		
	}
	return FReply::Handled();
}


////
FReply FSLEdModeToolkit::OnAddSemMon()
{
	FScopedTransaction Transaction(LOCTEXT("AddSemMonitorsST", "Add semantic monitor components"));
	if (bOnlySelected)
	{
		FSLEdUtils::AddSemanticMonitorComponents(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::AddSemanticMonitorComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnEnableOverlaps()
{
	FScopedTransaction Transaction(LOCTEXT("EnableOverlapsST", "Enable overlaps"));
	if (bOnlySelected)
	{
		FSLEdUtils::EnableOverlaps(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::EnableOverlaps(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnShowSemData()
{
	FScopedTransaction Transaction(LOCTEXT("ShowSemDataST", "Show semantic data"));
	if (bOnlySelected)
	{
		FSLEdUtils::ShowSemanticData(GetSelectedActors());
	}
	else
	{
		FSLEdUtils::ShowSemanticData(GEditor->GetEditorWorldContext().World());
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnEnableMaterialsForInstancedStaticMesh()
{
	FScopedTransaction Transaction(LOCTEXT("AllMatForInstancedStaticMeshST", "Enable all materials for instanced static mesh rendering"));
	FSLEdUtils::EnableAllMaterialsForInstancedStaticMesh();
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnGenericButton()
{
	FScopedTransaction Transaction(LOCTEXT("GenericST", "Generic button"));

	UE_LOG(LogTemp, Error, TEXT("%s::%d Selected actors: "), *FString(__func__), __LINE__);

	for (const auto Act : GetSelectedActors())
	{
		UE_LOG(LogTemp, Error, TEXT("\t\t Act=%s"), *Act->GetName());
	}
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
void FSLEdModeToolkit::DeselectComponentsOnly() const
{
	UActorComponent* SelectedActorComp = nullptr;
	for (FSelectionIterator It(GEditor->GetSelectedComponentIterator()); It; ++It)
	{
		GEditor->SelectComponent(CastChecked<UActorComponent>(*It), false, true);
	}
}

#undef LOCTEXT_NAMESPACE
