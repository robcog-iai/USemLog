// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdModeToolkit.h"
#include "SLEdMode.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "Engine/Selection.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "SLSemanticMapWriter.h"
#include "SLOverlapShape.h"
#include "Ids.h"
#include "Tags.h"

#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

// Ctor
FSLEdModeToolkit::FSLEdModeToolkit()
{
	bOverwriteSemanticMap = true;
	bOverwriteExistingClassNames = true;
	bOverwriteExistingVisualMaskValues = true;
}

void FSLEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
		[
			SNew(SVerticalBox)
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("GenSemMap", "Generate Map"))
								.IsEnabled(true)
								.ToolTipText(LOCTEXT("GenSemMapTooltip", "Export semantic map"))
								.OnClicked(this, &FSLEdModeToolkit::GenerateSemanticMap)
							]
						+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
								.ToolTipText(LOCTEXT("GenSemMap_Overwrite", "Overwrite"))
								.IsChecked(ECheckBoxState::Checked)
								.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteSemanticMap)
							]
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("GenerateNewSemIds", "Generate New Ids"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::GenerateNewSemanticIds)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("RemoveSemIds", "Remove All Ids"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::RemoveAllSemanticIds)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("SemanticallyAnnotateConstraints", "Annotate Constraints"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::SemanticallyAnnotateConstraints)
					]
				/////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("SetDefaultClassNames", "Set Class Names"))
								.IsEnabled(true)
								.OnClicked(this, &FSLEdModeToolkit::SetClassNamesToDefault)
							]
						+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
								.ToolTipText(LOCTEXT("SetDefaultClassNames_Overwrite", "Overwrite"))
							.IsChecked(ECheckBoxState::Checked)
							.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteClassNames)
							]
					]
				/////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("SetVisualMaskValues", "Set Visual Mask Values"))
							.IsEnabled(true)
							.OnClicked(this, &FSLEdModeToolkit::SetVisualMaskValues)
							]
						+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
								.ToolTipText(LOCTEXT("SetVisualMaskValues_Overwrite", "Overwrite"))
							.IsChecked(ECheckBoxState::Checked)
							.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteVisualMaskValues)
							]
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("ReplaceText", "Update Legacy Names"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::UpdateLegacyNames)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("ClearAllTags", "Remove All Tags"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::RemoveAllTags)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("UpdateSLOverlapShapeColors", "Update Semantic Overlap Shape Visuals"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::UpdateSLOverlapShapeColors)
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

/** Callbacks */
// Return true if any actors are selected in the viewport
bool FSLEdModeToolkit::AreActorsSelected()
{
	return GEditor->GetSelectedActors()->Num() != 0;
}

// Generate semantic map from editor world
FReply FSLEdModeToolkit::GenerateSemanticMap()
{
	// Create writer
	FSLSemanticMapWriter SemMapWriter;

	// TODO use bOverwriteSemanticMap

	// Generate map and write to file
	SemMapWriter.WriteToFile(GEditor->GetEditorWorldContext().World(),
		ESLOwlSemanticMapTemplate::IAIKitchen, TEXT("SemLog"), TEXT("SemanticMap"));

	return FReply::Handled();
}

// Set flag attribute depending on the check-box state
void FSLEdModeToolkit::OnCheckedOverwriteSemanticMap(ECheckBoxState NewCheckedState)
{
	bOverwriteSemanticMap = (NewCheckedState == ECheckBoxState::Checked);
}

// Generate new semantic ids
FReply FSLEdModeToolkit::GenerateNewSemanticIds()
{
	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		int32 TagIndex = FTags::GetTagTypeIndex(*ActItr, "SemLog");
		if (TagIndex != INDEX_NONE)
		{
			FTags::AddKeyValuePair(
				ActItr->Tags[TagIndex], "Id", FIds::NewGuidInBase64Url());
		}

		// Check component tags as well
		for (const auto& CompItr : ActItr->GetComponents())
		{
			int32 TagIndex = FTags::GetTagTypeIndex(CompItr, "SemLog");
			if (TagIndex != INDEX_NONE)
			{
				FTags::AddKeyValuePair(
					CompItr->ComponentTags[TagIndex], "Id", FIds::NewGuidInBase64());
			}
		}
	}
	return FReply::Handled();
}

