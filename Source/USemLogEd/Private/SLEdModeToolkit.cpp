// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
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
#include "SLContactBox.h"
#include "SLSkeletalDataComponent.h"
#include "Ids.h"
#include "Tags.h"
#include "ScopedTransaction.h"
#include "Animation/SkeletalMeshActor.h"

#define LOCTEXT_NAMESPACE "FSemLogEdModeToolkit"

// Ctor
FSLEdModeToolkit::FSLEdModeToolkit()
{
	bOverwriteSemanticMap = true;
	bOverwriteExistingClassNames = true;
	bOverwriteVisualMaskValues = true;
	bGenerateRandomVisualMasks = true;
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
						.Text(LOCTEXT("AnnotateConstraints", "Annotate Constraints"))
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
								.Text(LOCTEXT("GenerateVisualMasks", "Generate Visual Masks"))
							.IsEnabled(true)
							.OnClicked(this, &FSLEdModeToolkit::GenerateVisualMasks)
							]
						+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
								.ToolTipText(LOCTEXT("GenerateVisualMasks_Overwrite", "Overwrite"))
							.IsChecked(ECheckBoxState::Checked)
							.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteVisualMasks)
							]
						+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
								.ToolTipText(LOCTEXT("GenerateRandomVisualMasks_Overwrite", "Random(checked) / Incremental(unchecked)"))
							.IsChecked(ECheckBoxState::Checked)
							.OnCheckStateChanged(this, &FSLEdModeToolkit::OnCheckedOverwriteGenerateRandomVisualMasks)
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
						.Text(LOCTEXT("AddSLContactBoxColors", "Add Semantic Overlap Shapes"))
					.IsEnabled(true)
					.OnClicked(this, &FSLEdModeToolkit::AddSLContactBoxes)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("UpdateSLContactBoxColors", "Update Semantic Overlap Shape Visuals"))
						.IsEnabled(true)
						.OnClicked(this, &FSLEdModeToolkit::UpdateSLContactBoxColors)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("EnableAllOverlaps", "Enable All Overlaps"))
					.IsEnabled(true)
					.OnClicked(this, &FSLEdModeToolkit::EnableAllOverlaps)
					]
				////
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("TagSelectedAsContainers", "Tag Selected As Containers"))
					.IsEnabled(true)
					.OnClicked(this, &FSLEdModeToolkit::TagSelectedAsContainers)
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
	FScopedTransaction Transaction(LOCTEXT("GenerateNewSemanticIds", "Generate new Ids"));
	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		int32 TagIndex = FTags::GetTagTypeIndex(*ActItr, "SemLog");
		if (TagIndex != INDEX_NONE)
		{
			FTags::AddKeyValuePair(ActItr->Tags[TagIndex], "Id", FIds::NewGuidInBase64Url(), true, *ActItr);
		}

		// Check component tags as well
		for (const auto& CompItr : ActItr->GetComponents())
		{
			int32 CompTagIndex = FTags::GetTagTypeIndex(CompItr, "SemLog");
			if (CompTagIndex != INDEX_NONE)
			{
				FTags::AddKeyValuePair(CompItr->ComponentTags[CompTagIndex], "Id", FIds::NewGuidInBase64(), true, CompItr);
			}
		}
	}
	return FReply::Handled();
}

// Generate semantic ids for constraints
FReply FSLEdModeToolkit::SemanticallyAnnotateConstraints()
{
	FScopedTransaction Transaction(LOCTEXT("SemanticallyAnnotateConstraints", "Semantically annotated constraints"));
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
	FScopedTransaction Transaction(LOCTEXT("SetClassNamesToDefault", "Set class names to default values"));
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
					FTags::AddKeyValuePair(SMC, "SemLog", "Class", ClassName, true);
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
				FTags::AddKeyValuePair(*ActItr, "SemLog", "Class", ClassName, true);
			}
		}
	}
	return FReply::Handled();
}

