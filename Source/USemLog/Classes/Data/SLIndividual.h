// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLIndividualBase.h"
#include "SLIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USLIndividual : public USLIndividualBase
{
	GENERATED_BODY()

public:
	// Save data to owners tag
	virtual bool SaveToTag(bool bOverwrite = false) override;

	// Load data from owners tag
	virtual bool LoadFromTag(bool bOverwrite = false) override;

	// Set get Id
	void SetId(const FString& InId) { Id = InId; };
	FString GetId() const { return Id; };

	// Set get class
	void SetClass(const FString& InClass) { Class = InClass; };
	FString GetClass() const { return Class; };

protected:
	// Individual unique id
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Id;
	
	// Idividual class
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Class;
};
