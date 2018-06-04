// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl.h"

/**
* Events document template types
*/
UENUM(BlueprintType)
enum class EEventsTemplateType : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	IAI			  			UMETA(DisplayName = "IAI"),
};

/**
 * 
 */
struct USEMLOG_API FSLEventsWriter
{
public:
	// Default constructor
	FSLEventsWriter();

	// Write semantic map to file
	bool WriteToFile(EEventsTemplateType TemplateType = EEventsTemplateType::NONE,
		const FString& InDirectory = TEXT("SemLog"),
		const FString& InFilename = TEXT("SemanticMap"));

private:
	// Create events doc template
	TSharedPtr<FOwlEvents> CreateEventsDocTemplate(EEventsTemplateType TemplateType);
};
