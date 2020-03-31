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
		.Padding(25)
		[
			SNew(SVerticalBox)

			////
			+ CreateOverwriteSlot()

			////
			+ CreateOnlySelectedSlot()

			////
			+ CreateGenSemMapSlot()

			////
			+ CreateAddSemDataComponentsSlot()

			////
			+ CreateIdsSlot()

			////
			+ CreateClassNamesSlot()

			////
			+ CreateWriteVisualMasksSlot()

			////
			+ CreateRmAllSlot()

			////
			+ CreateAddSemMonSlot()

			////
			+ CreateEnableOverlapsSlot()

			////
			+ CreateShowSemData()

			////
			+ CreateEnableInstacedMeshMaterialsSlot()

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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateGenSemMapSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("GenSemMap", "Generate Semantic Map"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("GenSemMapTip", "Exports the generated semantic map to file"))
			.OnClicked(this, &FSLEdModeToolkit::OnGenSemMap)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateAddSemDataComponentsSlot()
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
				.Text(LOCTEXT("AddSemDataComp", "Add Semantic Data Components"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("AddSemDataCompTip", "Creates semantic data components.."))
				.OnClicked(this, &FSLEdModeToolkit::OnAddSemDataComp)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveToTag", "Save"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("SaveToTagTip", "Save data to tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnSaveToTag)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadFromTag", "Load"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("LoadFromTagTip", "Load data from tag.."))
				.OnClicked(this, &FSLEdModeToolkit::OnLoadFromTag)
			]
		];
}


SVerticalBox::FSlot& FSLEdModeToolkit::CreateIdsSlot()
{
	return 	SVerticalBox::Slot()
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
				.Text(LOCTEXT("GenSemIds", "Write Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("GenSemMapTip", "Generates unique ids for every semantic entity"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteSemIds)
			]

			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RmSemIds", "Remove Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmSemIdsTip", "Removes all generated ids"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmSemIds)
			]
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateClassNamesSlot()
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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateWriteVisualMasksSlot()
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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateRmAllSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("RmAllTags", "Remove all Tags"))
		.IsEnabled(true)
		.ToolTipText(LOCTEXT("RmAllTagsTip", "Removes all tags"))
		.OnClicked(this, &FSLEdModeToolkit::OnRmAll)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateAddSemMonSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
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
		.Padding(5)
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
		.Padding(5)
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
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("EnableMaterialsForInstancedStaticMesh", "Enable Materials for Instanced Static Mesh"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("EnableMaterialsForInstancedStaticMeshTip", "Make sure every material asset can be rendered as an instanced static mesh.."))
			.OnClicked(this, &FSLEdModeToolkit::OnEnableMaterialsForInstancedStaticMesh)
		];
}


/* Button callbacks */
////
FReply FSLEdModeToolkit::OnGenSemMap()
{
	FSLEdUtils::WriteSemanticMap(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

////
FReply FSLEdModeToolkit::OnAddSemDataComp()
{
	FScopedTransaction Transaction(LOCTEXT("AddSemanticDataCompST", "Add semantic data components"));
	if (bOnlySelected)
	{
		FSLEdUtils::AddSemanticDataComponents(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::AddSemanticDataComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnSaveToTag()
{
	FScopedTransaction Transaction(LOCTEXT("SaveDataToTagST", "Save semantic data to owners tag.."));
	if (bOnlySelected)
	{
		FSLEdUtils::SaveComponentDataToTag(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::SaveComponentDataToTag(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnLoadFromTag()
{
	FScopedTransaction Transaction(LOCTEXT("LoadDataFromTagST", "Load semantic data from owners tag.."));
	if (bOnlySelected)
	{
		FSLEdUtils::LoadComponentDataFromTag(GetSelectedActors(), bOverwrite);
	}
	else
	{
		FSLEdUtils::LoadComponentDataFromTag(GEditor->GetEditorWorldContext().World(), bOverwrite);
	}
	return FReply::Handled();
}

////
FReply FSLEdModeToolkit::OnWriteSemIds()
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

FReply FSLEdModeToolkit::OnRmSemIds()
{
	FScopedTransaction Transaction(LOCTEXT("RmSemIdsST", "Remove all semantic Ids"));
	if (bOnlySelected)
	{
		FSLEdUtils::RemoveUniqueIds(GetSelectedActors());
		//FSLEdUtils::RemoveTagKey(GetSelectedActors(), "SemLog", "Id");
	}
	else
	{
		FSLEdUtils::RemoveUniqueIds(GEditor->GetEditorWorldContext().World());
		//FSLEdUtils::RemoveTagKey(GEditor->GetEditorWorldContext().World(), "SemLog", "Id");
	}
	return FReply::Handled();
}

////
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
		//FSLEdUtils::RemoveTagKey(GetSelectedActors(), "SemLog", "Class");
	}
	else
	{
		FSLEdUtils::RemoveClassNames(GEditor->GetEditorWorldContext().World());
		//FSLEdUtils::RemoveTagKey(GEditor->GetEditorWorldContext().World(), "SemLog", "Class");
	}
	return FReply::Handled();
}

////
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
		FSLEdUtils::RemoveTagKey(GetSelectedActors(), "SemLog", "VisMask");
	}
	else
	{
		FSLEdUtils::RemoveTagKey(GEditor->GetEditorWorldContext().World(), "SemLog", "VisMask");
	}	
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmAll ()
{
	FScopedTransaction Transaction(LOCTEXT("RmAllST", "Remove all SemLog tags"));
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


/* Checkbox callbacks */
void FSLEdModeToolkit::OnCheckedOverwrite(ECheckBoxState NewCheckedState)
{
	bOverwrite = (NewCheckedState == ECheckBoxState::Checked);
}

void FSLEdModeToolkit::OnCheckedOnlySelected(ECheckBoxState NewCheckedState)
{
	bOnlySelected = (NewCheckedState == ECheckBoxState::Checked);
}


/* Helper functions */
TArray<AActor*> FSLEdModeToolkit::GetSelectedActors() const
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	TArray<AActor*> Actors;
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AActor* Actor = Cast<AActor>(*Iter))
		{
			Actors.Add(Actor);
		}
	}
	return Actors;
}

#undef LOCTEXT_NAMESPACE
