// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SlateBasics.h"

class FSLEdStyle
{
public:
	// Create style singleton
	static void Initialize();

	// Destroy singleton
	static void Shutdown();

	// Get reference to the style singleton
	static const ISlateStyle& Get();
	
	// Reloads all texture resources from disk
	static void ReloadTextures();

	// Style name
	static FName GetStyleSetName();

private:
	// Returns a new style
	static TSharedRef<class FSlateStyleSet> Create();

private:
	// Style set singletone instance
	static TSharedPtr<class FSlateStyleSet> StyleSetInstance;
};
