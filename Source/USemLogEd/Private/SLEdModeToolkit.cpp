// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdModeToolkit.h"
#include "SLEdMode.h"
#include "SLEdUtils.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "Engine/Selection.h"
#include "ScopedTransaction.h"
#include "AssetRegistryModule.h"

// SL
#include "Data/SLDataComponent.h"
#include "SLSkeletalDataComponent.h"
#include "Monitors/SLContactBox.h"

// UUtils
#include "Utils/SLTagIO.h"

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
			+ CreateWriteIdsSlot()

			////
			+ CreateRmIdsSlot()

			////
			+ CreateWriteClassNamesSlot()

			////
			+ CreateRmClassNamesSlot()

			////
			+ CreateWriteVisualMasksSlot()

			////
			+ CreateRmVisualMasksSlot()


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
				.Text(LOCTEXT("EnableMaterialsForInstancedStaticMesh", "Enable Materials for Instanced Static Mesh"))
				.IsEnabled(true)
				.OnClicked(this, &FSLEdModeToolkit::EnableMaterialsForInstancedStaticMesh)
			]

			////
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("GenerateSemanticComponent", "Generate Semantic Components"))
				.IsEnabled(true)
				.OnClicked(this, &FSLEdModeToolkit::GenerateSemanticComponents)
			]

			////
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("ShowSemanticData", "Show Semantic Data"))
				.IsEnabled(true)
				.OnClicked(this, &FSLEdModeToolkit::ShowSemanticData)
			]
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
			.Padding(10)
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
			.Padding(10)
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

SVerticalBox::FSlot& FSLEdModeToolkit::CreateWriteIdsSlot()
{
	return 	SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("GenSemIds", "Generate Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("GenSemMapTip", "Generates unique ids for every semantic entity"))
				.OnClicked(this, &FSLEdModeToolkit::OnWriteSemIds)
			];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateRmIdsSlot()
{
	return SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("RmSemIds", "Remove Ids"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmSemIdsTip", "Removes all generated ids"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmSemIds)
			];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateWriteClassNamesSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("WriteClassNames", "Write Class Names"))
		.IsEnabled(true)
		.ToolTipText(LOCTEXT("WriteClassNames", "Writes known class names"))
		.OnClicked(this, &FSLEdModeToolkit::OnWriteClassNames)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateRmClassNamesSlot()
{
	return SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("RmClassNames", "Remove Class Names"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmClassNamesTip", "Removes all class names"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmClassNames)
			];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateWriteVisualMasksSlot()
{
	return SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("WriteVisualMasks", "Write Visual Masks"))
			.IsEnabled(true)
			.ToolTipText(LOCTEXT("WriteVisualMasksTip", "Writes unique visual masks for visual entities"))
			.OnClicked(this, &FSLEdModeToolkit::OnWriteClassNames)
		];
}

SVerticalBox::FSlot& FSLEdModeToolkit::CreateRmVisualMasksSlot()
{
	return SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("RmVisualMasks", "Remove Visual Masks"))
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("RmVisualMasksTip", "Removes all visual masks"))
				.OnClicked(this, &FSLEdModeToolkit::OnRmClassNames)
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
	FSLTagIO::RemoveWorldKVPairs(GEditor->GetEditorWorldContext().World(), "SemLog", "Id");
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
	FSLTagIO::RemoveWorldKVPairs(GEditor->GetEditorWorldContext().World(), "SemLog", "Class");
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
	FSLTagIO::RemoveWorldKVPairs(GEditor->GetEditorWorldContext().World(), "SemLog", "VisMask");
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
	FScopedTransaction Transaction(LOCTEXT("AddSLContactBoxes", "Add contact overlap shapes"));
	//// Iterate only static mesh actors
	//for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	//{
	//	// Continue only if a valid mesh component is available
	//	if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
	//	{
	//		// Ignore if actor is not tagged
	//		if (FTags::HasKey(*ActItr, "SemLog", "Class"))
	//		{
	//			// Continue if no previous components are created
	//			TArray<USLContactBox*> Comps;
	//			ActItr->GetComponents<USLContactBox>(Comps);
	//			//if (Comps.Num() == 0)
	//			//{
	//			//	USLContactBox* Comp = NewObject<USLContactBox>(*ActItr);
	//			//	Comp->RegisterComponent();
	//			//	/*FTransform T;
	//			//	ActItr->AddComponent("USLContactBox", false, T, USLContactBox::StaticClass());*/
	//			//}
	//		}
	//	}
	//}

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
	//// Iterate only static mesh actors
	//for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	//{
	//	// Continue only if a valid mesh component is available
	//	if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
	//	{
	//		//// Ignore if actor is not tagged
	//		//if (FTags::HasKey(*ActItr, "SemLog", "Class"))
	//		//{
	//		//	SMC->SetGenerateOverlapEvents(true);
	//		//}
	//	}
	//}

	return FReply::Handled();
}

