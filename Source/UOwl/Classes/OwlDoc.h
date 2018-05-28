// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlNode.h"

/**
*
*/
struct FOwlDoc
{
public:
	// Default constructor
	FOwlDoc() {}

	// Init constructor
	FOwlDoc(const FString& InDeclaration,
		const FOwlEntityDTD& InEntityDefinitions,
		const FOwlNode& InRoot) :
		Declaration(InDeclaration),
		EntityDefinitions(InEntityDefinitions),
		Root(InRoot)
	{}

	// Destructor
	~FOwlDoc() {}

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
	FOwlEntityDTD EntityDefinitions;

	// Root (including namespace declarations as attributes)
	FOwlNode Root;

	// Current state of the indentation
	FString Indent;
};
