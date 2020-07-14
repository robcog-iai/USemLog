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

	TextLineScreenSize = 5.f;
	//ScaleText1 = 1.f;
	//ScaleText2 = 0.8f;
	//ScaleText3 = 0.8f;

	//static USLIndividualVisualAssets* AssetsContainer = Cast<USLIndividualVisualAssets>(StaticLoadObject(
	//	USLIndividualVisualAssets::StaticClass(), NULL, AssetContainerPath,
	//	NULL, LOAD_None, NULL));

	//Text1 = CreateTextComponentSubobject("FirstText", AssetsContainer);
	//Text2 = CreateTextComponentSubobject("SecondText", AssetsContainer);
	//Text2->SetTextRenderColor(FColor::Red);
	//Text3 = CreateTextComponentSubobject("ThirdText", AssetsContainer);
	//Text3->SetTextRenderColor(FColor::White);
}

// Called before destroying the object.
void USLIndividualTextComponent::BeginDestroy()
{
	for (const auto& Pair : TextLines)
	{
		Pair.Value->ConditionalBeginDestroy();
	}
	Super::BeginDestroy();
}

// Add text (return false if key already exists)
void USLIndividualTextComponent::AddNewTextLine(const FString& Key, const FString& Text, FColor Color)
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
		AddOrderedTextLine(Key, NewLine);
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
		AddOrderedTextLine(Key, NewLine);
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
		AddOrderedTextLine(Key, NewLine);
	}
}

// Remove text line (returns false if not found)
bool USLIndividualTextComponent::RemoveTextLine(const FString& Key)
{
	UTextRenderComponent* RemovedCopy;
	if (TextLines.RemoveAndCopyValue(Key, RemovedCopy))
	{
		int32 RemovedLineOrderCopy;
		if (TextLineOrder.RemoveAndCopyValue(RemovedCopy, RemovedLineOrderCopy))
		{
			// If the removed line was not the last, move all lower lines one step higher
			if (RemovedLineOrderCopy <= TextLineOrder.Num())
			{
				for(auto& OrderPair : TextLineOrder)
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

// Recalculate the size of the text
void USLIndividualTextComponent::ResizeText()
{
	//if (GetOwner())
	//{
	//	FVector BoundsOrigin;
	//	FVector BoxExtent;
	//	GetOwner()->GetActorBounds(false, BoundsOrigin, BoxExtent);
	//	TextScreenSize = FMath::Clamp(BoxExtent.Size() / 30.f, MinClampTextSize, MaxClampTextSize);
	//}

	//const float FirstSize = TextScreenSize * ScaleText1;
	//const float SecondSize = TextScreenSize * ScaleText2;
	//const float ThirdSize = TextScreenSize * ScaleText3;

	//const float FirstRelLoc = (FirstSize + SecondSize + ThirdSize);
	//const float SecondRelLoc = (SecondSize + ThirdSize);
	//const float ThirdRelLoc = ThirdSize;

	//if (Text1 && Text1->IsValidLowLevel() && !Text1->IsPendingKill())
	//{
	//	Text1->SetWorldSize(FirstSize);
	//	Text1->SetRelativeLocation(FVector(0.f, 0.f, FirstRelLoc));
	//}

	//if (Text2 && Text2->IsValidLowLevel() && !Text2->IsPendingKill())
	//{
	//	Text2->SetWorldSize(SecondSize);
	//	Text2->SetRelativeLocation(FVector(0.f, 0.f, SecondRelLoc));
	//}

	//if (Text3 && Text3->IsValidLowLevel() && !Text3->IsPendingKill())
	//{
	//	Text3->SetWorldSize(ThirdSize);
	//	Text3->SetRelativeLocation(FVector(0.f, 0.f, ThirdRelLoc));
	//}
}

// Point text towards the camera
bool USLIndividualTextComponent::PointToCamera()
{
	// True if we are in the editor (this is still true when using Play In Editor). You may want to use GWorld->HasBegunPlay in that case)	
	if (GIsEditor)
	{
		// TODO check if standalone e.g. 
		//if (GIsPlayInEditorWorld) 

#if WITH_EDITOR
		for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
		{
			if (LevelVC && LevelVC->IsPerspective())
			{
				SetWorldRotation(LevelVC->GetViewRotation() + FRotator(180.f, 0.f, 180.f));
				break;
			}
		}
#endif //WITH_EDITOR
	}
	else if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		//PC->PlayerCameraManager; // This will not call or yield anything
	}

	return false;
}

// Create a new text render component
UTextRenderComponent* USLIndividualTextComponent::CreateNewTextLine(const FString& Name)
{
	UTextRenderComponent* TRC = NewObject<UTextRenderComponent>(this);
	TRC->PrimaryComponentTick.bCanEverTick = false;
	TRC->SetHorizontalAlignment(EHTA_Center);
	TRC->SetVerticalAlignment(EVRTA_TextBottom);	
	TRC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	if (TextMaterial)
	{
		TRC->SetTextMaterial(TextMaterial);
	}
	return TRC;
}

// Move line one step higher
void USLIndividualTextComponent::MoveTextLineOneStepHigher(UTextRenderComponent* TRC)
{
	TRC->AddLocalOffset(FVector(0.f, 0.f, TextLineScreenSize));
}

// Add text line to the map and update its line order
void USLIndividualTextComponent::AddOrderedTextLine(const FString& Key, UTextRenderComponent* TRC)
{
	TRC->SetRelativeLocation(FVector(0.f, 0.f, TextLineScreenSize * TextLineOrder.Num()));
	TextLines.Emplace(Key, TRC);
	int32 OrderValue = TextLineOrder.Num() + 1;
	TextLineOrder.Emplace(TRC, OrderValue);
}
