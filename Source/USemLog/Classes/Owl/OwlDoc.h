// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Owl.h"
#include "Owl/OwlNode.h"

namespace SLOwl
{
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

		// Init constructor
		FOwlDoc(const FString& InDeclaration,
			const FEntityDTD& InEntityDTD,
			const FOwlNode& InRoot);

		// Destructor
		~FOwlDoc();

		// Return document as string
		FString ToString() const;
	
	private:
		// Declaration ("<?xml version="1.0" encoding="utf-8"?>")
		FString Declaration;

		// Document Type Definition (DTD) for Entity Declaration
		FEntityDTD EntityDTD;

		// Root (including namespace declarations as attributes)
		FOwlNode Root;
	};

} // namespace SLOwl