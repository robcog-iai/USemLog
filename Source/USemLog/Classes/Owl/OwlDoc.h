// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

namespace SLOwl
{
	// Indent step
	static const FString INDENT_STEP = TEXT("\t");

	/**
	 * 
	 */
	class FOwlDoc
	{
	public:
		// Current state of the indentation
		static FString Indent;

		// Default constructor
		FOwlDoc();

		// Destructor
		~FOwlDoc();

		// Return document as string
		FString ToString() const;
	};

} // namespace SLOwl