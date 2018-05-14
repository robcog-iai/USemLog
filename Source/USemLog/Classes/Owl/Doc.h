// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Owl.h"
#include "Owl/Node.h"

namespace SLOwl
{
	/**
	 * 
	 */
	struct FDoc
	{
	public:
		// Default constructor
		FDoc() {}

		// Init constructor
		FDoc(const FString& InDeclaration,
			const FEntityDTD& InEntityDefinitions,
			const FNode& InRoot) :
			Declaration(InDeclaration),
			EntityDefinitions(InEntityDefinitions),
			Root(InRoot)
		{}

		// Destructor
		~FDoc() {}

		// Return document as string
		FString ToString()
		{
			FString DocStr;
			if (!Declaration.IsEmpty())
			{
				DocStr += Declaration + TEXT("\n\n");
			}
			DocStr += EntityDefinitions.ToString();
			DocStr += Root.ToString(Indent);
			return DocStr;
		}

		// Declaration ("<?xml version="1.0" encoding="utf-8"?>")
		FString Declaration;

		// Document Type Definition (DTD) for Entity Declaration
		FEntityDTD EntityDefinitions;

		// Root (including namespace declarations as attributes)
		FNode Root;

		// Current state of the indentation
		FString Indent;
	};

} // namespace SLOwl