// Set flag attribute depending on the checkbox state
void FSLEdModeToolkit::OnCheckedOverwriteClassNames(ECheckBoxState NewCheckedState)
{
	// TODO now everything will be overwritten
	bOverwriteVisualMaskValues = true;
	//bOverwriteExistingClassNames = (NewCheckedState == ECheckBoxState::Checked);
}

// Set unique mask colors in hex for the entities (random or incremental)
FReply FSLEdModeToolkit::GenerateVisualMasks()
{
	FScopedTransaction Transaction(LOCTEXT("GenerateVisualMasks", "Generate Visual Masks"));
	if (bGenerateRandomVisualMasks)
	{
		return FSLEdModeToolkit::GenerateVisualMasksRand();
	}
	else
	{
		return FSLEdModeToolkit::GenerateVisualMasksInc();
	}
}

// Set unique mask colors in hex for the entities (use random colors)
FReply FSLEdModeToolkit::GenerateVisualMasksRand()
{
	// Lambda for generating unique colors as hex string
	auto GenerateUniqueColorLambda = [](const uint8 Tolerance, const int32 NrOfTrials, TArray<FColor>& ConsumedColors)->FString
	{
		// Iterate generating a random color until it is unique (and differs from black)
		for (int32 Idx = 0; Idx < NrOfTrials; ++Idx)
		{
			FColor RandColor = FColor::MakeRandomColor();
			// Find by predicate lambda
			auto AlmostEqualPredicate = [RandColor, Tolerance](const FColor& Item)
			{
				return FMath::Abs(RandColor.R - Item.R) <= Tolerance
					&& FMath::Abs(RandColor.G - Item.G) <= Tolerance
					&& FMath::Abs(RandColor.B - Item.B) <= Tolerance;
			};

			// Continue if the random color is not similar to black
			if (!AlmostEqualPredicate(FColor::Black))
			{
				// Cache image if there is no other similar stored
				if (FColor* SemColor = ConsumedColors.FindByPredicate(AlmostEqualPredicate))
				{
					// Conflict
					//UE_LOG(LogTemp, Warning, TEXT("\t\t\t%s::%d Conflict between RandColor=%s; and SemColor=%s; Trial=%d"),
					//	*FString(__func__), __LINE__, *RandColor.ToString(), *SemColor->ToString(), Idx);
				}
				else
				{
					// Different color found
					ConsumedColors.Emplace(RandColor);
					return RandColor.ToHex();
				}
			}
		}
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a unique color, saving as black.."), *FString(__func__), __LINE__);
		return FColor::Black.ToHex();
	};

	const uint8 Tolerance = 27;
	const int32 NrOfTrials = 1000;
	TArray<FColor> ConsumedColors;

	// Iterate only static mesh actors
	for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			if (bOverwriteVisualMaskValues)
			{
				// Check if the actor or the component is semantically annotated
				if (FTags::HasKey(*ActItr, "SemLog", "Class"))
				{
					FTags::AddKeyValuePair(*ActItr, "SemLog", "VisMask", GenerateUniqueColorLambda(Tolerance, NrOfTrials, ConsumedColors), true);
				}
				else if (FTags::HasKey(SMC, "SemLog", "Class"))
				{
					FTags::AddKeyValuePair(SMC, "SemLog", "VisMask", GenerateUniqueColorLambda(Tolerance, NrOfTrials, ConsumedColors), true);
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
	
	// Iterate skeletal data components
	for (TObjectIterator<USLSkeletalDataComponent> ObjItr; ObjItr; ++ObjItr)
	{
		// Valid if its parent is a skeletal mesh component
		if (Cast<USkeletalMeshComponent>(ObjItr->GetAttachParent()))
		{
			if (bOverwriteVisualMaskValues)
			{
				for (auto& Pair : ObjItr->SemanticBonesData)
				{
					// Check if data is set (it has a semantic class)
					if (Pair.Value.IsSet())
					{
						Pair.Value.VisualMask = GenerateUniqueColorLambda(Tolerance, NrOfTrials, ConsumedColors);

						// Add the mask to the map used at runtime as well
						if (ObjItr->AllBonesData.Contains(Pair.Key))
						{
							ObjItr->AllBonesData[Pair.Key].VisualMask = Pair.Value.VisualMask;
						}
						else
						{
							// This should not happen, the two maps should be synced
							UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot fine bone %s, maps are not synced.."), 
								*FString(__func__), __LINE__, *Pair.Key.ToString());
						}
					}
				}
			}
			else 
			{
				// Not implemented
			}
		}
	}
	
	return FReply::Handled();
}

// Set unique mask colors in hex for the entities (use incremental colors)
FReply FSLEdModeToolkit::GenerateVisualMasksInc()
{
	auto GenerateUniqueColorLambda = [](const uint8 Step, FColor& ColorIdx)->FString
	{
		const uint8 StepFrom255 = 255 - Step;
		if (ColorIdx.B > StepFrom255)
		{
			ColorIdx.B = ColorIdx.B - StepFrom255;
			if (ColorIdx.G > StepFrom255)
			{
				ColorIdx.G = ColorIdx.G - StepFrom255;
				if (ColorIdx.R > StepFrom255)
				{
					ColorIdx = FColor::White;
					UE_LOG(LogTemp, Error, TEXT("%s::%d Reached the maximum possible color values.."), *FString(__func__), __LINE__);
					return FColor::Black.ToHex();;
				}
				else
				{
					ColorIdx.R += Step;
					return ColorIdx.ToHex();;
				}
			}
			else
			{
				ColorIdx.G += Step;
				return ColorIdx.ToHex();;
			}
		}
		else
		{
			ColorIdx.B += Step;
			return ColorIdx.ToHex();;
		}
	};

	// Generated colors params
	const uint8 Tolerance = 27;
	FColor CIdx(0, 0, 0, 255);

	// Lambda to shuffle the unqiue colors array
	auto ArrayShuffleLambda = [](const TArray<FString>& Colors)
	{
		int32 LastIndex = Colors.Num() - 1;
		for (int32 i = 0; i < LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(0, LastIndex);
			if (i != Index)
			{
				const_cast<TArray<FString>*>(&Colors)->Swap(i, Index);
			}
		}
	};

	// Add all possible colors to an array and shuffle them (avoid having similar color shades next to each other)
	TArray<FString> UniqueColors;
	bool bIsColorBlack = false;
	while (!bIsColorBlack)
	{
		FString UC = GenerateUniqueColorLambda(Tolerance, CIdx);
		if (UC.Equals(FColor::Black.ToHex()))
		{
			bIsColorBlack = true;
		}
		else
		{
			UniqueColors.Add(UC);
		}
	}

	// Shuffle the array
	ArrayShuffleLambda(UniqueColors);
	UE_LOG(LogTemp, Warning, TEXT("%s::%d UniqueColorsNum() = %d"), *FString(__func__), __LINE__, UniqueColors.Num());

	// Iterate only static mesh actors
	uint32 UsedColors = 0;
	for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			if (bOverwriteVisualMaskValues)
			{
				// Check if the actor or the component is semantically annotated
				if (FTags::HasKey(*ActItr, "SemLog", "Class"))
				{
					if (UniqueColors.Num() == 0)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d [Actor] ActItr=%s; No more unique colors, saving as black"),
							*FString(__func__), __LINE__, *ActItr->GetName());
						FTags::AddKeyValuePair(*ActItr, "SemLog", "VisMask", FColor::Black.ToHex(), true);
					}
					else
					{
						const FString ColorStr = UniqueColors.Pop(false);
						FTags::AddKeyValuePair(*ActItr, "SemLog", "VisMask", ColorStr, true);
						UsedColors++;
						UE_LOG(LogTemp, Warning, TEXT("%s::%d [Actor] ActItr=%s; Color=%s; \t\t\t\t [%d/%d] "),
							*FString(__func__), __LINE__, *ActItr->GetName(), *ColorStr, UsedColors, UniqueColors.Num());
					}
				}
				else if (FTags::HasKey(SMC, "SemLog", "Class"))
				{
					if (UniqueColors.Num() == 0)
					{
						FTags::AddKeyValuePair(SMC, "SemLog", "VisMask", FColor::Black.ToHex(), true);
						UE_LOG(LogTemp, Error, TEXT("%s::%d [Comp] Owner=%s; No more unique colors, saving as black"),
							*FString(__func__), __LINE__, *ActItr->GetName());
					}
					else
					{
						const FString ColorStr = UniqueColors.Pop(false);
						FTags::AddKeyValuePair(SMC, "SemLog", "VisMask", ColorStr, true);
						UsedColors++;
						UE_LOG(LogTemp, Warning, TEXT("%s::%d [Comp] Owner=%s; Color=%s; \t\t\t\t [%d/%d] "),
							*FString(__func__), __LINE__, *ActItr->GetName(), *ColorStr, UsedColors, UniqueColors.Num());
					}
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
	
	// Iterate skeletal data components
	for (TObjectIterator<USLSkeletalDataComponent> ObjItr; ObjItr; ++ObjItr)
	{
		// Valid if its parent is a skeletal mesh component
		if (Cast<USkeletalMeshComponent>(ObjItr->GetAttachParent()))
		{
			if (bOverwriteVisualMaskValues)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d [Skel] Owner=%s;"), *FString(__func__), __LINE__, *ObjItr->GetOwner()->GetName());
				
				for (auto& Pair : ObjItr->SemanticBonesData)
				{
					// Check if data is set (it has a semantic class)
					if (Pair.Value.IsSet())
					{
						if (UniqueColors.Num() == 0)
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d \t Class=%s; No more unique colors, saving as black"),
								*FString(__func__), __LINE__, *Pair.Value.Class);
							Pair.Value.VisualMask = FColor::Black.ToHex();
						}
						else
						{
							const FString ColorStr = UniqueColors.Pop(false);
							Pair.Value.VisualMask = ColorStr;
							UsedColors++;
							UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Class=%s; Color=%s; \t\t\t\t [%d/%d]"),
								*FString(__func__), __LINE__, *Pair.Value.Class, *ColorStr, UsedColors, UniqueColors.Num());
						}

						// Add the mask to the map used at runtime as well
						if (ObjItr->AllBonesData.Contains(Pair.Key))
						{
							ObjItr->AllBonesData[Pair.Key].VisualMask = Pair.Value.VisualMask;
						}
						else
						{
							// This should not happen, the two maps should be synced
							UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot find bone %s, maps are not synced.."),
								*FString(__func__), __LINE__, *Pair.Key.ToString());
						}
					}
				}
			}
			else
			{
				// Not implemented
			}
		}
	}
	return FReply::Handled();
}

