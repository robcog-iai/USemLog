// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdModeToolkit.h"
#include "SLEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"

#include "SLMap.h"


#define LOCTEXT_NAMESPACE "FSLEdModeToolkit"

FSLEdModeToolkit::FSLEdModeToolkit()
{
}

void FSLEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	const float Factor = 256.0f;
	
	struct Locals
	{
		static bool AreActorsSelected()
		{
			return GEditor->GetSelectedActors()->Num() != 0;
		}

		static FReply GenerateSemanticMap()
		{
			USLMap* SemMap = NewObject<USLMap>();
			if (SemMap)
			{
				SemMap->Generate(GEditor->GetEditorWorldContext().World());
				SemMap->WriteToFile();
			}
			//SemMap->BeginDestroy();
			return FReply::Handled();
		}

		static FReply AddContactListener()
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
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("AButton", "A Button"))
					.IsEnabled_Static(&Locals::AreActorsSelected)
					.OnClicked_Static(&Locals::AddContactListener)
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
