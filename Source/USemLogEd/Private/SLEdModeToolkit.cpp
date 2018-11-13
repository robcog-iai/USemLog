// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdModeToolkit.h"
#include "SLEdMode.h"
#include "SLEdToolkitStatics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

FSLEdModeToolkit::FSLEdModeToolkit()
{
}

void FSLEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("GenSemMap", "Generate Semantic Map"))
				.IsEnabled(true)
				.OnClicked_Static(&FSLEdToolkitStatics::GenerateSemanticMap)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SCheckBox)
					.ToolTipText(LOCTEXT("GenSemMap_Overwrite", "Overwrite"))
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged_Static(&FSLEdToolkitStatics::OnCheckedOverwriteSemanticMap)
				]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddRuntimeManager", "Add Runtime Manager"))
					.IsEnabled_Static(&FSLEdToolkitStatics::NoRuntimeManagerInTheWorld)
					.OnClicked_Static(&FSLEdToolkitStatics::AddRuntimeManager)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddLevelInfo", "Add Level Info"))
				.IsEnabled_Static(&FSLEdToolkitStatics::NoLevelInfoInTheWorld)
				.OnClicked_Static(&FSLEdToolkitStatics::AddLevelinfo)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("GenerateNewIds", "Generate New Ids"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::GenerateNewIds)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("ClearIds", "Clear Ids"))
				.IsEnabled(true)
				.OnClicked_Static(&FSLEdToolkitStatics::ClearIds)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("TagSemanticContraints", "Tag Semantic Constraints"))
				.IsEnabled(true)
				.OnClicked_Static(&FSLEdToolkitStatics::TagSemanticConstraints)
				]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("TagSemanticClasses", "Tag Semantic Classes"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::TagSemanticClasses)
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("ClearClasses", "Clear Classes"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::ClearClasses)
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("ReplaceText", "Replace Text"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::ReplaceText)
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("ClearAllTags", "Clear All Tags"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::ClearAllTags)
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("UpdateSLOverlapShapeColors", "Update SL Overlap Shape Colors"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::UpdateSLOverlapShapeColors)
					]
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FName FSLEdModeToolkit::GetToolkitFName() const
{
	return FName("Semantic Logger Editor Mode");
}

FText FSLEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("SemLogEdModeToolkit", "DisplayName", "Semantic Logger Editor Mode Tool");
}

class FEdMode* FSLEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FSLEdMode::EM_SLEdModeId);
}

#undef LOCTEXT_NAMESPACE