// Set flag attribute depending on the checkbox state
void FSLEdModeToolkit::OnCheckedOverwriteVisualMasks(ECheckBoxState NewCheckedState)
{
	//bOverwriteVisualMaskValues = (NewCheckedState == ECheckBoxState::Checked);
	bOverwriteVisualMaskValues = true;
}

// Set flag attribute depending on the checkbox state
void FSLEdModeToolkit::OnCheckedOverwriteGenerateRandomVisualMasks(ECheckBoxState NewCheckedState)
{
	bGenerateRandomVisualMasks = (NewCheckedState == ECheckBoxState::Checked);
}

// Remove all semantic ids
FReply FSLEdModeToolkit::RemoveAllSemanticIds()
{
	FScopedTransaction Transaction(LOCTEXT("RemoveAllSemanticIds", "Remove all semantic Ids"));
	FTags::RemoveAllKeyValuePairs(GEditor->GetEditorWorldContext().World(), "SemLog", "Id");
	return FReply::Handled();
}

// Update legacy namings from tags
FReply FSLEdModeToolkit::UpdateLegacyNames()
{
	FScopedTransaction Transaction(LOCTEXT("UpdateLegacyNames", "Update legacy names"));
	// What to replace
	const FString SearchText = "LogType";
	// With what
	const FString ReplaceText = "Mobility";

	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		ActItr->Modify();
		for (auto& Tag : ActItr->Tags)
		{
			FString TagAsString = Tag.ToString();
			if (TagAsString.ReplaceInline(*SearchText, *ReplaceText) > 0)
			{
				Tag = FName(*TagAsString);
			}
		}
		// Iterate actor components
		TArray<UActorComponent*> Comps;
		ActItr->GetComponents<UActorComponent>(Comps);
		for (auto& C : Comps)
		{
			for (auto& Tag : C->ComponentTags)
			{
				FString TagAsString = Tag.ToString();
				if (TagAsString.ReplaceInline(*SearchText, *ReplaceText) > 0)
				{
					Tag = FName(*TagAsString);
				}
			}
		}
	}
	return FReply::Handled();
}

