// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdModeToolkit.h"
#include "SLEdMode.h"
#include "SLEdToolkitStatics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
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
					SNew(SButton)
					.Text(LOCTEXT("GenSemMap", "Generate Semantic Map"))
					.IsEnabled(true)
					.OnClicked_Static(&FSLEdToolkitStatics::GenerateSemanticMap)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddRuntimeManager", "Add Runtime Manager"))
					.IsEnabled_Static(&FSLEdToolkitStatics::NoRuntimeManager)
					.OnClicked_Static(&FSLEdToolkitStatics::AddRuntimeManager)
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