// Generate semantic ids for constraints
FReply FSLEdModeToolkit::SemanticallyAnnotateConstraints()
{
	for (TObjectIterator<UPhysicsConstraintComponent> ConstrItr; ConstrItr; ++ConstrItr)
	{
		// Check if constraint is not already tagged
		if (!FTags::HasType(*ConstrItr, "SemLog") &&
			ConstrItr->ConstraintActor1 != nullptr &&
			ConstrItr->ConstraintActor2 != nullptr)
		{
			// Check if constrained actors are tagged with a class
			if (FTags::HasKey(ConstrItr->ConstraintActor1, "SemLog", "Class") &&
				FTags::HasKey(ConstrItr->ConstraintActor2, "SemLog", "Class"))
			{
				FTags::AddKeyValuePair(*ConstrItr, "SemLog", "Id", FIds::NewGuidInBase64Url());
			}
		}
	}
	return FReply::Handled();
}

// Name semantic classes from asset name
FReply FSLEdModeToolkit::SetClassNamesToDefault()
{
	// Iterate only static mesh actors
	for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			// Ignore if actor is already tagged
			if (!FTags::HasKey(*ActItr, "SemLog", "Class"))
			{
				// Ignore if component is already tagged
				if (!FTags::HasKey(SMC, "SemLog", "Class"))
				{
					// Get the class name from the asset name
					FString ClassName = SMC->GetStaticMesh()->GetFullName();

					// Remove path info and prefix
					int32 FindCharPos;
					ClassName.FindLastChar('.', FindCharPos);
					ClassName.RemoveAt(0, FindCharPos + 1);
					ClassName.RemoveFromStart(TEXT("SM_"));

					// Check if the class should be added to the actor or the component
					if (FTags::HasType(*ActItr, "SemLog"))
					{
						// Tag the actor because it is semantically tagged but is missing the class name
						FTags::AddKeyValuePair(*ActItr, "SemLog", "Class", ClassName);
					}
					else if (FTags::HasType(SMC, "SemLog"))
					{
						// Tag the component because it is semantically tagged but is missing the class name
						FTags::AddKeyValuePair(SMC, "SemLog", "Class", ClassName);
					}
					else
					{
						// None have the semlog tag key, generate new one to the actor
						FTags::AddTagType(*ActItr, "SemLog");
						FTags::AddKeyValuePair(*ActItr, "SemLog", "Class", ClassName);
					}
				}
				else if (bOverwriteExistingClassNames)
				{
					// Get the class name from the asset name
					FString ClassName = SMC->GetStaticMesh()->GetFullName();
					// Remove path info and prefix
					int32 FindCharPos;
					ClassName.FindLastChar('.', FindCharPos);
					ClassName.RemoveAt(0, FindCharPos + 1);
					ClassName.RemoveFromStart(TEXT("SM_"));
					FTags::AddKeyValuePair(SMC, "SemLog", "Class", ClassName);
				}

			}
			else if (bOverwriteExistingClassNames)
			{
				// Get the class name from the asset name
				FString ClassName = SMC->GetStaticMesh()->GetFullName();
				// Remove path info and prefix
				int32 FindCharPos;
				ClassName.FindLastChar('.', FindCharPos);
				ClassName.RemoveAt(0, FindCharPos + 1);
				ClassName.RemoveFromStart(TEXT("SM_"));
				FTags::AddKeyValuePair(*ActItr, "SemLog", "Class", ClassName);
			}
		}
	}
	return FReply::Handled();
}

// Set flag attribute depending on the checkbox state
void FSLEdModeToolkit::OnCheckedOverwriteClassNames(ECheckBoxState NewCheckedState)
{
	// TODO now everything will be overwritten
	bOverwriteExistingVisualMaskValues = true;
	//bOverwriteExistingClassNames = (NewCheckedState == ECheckBoxState::Checked);
}

