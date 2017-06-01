// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

//#include "SemLogEdModule.h"
//#include "SemLogEdMode.h"
//#include "SemLogEdModeToolkit.h"

#include "SemLogEdModeToolkit.h"
#include "SemLogEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
//#include "Private/EdModeUtils.h"


#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

FSemLogEdModeToolkit::FSemLogEdModeToolkit()
{
}

void FSemLogEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	const float Factor = 256.0f;
	
	struct Locals
	{
		static FReply GenerateSemanticMap()
		{
			return FReply::Handled();
		}
	};

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
					.Text(LOCTEXT("SemlogGenSemMap", "Generate Semantic Map"))
					.IsEnabled(true)
					.OnClicked_Static(&Locals::GenerateSemanticMap)
				]
			//SNew(SVerticalBox)
			//	+ SVerticalBox::Slot()
			//	.AutoHeight()
			//	.HAlign(HAlign_Center)
			//	.Padding(50)
			//	[
			//		SNew(STextBlock)
			//		.AutoWrapText(true)
			//		.Text(LOCTEXT("HelperLabel", "Semantic Logging, generate visualize the semantic representation of the objects"))
			//	]
			//+ SVerticalBox::Slot()
			//	.AutoHeight()
			//	.HAlign(HAlign_Center)
			//	[
			//		SNew(SButton)
			//		.Text(LOCTEXT("ShowAllObjectsTagsLabel", "ShowAllTags"))
			//		.IsEnabled(true)
			//		.OnClicked_Static(&EdUtils:OnShowAllObjectsTagsButtonClick)
			//	]
			//+ SVerticalBox::Slot()
			//	.AutoHeight()
			//	.HAlign(HAlign_Center)
			//	[
			//		SNew(SButton)
			//		.Text(LOCTEXT("ShowSelectedObjectsTagsLabel", "ShowSelectdTags"))
			//		.IsEnabled_Static(&EdUtils:IsAnythingSelected)
			//		.OnClicked_Static(&EdUtils:OnShowSelectedObjectsTagsButtonClick)
			//	]
			//+ SVerticalBox::Slot()
			//	.AutoHeight()
			//	.HAlign(HAlign_Left)
			//	.Padding(10)
			//	[
			//		SNew(STextBlock)
			//		.Text(LOCTEXT("TagsText", "etc.etc.\netc.etc"))// &EdUtils:GetTagsText)
			//	]
			//+ SVerticalBox::Slot()
			//	.AutoHeight()
			//	.HAlign(HAlign_Left)
			//	.Padding(10)
			//	[
			//		SNew(STextBlock)
			//		.Text(&EdUtils:GetTagsText)
			//	]
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FName FSemLogEdModeToolkit::GetToolkitFName() const
{
	return FName("Semantic Logger Editor Mode");
}

FText FSemLogEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("SemLogEdModeToolkit", "DisplayName", "Semantic Logger Editor Mode Tool");
}

class FEdMode* FSemLogEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FSemLogEdMode::EM_SemLogEdModeId);
}

#undef LOCTEXT_NAMESPACE
