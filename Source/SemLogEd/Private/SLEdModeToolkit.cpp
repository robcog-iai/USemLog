// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
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

					//SNew(SHorizontalBox)
					//.Visibility(this, &SGitSourceControlSettings::CanInitializeGitRepository)
					//+ SHorizontalBox::Slot()
					//.FillWidth(0.1f)
					//[
					//	SNew(SCheckBox)
					//	.ToolTipText(LOCTEXT("CreateGitIgnore_Tooltip", "Create and add a standard '.gitignore' file"))
					//	.IsChecked(ECheckBoxState::Checked)
					//	.OnCheckStateChanged(this, &SGitSourceControlSettings::OnCheckedCreateGitIgnore)
					//]
					//+ SHorizontalBox::Slot()
					//.FillWidth(2.9f)
					//.VAlign(VAlign_Center)
					//[
					//	SNew(STextBlock)
					//	.Text(LOCTEXT("CreateGitIgnore", "Add a .gitignore file"))
					//	.ToolTipText(LOCTEXT("CreateGitIgnore_Tooltip", "Create and add a standard '.gitignore' file"))
					//	.Font(Font)
					//]
				

					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
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
