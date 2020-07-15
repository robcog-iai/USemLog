// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualTextComponent.h"
#include "Individuals/SLIndividualVisualAssets.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
USLIndividualTextComponent::USLIndividualTextComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	TextMaterial = Cast<UMaterial>(StaticLoadObject( UMaterial::StaticClass(), NULL, TextMaterialPath, NULL, LOAD_None, NULL));
	TextLineWorldSize = 2.5f;
}

// Called before destroying the object.
void USLIndividualTextComponent::BeginDestroy()
{
	ClearAllTextLines();
	Super::BeginDestroy();
}

// Add text (return false if key already exists)
void USLIndividualTextComponent::AddTextLine(const FString& Key, const FString& Text, FColor Color)
{
	if (UTextRenderComponent** FoundLine = TextLines.Find(Key))
	{
		(*FoundLine)->SetText(FText::FromString(Text));
		(*FoundLine)->SetTextRenderColor(Color);
	}
	else if (UTextRenderComponent* NewLine = CreateNewTextLine("TextLine_" + Key))
	{
		NewLine->SetText(FText::FromString(Text));
		NewLine->SetTextRenderColor(Color);
		SetTextLineOrder(Key, NewLine);
	}
}

// Set text value (creates a new line if it does not exist)
void USLIndividualTextComponent::SetTextLineValue(const FString& Key, const FString& Text)
{
	if (UTextRenderComponent** FoundLine = TextLines.Find(Key))
	{
		(*FoundLine)->SetText(FText::FromString(Text));
	}
	else if (UTextRenderComponent* NewLine = CreateNewTextLine("TextLine_" + Key))
	{
		NewLine->SetText(FText::FromString(Text));
		SetTextLineOrder(Key, NewLine);
	}
}

// Set text render color value (adds a new line if it does not exist)
void USLIndividualTextComponent::SetTextLineColor(const FString& Key, FColor Color)
{
	if (UTextRenderComponent** FoundLine = TextLines.Find(Key))
	{
		(*FoundLine)->SetTextRenderColor(Color);
	}
	else if (UTextRenderComponent* NewLine = CreateNewTextLine("TextLine_" + Key))
	{
		NewLine->SetTextRenderColor(Color);
		SetTextLineOrder(Key, NewLine);
	}
}

// Set text line value and color
void USLIndividualTextComponent::SetTextLineValueAndColor(const FString& Key, const FString& Text, FColor Color)
{
	if (UTextRenderComponent** FoundLine = TextLines.Find(Key))
	{
		(*FoundLine)->SetText(FText::FromString(Text));
		(*FoundLine)->SetTextRenderColor(Color);
	}
	else if (UTextRenderComponent* NewLine = CreateNewTextLine("TextLine_" + Key))
	{
		NewLine->SetText(FText::FromString(Text));
		NewLine->SetTextRenderColor(Color);
		SetTextLineOrder(Key, NewLine);
	}
}

// Remove text line (returns false if not found)
bool USLIndividualTextComponent::RemoveTextLine(const FString& Key)
{
	UTextRenderComponent* RemovedCopy;
	if (TextLines.RemoveAndCopyValue(Key, RemovedCopy))
	{
		int32 RemovedLineOrderCopy;
		if (TextLinesOrder.RemoveAndCopyValue(RemovedCopy, RemovedLineOrderCopy))
		{
			// If the removed line was not the last, move all lower lines one step higher
			if (RemovedLineOrderCopy <= TextLinesOrder.Num())
			{
				for(auto& OrderPair : TextLinesOrder)
				{
					OrderPair.Value--;
					MoveTextLineOneStepHigher(OrderPair.Key);
				}
			}
			return true;
		}
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s's text no line number found for component %s, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName(), *RemovedCopy->GetName());
	}
	return false;
}

// Clear all text lines
void USLIndividualTextComponent::ClearAllTextLines()
{
	for (const auto& Pair : TextLines)
	{
		Pair.Value->ConditionalBeginDestroy();
	}
	TextLines.Empty();
	TextLinesOrder.Empty();
}

// Remove all lines that are not in the ignore key array
void USLIndividualTextComponent::ClearTextLineValues(TArray<FString>& IgnoreKeys)
{
	TArray<FString> KeysToRemove;
	for (const auto& KeyVal : TextLines)
	{
		if (!IgnoreKeys.Contains(KeyVal.Key))
		{
			KeysToRemove.Add(KeyVal.Key);
		}
	}

	for (const auto& Key : KeysToRemove)
	{
		RemoveTextLine(Key);
	}
}

// Create a new text render component
UTextRenderComponent* USLIndividualTextComponent::CreateNewTextLine(const FString& Name)
{
	UTextRenderComponent* TRC = NewObject<UTextRenderComponent>(this);
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

	TRC->SetWorldSize(TextLineWorldSize);
	TRC->SetVisibility(true);
	if (TextMaterial)
	{
		TRC->SetTextMaterial(TextMaterial);
	}
	return TRC;
}

// Move line one step higher
void USLIndividualTextComponent::MoveTextLineOneStepHigher(UTextRenderComponent* TRC)
{
	TRC->AddLocalOffset(FVector(0.f, 0.f, TextLineWorldSize));
}

// Add text line to the map and update its line order
void USLIndividualTextComponent::SetTextLineOrder(const FString& Key, UTextRenderComponent* TRC)
{
	float ZOffset = TextLineWorldSize * TextLinesOrder.Num();
	TRC->SetRelativeLocation(FVector(0.f, 0.f, - ZOffset));
	TextLines.Emplace(Key, TRC);
	int32 OrderValue = TextLinesOrder.Num() + 1;
	TextLinesOrder.Emplace(TRC, OrderValue);
}