// Set unique mask colors in hexa for the entities
FReply FSLEdModeToolkit::SetVisualMaskValues()
{
	// Keep all colors in a set to ensure uniqueness;
	TSet<FColor> TotalMaskColors;

	// TODO add a min difference between the colors (e.g. return Abs(R-r) > 5 || Abs(G-g) > 5 || Abs(B-b) > 5);
	// Lambda for generating unique colors as hex string
	auto GetUniqueRandomColorLambda = [&TotalMaskColors]()->FString
	{
		// Iterate generating a random color until it is unique (and differs from black)
		for (int32 Idx = 0; Idx < 10; ++Idx)
		{
			FColor RandColor = FColor::MakeRandomColor();
			if (RandColor != FColor::Black && !TotalMaskColors.Contains(RandColor))
			{
				TotalMaskColors.Emplace(RandColor);
				return RandColor.ToHex();
			}
		}
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a unique color, setting as black.."), TEXT(__FUNCTION__), __LINE__);
		return FColor::Black.ToHex();
	};

	// Iterate only static mesh actors
	for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			if (bOverwriteExistingVisualMaskValues)
			{
				// Check if the actor or the component is semantically annotated
				if (FTags::HasKey(*ActItr, "SemLog", "Class"))
				{
					FTags::AddKeyValuePair(*ActItr, "SemLog", "VisMask", GetUniqueRandomColorLambda(), true);
				}
				else if (FTags::HasKey(SMC, "SemLog", "Class"))
				{
					FTags::AddKeyValuePair(SMC, "SemLog", "VisMask", GetUniqueRandomColorLambda(), true);
				}
			}
			else
			{
				// TODO not implemented
				// Load all existing values into the array first
				// then start adding new values with AddUnique
			}
		}

	}
	return FReply::Handled();
}

// Set flag attribute depending on the checkbox state
void FSLEdModeToolkit::OnCheckedOverwriteVisualMaskValues(ECheckBoxState NewCheckedState)
{
	bOverwriteExistingVisualMaskValues = (NewCheckedState == ECheckBoxState::Checked);
}

// Remove all semantic ids
FReply FSLEdModeToolkit::RemoveAllSemanticIds()
{
	FTags::RemoveAllKeyValuePairs(GEditor->GetEditorWorldContext().World(), "SemLog", "Id");
	return FReply::Handled();
}

// Update legacy namings from tags
FReply FSLEdModeToolkit::UpdateLegacyNames()
{
	// What to replace
	const FString SearchText = "LogType";
	// With what
	const FString ReplaceText = "Mobility";

	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		for (auto& T : ActItr->Tags)
		{
			FString TagAsString = T.ToString();
			TagAsString.ReplaceInline(*SearchText, *ReplaceText);
			T = FName(*TagAsString);
		}
		// Iterate actor components
		TArray<UActorComponent*> Comps;
		ActItr->GetComponents<UActorComponent>(Comps);
		for (auto& C : Comps)
		{
			for (auto& T : C->ComponentTags)
			{
				FString TagAsString = T.ToString();
				TagAsString.ReplaceInline(*SearchText, *ReplaceText);
				T = FName(*TagAsString);
			}
		}
	}
	return FReply::Handled();
}

// Remove all tags
FReply FSLEdModeToolkit::RemoveAllTags()
{
	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		ActItr->Tags.Empty();

		// Iterate actor components
		TArray<UActorComponent*> Comps;
		ActItr->GetComponents<UActorComponent>(Comps);
		for (auto& C : Comps)
		{
			C->ComponentTags.Empty();
		}
	}
	return FReply::Handled();
}

// Update semantic visual shape visuals
FReply FSLEdModeToolkit::UpdateSLOverlapShapeColors()
{
	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Iterate actor components
		TArray<USLOverlapShape*> Comps;
		ActItr->GetComponents<USLOverlapShape>(Comps);
		for (auto& C : Comps)
		{
			C->UpdateVisualColor();
		}
	}
	return FReply::Handled();
}