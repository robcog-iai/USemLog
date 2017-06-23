#pragma once

#include "CoreMinimal.h"
#include <string>
#include <algorithm>

struct Utils
{
	// Generate random FString
	static FORCEINLINE FString GenerateRandomFString(const uint32 Length)
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
};