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
			+ CreateAddSemDataSlot()

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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateIdsSlot()
{
	return 	SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("GenSemIds", "Write Ids"))
					.IsEnabled(true)
					.ToolTipText(LOCTEXT("GenSemMapTip", "Generates unique ids for every semantic entity"))
					.OnClicked(this, &FSLEdModeToolkit::OnWriteSemIds)
				]

				+ SHorizontalBox::Slot()
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
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("WriteClassNames", "Write Class Names"))
					.IsEnabled(true)
					.ToolTipText(LOCTEXT("WriteClassNames", "Writes known class names"))
					.OnClicked(this, &FSLEdModeToolkit::OnWriteClassNames)
				]

				+ SHorizontalBox::Slot()
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
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("WriteVisualMasks", "Write Visual Masks"))
					.IsEnabled(true)
					.ToolTipText(LOCTEXT("WriteVisualMasksTip", "Writes unique visual masks for visual entities"))
					.OnClicked(this, &FSLEdModeToolkit::OnWriteClassNames)
				]

				+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Text(LOCTEXT("RmVisualMasks", "Remove Visual Masks"))
					.IsEnabled(true)
					.ToolTipText(LOCTEXT("RmVisualMasksTip", "Removes all visual masks"))
					.OnClicked(this, &FSLEdModeToolkit::OnRmClassNames)
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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateAddSemDataSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("AddSemData", "Add Semantic Data Components"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("AddSemDataTip", "Creates or updates semantic data components.."))
			.OnClicked(this, &FSLEdModeToolkit::OnAddSemData)
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
// Generate semantic map from editor world
FReply FSLEdModeToolkit::OnGenSemMap()
{
	FSLEdUtils::WriteSemanticMap(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteSemIds()
{
	FScopedTransaction Transaction(LOCTEXT("GenNewSemIds", "Generate new semantic Ids"));
	FSLEdUtils::WriteUniqueIds(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmSemIds()
{
	FScopedTransaction Transaction(LOCTEXT("RmSemIds", "Remove all semantic Ids"));
	FSLEdUtils::RemoveTagKey(GEditor->GetEditorWorldContext().World(), "SemLog", "Id");
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("WriteClassNames", "Write class names"));
	FSLEdUtils::WriteClassNames(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmClassNames()
{
	FScopedTransaction Transaction(LOCTEXT("RmClassNames", "Remove all class names"));
	FSLEdUtils::RemoveTagKey(GEditor->GetEditorWorldContext().World(), "SemLog", "Class");
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnWriteVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("WriteVisualMasks", "Write visual masks"));
	FSLEdUtils::WriteVisualMasks(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("RmVisualMasks", "Remove all visual masks names"));
	FSLEdUtils::RemoveTagKey(GEditor->GetEditorWorldContext().World(), "SemLog", "VisMask");
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnRmAll ()
{
	FScopedTransaction Transaction(LOCTEXT("RmAll", "Remove all SemLog tags"));
	FSLEdUtils::RemoveTagType(GEditor->GetEditorWorldContext().World(), "SemLog");
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnAddSemMon()
{
	FScopedTransaction Transaction(LOCTEXT("AddSemMonitors", "Add semantic monitor components"));
	FSLEdUtils::AddSemanticMonitorComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnAddSemData()
{
	FScopedTransaction Transaction(LOCTEXT("AddSemanticDataComp", "Add semantic data components"));
	FSLEdUtils::AddSemanticDataComponents(GEditor->GetEditorWorldContext().World(), bOverwrite);
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnEnableOverlaps()
{
	FScopedTransaction Transaction(LOCTEXT("EnableOverlaps", "Enable overlaps"));
	FSLEdUtils::EnableOverlaps(GEditor->GetEditorWorldContext().World());
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnShowSemData()
{
	FScopedTransaction Transaction(LOCTEXT("ShowSemData", "Show semantic data"));
	FSLEdUtils::ShowSemanticData(GEditor->GetEditorWorldContext().World());
	return FReply::Handled();
}

FReply FSLEdModeToolkit::OnEnableMaterialsForInstancedStaticMesh()
{
	FScopedTransaction Transaction(LOCTEXT("AllMatForInstancedStaticMesh", "Enable all materials for instanced static mesh rendering"));
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


// Return true if any actors are selected in the viewport
bool FSLEdModeToolkit::AreActorsSelected()
{
	return GEditor->GetSelectedActors()->Num() != 0;
}

#undef LOCTEXT_NAMESPACE
