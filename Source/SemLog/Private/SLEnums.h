#pragma once

/** Enum indicating the log type */
UENUM(BlueprintType)
enum class ESemLogType : uint8
{
	Dynamic			UMETA(DisplayName = "Dynamic"),
	Static			UMETA(DisplayName = "Static"),
};