// Remove all tags
FReply FSLEdModeToolkit::RemoveAllTags()
{
	FScopedTransaction Transaction(LOCTEXT("RemoveAllTags", "Remove all tags"));
	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		ActItr->Modify();
		if (ActItr->Tags.Num() > 0)
		{
			ActItr->Tags.Empty();
		}

		// Iterate actor components
		TArray<UActorComponent*> Comps;
		ActItr->GetComponents<UActorComponent>(Comps);
		for (auto& C : Comps)
		{
			if (C->ComponentTags.Num() > 0)
			{
				C->ComponentTags.Empty();
			}
		}
	}
	return FReply::Handled();
}

// Add semantic overlap shapes
FReply FSLEdModeToolkit::AddSLContactBoxes()
{
	FScopedTransaction Transaction(LOCTEXT("AddSLContactBoxs", "Add contact overlap shapes"));
	// Iterate only static mesh actors
	for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			// Ignore if actor is not tagged
			if (FTags::HasKey(*ActItr, "SemLog", "Class"))
			{
				// Continue if no previous components are created
				TArray<USLContactBox*> Comps;
				ActItr->GetComponents<USLContactBox>(Comps);
				//if (Comps.Num() == 0)
				//{
				//	USLContactBox* Comp = NewObject<USLContactBox>(*ActItr);
				//	Comp->RegisterComponent();
				//	/*FTransform T;
				//	ActItr->AddComponent("USLContactBox", false, T, USLContactBox::StaticClass());*/
				//}
			}
		}
	}

	return FReply::Handled();
}

