// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualTextComponent.generated.h"

// Forward declaration
class UTextRenderComponent;
class UMaterial;

UCLASS(ClassGroup = (SL), meta = (BlueprintSpawnableComponent), DisplayName = "SL Individual Text")
class USLIndividualTextComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualTextComponent();

protected:
	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:
	// Add text (if key already exists, it applies the changes)
	void AddTextLine(const FString& Key, const FString& Text="", FColor Color = FColor::White);

	// Set text value (adds a new line if it does not exist)
	void SetTextLineValue(const FString& Key, const FString& Text);

	// Set text render color value (adds a new line if it does not exist)
	void SetTextLineColor(const FString& Key, FColor Color);

	// Remove text line (returns false if not found)
	bool RemoveTextLine(const FString& Key);

private:
	// Create a new text render component
	UTextRenderComponent* CreateNewTextLine(const FString& Name);

	// Move line one step higher
	void MoveTextLineOneStepHigher(UTextRenderComponent* TRC);

	// Add text line to the map and update its line order
	void SetTextLineOrder(const FString& Key, UTextRenderComponent* TRC);

private:
	// Material template used for creating dynamic materials
	UPROPERTY()
	UMaterial* TextMaterial;

	// Text info
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<FString, UTextRenderComponent*> TextLines;

	// The text line numbering (first=1 is top, each line goes lower)
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<UTextRenderComponent*, int32> TextLinesOrder;

	// Text size template value 
	float TextLineWorldSize;
	
	/* Constants */
	// Clamp the template text size between these values
	constexpr static float MinClampTextSize = 3.f;
	constexpr static float MaxClampTextSize = 6.f;
	
	constexpr static TCHAR* TextMaterialPath = TEXT("Material'/USemLog/Individuals/M_InfoTextTranslucent.M_InfoTextTranslucent'");
};