// Enable all materials to be used as instanced static meshes
FReply FSLEdModeToolkit::EnableMaterialsForInstancedStaticMesh()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AllAsset;
	AssetRegistryModule.Get().GetAssetsByPath(TEXT("/Game/"), AllAsset, true, false);

	for (FAssetData Data : AllAsset)
	{
		if (Data.AssetClass.ToString().Equals(TEXT("Material")))
		{
			UMaterial* Material = Cast<UMaterial>(Data.GetAsset());
			if (!Material->bUsedWithInstancedStaticMeshes)
			{
				Material->bUsedWithInstancedStaticMeshes = true;
				Data.GetPackage()->MarkPackageDirty();
				UE_LOG(LogTemp, Error, TEXT("%s::%d Material: %s is enabled for instanced static mesh.."), *FString(__func__), __LINE__, *Data.GetPackage()->GetFName().ToString());
			}
		}
	}

	return FReply::Handled();
}

// Generate semantic components for each actor
FReply FSLEdModeToolkit::GenerateSemanticComponents()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	//for (ULevelStreaming* LevelStreaming : World->GetStreamingLevels())
	//{
	//	if (LevelStreaming && LevelStreaming->IsLevelVisible())
	//	{
	//		if (ULevel* Level = LevelStreaming->GetLoadedLevel())
	//		{
	//			// Iterate method 1
	//			for (TActorIterator<AStaticMeshActor> ActorItr(Level->GetWorld()); ActorItr; ++ActorItr)
	//			{
	//				// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
	//				AStaticMeshActor* Mesh = *ActorItr;
	//				Mesh->AddActorLocalOffset(FVector(500, 500, 500));
	//			}
	//			// Iterate method 2
	//			for (AActor* Actor : Level->Actors)
	//			{
	//				// Store quick map of id to actor pointer
	//				if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Actor))
	//				{
	//					AsSMA->AddActorLocalOffset(FVector(1000, 1000, 100));
	//				}
	//			}
	//		}
	//	}
	//}

	return FReply::Handled();
}


// Show the semantic information of the actors
FReply FSLEdModeToolkit::ShowSemanticData()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t\t Act=%s;"),
			*FString(__FUNCTION__), __LINE__, *It->GetName());
	}

	UE_LOG(LogTemp, Error, TEXT("%s::%d **********************"),
		*FString(__FUNCTION__), __LINE__);

	for (ULevelStreaming* LevelStreaming : World->GetStreamingLevels())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Streaming level=%s;"),
			*FString(__FUNCTION__), __LINE__, *LevelStreaming->GetName());
		if (LevelStreaming && LevelStreaming->IsLevelVisible())
		{	
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Is visible=%s;"),
				*FString(__FUNCTION__), __LINE__, *LevelStreaming->GetName());
			if (ULevel* Level = LevelStreaming->GetLoadedLevel())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t Loaded level=%s;"),
					*FString(__FUNCTION__), __LINE__, *Level->GetName());
				for (AActor* Actor : Level->Actors)
				{
					if (Actor)
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Act=%s;"),
							*FString(__FUNCTION__), __LINE__, *Actor->GetName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t Act=nullptr;"),
							*FString(__FUNCTION__), __LINE__);
					}
					
					

					//// Make sure the actor does not have a component already
					//if (Actor->GetComponentByClass(USLDataComponent::StaticClass()))
					//{
					//	//USLDataComponent* SemanticDataComponent = NewObject<USLDataComponent>(USLDataComponent::StaticClass(), Actor);
					//	//UE_LOG(LogTemp, Error, TEXT("%s::%d %s received a new semantic data component (%s).."), *FString(__FUNCTION__), __LINE__, *Actor->GetName(), *SemanticDataComponent->GetName());
					//}
					//else
					//{
					//	//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s already has a semantic data component.."), *FString(__FUNCTION__), __LINE__, *Actor->GetName());
					//}
				}


				UE_LOG(LogTemp, Error, TEXT("%s::%d ----"), *FString(__FUNCTION__), __LINE__);

				// Iterate method 1
				for (TActorIterator<AActor> ActorItr(Level->GetWorld()); ActorItr; ++ActorItr)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Act=%s;"),
						*FString(__FUNCTION__), __LINE__, *ActorItr->GetName());
				}
			}
		}
	}

	return FReply::Handled();
}




// Return true if any actors are selected in the viewport
bool FSLEdModeToolkit::AreActorsSelected()
{
	return GEditor->GetSelectedActors()->Num() != 0;
}

#undef LOCTEXT_NAMESPACE