// Update semantic visual shape visuals
FReply FSLEdModeToolkit::UpdateSLContactBoxColors()
{
	for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Iterate actor components
		TArray<USLContactBox*> Comps;
		ActItr->GetComponents<USLContactBox>(Comps);
		for (auto& C : Comps)
		{
			C->UpdateVisualColor();
		}
	}
	return FReply::Handled();
}

// Enable all overlaps
FReply FSLEdModeToolkit::EnableAllOverlaps()
{
	FScopedTransaction Transaction(LOCTEXT("EnableAllOverlaps", "Enable all overlaps"));
	// Iterate only static mesh actors
	for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			// Ignore if actor is not tagged
			if (FTags::HasKey(*ActItr, "SemLog", "Class"))
			{
				SMC->SetGenerateOverlapEvents(true);
			}
		}
	}

	return FReply::Handled();
}

// Enable all overlaps
FReply FSLEdModeToolkit::TagSelectedAsContainers()
{
	FScopedTransaction Transaction(LOCTEXT("TagSelectedAsContainer", "Tag Selected As Containers"));

	TArray<AStaticMeshActor*> SelectedSMAs;
	GEditor->GetSelectedActors()->GetSelectedObjects(SelectedSMAs);

	// Iterate only static mesh actors
	for (auto& ActItr : SelectedSMAs)
	{
		// Ignore if actor is not tagged
		if (FTags::HasKey(ActItr, "SemLog", "Class"))
		{
			if (!FTags::HasKey(ActItr, "SemLog", "Container"))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s tagged as container.."), 
					*FString(__func__), __LINE__, *ActItr->GetName());
				FTags::AddKeyValuePair(ActItr, "SemLog", "Container", "True");
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is already tagged as container.."), 
					*FString(__func__), __LINE__, *ActItr->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no Class tag, skipping.."), 
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
