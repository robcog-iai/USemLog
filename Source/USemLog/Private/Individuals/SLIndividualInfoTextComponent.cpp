// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualInfoTextComponent.h"
#include "Individuals/SLIndividualVisualAssets.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
USLIndividualInfoTextComponent::USLIndividualInfoTextComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	TextMaterial = Cast<UMaterial>(StaticLoadObject( UMaterial::StaticClass(), NULL, TextMaterialPath, NULL, LOAD_None, NULL));
	TextSize = 1.25f;
}

// Called before destroying the object.
void USLIndividualInfoTextComponent::BeginDestroy()
{
	ClearAllRows();
	Super::BeginDestroy();
}

// Add text row (if key already exists, set values)
void USLIndividualInfoTextComponent::AddRow(const FString& Key, const FString& Text, FColor Color)
{
	if (FSLIndividualInfoTextRow* FoundRow = TextRows.Find(Key))
	{
		FoundRow->Text->SetText(FText::FromString(Text));
		FoundRow->Text->SetTextRenderColor(Color);
	}
	else if(UTextRenderComponent* TRC = CreateNewTextRenderComponent("TextRow_" + Key))
	{
		FSLIndividualInfoTextRow NewRow;
		NewRow.Text = TRC;
		NewRow.Text->SetText(FText::FromString(Text));
		NewRow.Text->SetTextRenderColor(Color);
		NewRow.RowNum = TextRows.Num() + 1;
		NewRow.Text->SetRelativeLocation(FVector(0.f, 0.f, - (TextSize * NewRow.RowNum)));
		TextRows.Emplace(Key, NewRow);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's text component could not create a new text render component, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
}

// Set the row text value
void USLIndividualInfoTextComponent::SetRowValue(const FString& Key, const FString& Text)
{
	if (FSLIndividualInfoTextRow* FoundRow = TextRows.Find(Key))
	{
		FoundRow->Text->SetText(FText::FromString(Text));
	}
	else if (UTextRenderComponent* TRC = CreateNewTextRenderComponent("TextRow_" + Key))
	{
		FSLIndividualInfoTextRow NewRow;
		NewRow.Text = TRC;
		NewRow.Text->SetText(FText::FromString(Text));
		NewRow.RowNum = TextRows.Num() + 1;
		NewRow.Text->SetRelativeLocation(FVector(0.f, 0.f, -(TextSize * NewRow.RowNum)));
		TextRows.Emplace(Key, NewRow);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's text component could not create a new text render component, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
}

// Set the row color
void USLIndividualInfoTextComponent::SetRowColor(const FString& Key, FColor Color)
{
	if (FSLIndividualInfoTextRow* FoundRow = TextRows.Find(Key))
	{
		FoundRow->Text->SetTextRenderColor(Color);
	}
	else if (UTextRenderComponent* TRC = CreateNewTextRenderComponent("TextRow_" + Key))
	{
		FSLIndividualInfoTextRow NewRow;
		NewRow.Text = TRC;
		NewRow.Text->SetTextRenderColor(Color);
		NewRow.RowNum = TextRows.Num() + 1;
		NewRow.Text->SetRelativeLocation(FVector(0.f, 0.f, -(TextSize * NewRow.RowNum)));
		TextRows.Emplace(Key, NewRow);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's text component could not create a new text render component, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
}

// Set the row text value and color
void USLIndividualInfoTextComponent::SetRowValueAndColor(const FString& Key, const FString& Text, FColor Color)
{
	AddRow(Key, Text, Color);
}

// Remove text row from the component
bool USLIndividualInfoTextComponent::ClearRow(const FString& Key)
{
	if (FSLIndividualInfoTextRow* FoundRow = TextRows.Find(Key))
	{
		// Move below located rows one step higher
		if (FoundRow->RowNum <= TextRows.Num())
		{
			for (auto& KeyVal : TextRows)
			{
				if (KeyVal.Value.RowNum < FoundRow->RowNum)
				{
					MoveRowOneStepHigher(KeyVal.Value);
				}
			}
		}
		FoundRow->Text->ConditionalBeginDestroy();
		TextRows.FindAndRemoveChecked(Key);
		//TextRows.Remove(Key);
		return true;
	}
	return false;

	//FSLIndividualInfoTextRow RemovedRow;
	//if (TextRows.RemoveAndCopyValue(Key, RemovedRow))
	//{
	//	// Move below located rows one step higher
	//	if (RemovedRow.RowNum <= TextRows.Num())
	//	{
	//		for (auto& KeyVal : TextRows)
	//		{
	//			if (KeyVal.Value.RowNum < RemovedRow.RowNum)
	//			{
	//				MoveRowOneStepHigher(KeyVal.Value);
	//			}
	//		}
	//	}
	//	RemovedRow.Text->ConditionalBeginDestroy();
	//	return true;
	//}
	//return false;
}

// Remove all text rows
int32 USLIndividualInfoTextComponent::ClearAllRows()
{
	int32 Num = 0;
	for (auto& KeyVal : TextRows)
	{
		KeyVal.Value.Text->ConditionalBeginDestroy();
		//KeyVal.Value.Text->DestroyComponent();
		Num++;
	}
	TextRows.Empty();
	return Num;
}

// Remove all text rows but the given ignore keys
int32 USLIndividualInfoTextComponent::ClearAllRowsBut(TSet<FString>& IgnoreKeys)
{
	int32 Num = 0;

	// Cache the keys which needs to be removed (cannot remove from map during the iteration)
	TArray<FString> KeysToRemove;
	for (const auto& KeyVal : TextRows)
	{
		if (!IgnoreKeys.Contains(KeyVal.Key))
		{
			KeysToRemove.Add(KeyVal.Key);
		}
	}

	for (const auto& Key : KeysToRemove)
	{
		if (ClearRow(Key))
		{
			Num++;
		}
	}
	return Num;
}

// Create a new text render component
UTextRenderComponent* USLIndividualInfoTextComponent::CreateNewTextRenderComponent(const FString& Name)
{
	UTextRenderComponent* TRC = NewObject<UTextRenderComponent>(this, FName(*Name));
	TRC->RegisterComponent();
	TRC->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	TRC->PrimaryComponentTick.bCanEverTick = false;
	TRC->SetMobility(EComponentMobility::Movable);
	TRC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TRC->bSelectable = false;
	TRC->SetGenerateOverlapEvents(false);
	TRC->SetCanEverAffectNavigation(false);
	TRC->bCastDynamicShadow = false;
	TRC->bCastStaticShadow = false;
	TRC->bAffectDistanceFieldLighting = false;
	TRC->bAffectDynamicIndirectLighting = false;
	TRC->SetHorizontalAlignment(EHTA_Center);
	TRC->SetVerticalAlignment(EVRTA_TextBottom);

	TRC->SetWorldSize(TextSize);
	TRC->SetVisibility(true);
	if (TextMaterial)
	{
		TRC->SetTextMaterial(TextMaterial);
	}
	return TRC;
}

// Move row location one step higher
void USLIndividualInfoTextComponent::MoveRowOneStepHigher(FSLIndividualInfoTextRow& TextRow)
{
	TextRow.RowNum--;
	TextRow.Text->AddLocalOffset(FVector(0.f, 0.f, TextSize));
}

