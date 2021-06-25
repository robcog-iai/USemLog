// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualInfoTextComponent.generated.h"

// Forward declaration
class UTextRenderComponent;
class UMaterial;

/*
* Represents a row in the individual description
*/
USTRUCT()
struct FSLIndividualInfoTextRow
{
	GENERATED_BODY()

	// The text render component
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger"*/)
	UTextRenderComponent* Text;

	// The row number (1 represnts the first row)
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger"*/)
	int32 RowNum = 0;
};


UCLASS(ClassGroup = (SL), meta = (BlueprintSpawnableComponent), DisplayName = "SL Individual Text")
class USLIndividualInfoTextComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualInfoTextComponent();

protected:
	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:
	// Add text row (if key already exists, set values)
	void AddRow(const FString& Key, const FString& Text = "", FColor Color = FColor::White);

	// Set text row value (adds a new row if it does not exist)
	void SetRowValue(const FString& Key, const FString& Text);

	// Set text render color value (adds a new row if it does not exist)
	void SetRowColor(const FString& Key, FColor Color);

	// Set text line value and color
	void SetRowValueAndColor(const FString& Key, const FString& Text, FColor Color);

	// Remove text row (returns false if not found)
	bool ClearRow(const FString& Key);

	// Clear all text rows
	int32 ClearAllRows();

	// Remove all lines that are not in the ignore key array
	int32 ClearAllRowsBut(TSet<FString>& IgnoreKeys);

private:
	// Create a new text render component
	UTextRenderComponent* CreateNewTextRenderComponent(const FString& Name);

	// Move row one step higher
	void MoveRowOneStepHigher(FSLIndividualInfoTextRow& TextRow);

private:
	// Map of the row key to the text row structure
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger"*/)
	TMap<FString, FSLIndividualInfoTextRow> TextRows;

	// Material template used for creating dynamic materials
	UPROPERTY()
	UMaterial* TextMaterial;

	// Text size template value 
	float TextSize;
	
	/* Constants */
	static constexpr auto TextMaterialPath = TEXT("Material'/USemLog/Individuals/M_InfoTextTranslucent.M_InfoTextTranslucent'");
};
