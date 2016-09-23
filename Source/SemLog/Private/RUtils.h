#pragma once
#include "SemLogPrivatePCH.h"
#include <string>
#include <algorithm>
#include "RUtils.generated.h"

#define Log(text) UE_LOG(LogTemp, Warning, text);
#define Print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)
#define PrintRed(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red,text)
#define PrintGreen(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)
#define PrintDur(text, duration) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, duration, FColor::White,text)

/** Utils */
USTRUCT()
struct FRUtils
{
GENERATED_USTRUCT_BODY()

	// Default constructor	
	FRUtils()
	{
	};

	// Destructor
	~FRUtils()
	{
	};

	// Generate random FString
	static FORCEINLINE FString GenerateRandomFString(const int32 Length)
	{
		auto RandChar = []() -> char
		{
			const char CharSet[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			const size_t MaxIndex = (sizeof(CharSet) - 1);
			return CharSet[rand() % MaxIndex];
		};
		std::string RandString(Length, 0);
		std::generate_n(RandString.begin(), Length, RandChar);
		// Return as Fstring
		return FString(RandString.c_str());
	}

	// Convert FString to const char*
	static FORCEINLINE const char* FStringToChar(const FString FStr)
	{
		const std::string str = TCHAR_TO_UTF8(*FStr);
		char *cstr = new char[str.length() + 1];
		strcpy(cstr, str.c_str());
		return cstr;

		//std::string str = TCHAR_TO_UTF8(*FStr);
		//char* cstr = (char *)malloc(sizeof(char) * (str.length() + 1));
		//strncpy_s(cstr, str.length(), str.c_str(), str.length());
		//return cstr;
	}

	// Get the enum value to string
	template<typename TEnum>
	static FORCEINLINE FString GetEnumValueToString(const FString& Name, TEnum Value)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
		if (!EnumPtr)
		{
			return FString("Invalid");
		}
		return EnumPtr->GetEnumName((int32)Value);
	};
};
