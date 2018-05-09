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
	class FDoc
	{
	public:
		// Default constructor
		FDoc();

		// Init constructor
		FDoc(const FString& InDeclaration,
			const FEntityDTD& InEntityDTD,
			const FNode& InRoot);

		// Destructor
		~FDoc();

		// Get doc declaration
		FPrefixName GetDeclaration() const { return Declaration; }

		// Set doc declaration
		void SetDeclaration(const FString& InDeclaration) { Declaration = InDeclaration; }

		// Get entity document type definition
		FEntityDTD& GetEntityDTD() { return EntityDTD; }

		// Get entity document type definition
		const FEntityDTD& GetEntityDTD() const { return EntityDTD; }

		// Set entity document type definition
		void SetEntityDTD(const FEntityDTD& InEntityDTD) { EntityDTD = InEntityDTD; }

		// Get root node
		FNode& GetRoot() { return Root; }

		// Get root node
		const FNode& GetRoot() const { return Root; }

		// Set root node
		void SetRoot(const FNode& InRoot) { Root = InRoot; };

		// Return document as string
		FString ToString();
	
	private:
		// Declaration ("<?xml version="1.0" encoding="utf-8"?>")
		FString Declaration;

		// Document Type Definition (DTD) for Entity Declaration
		FEntityDTD EntityDTD;

		// Root (including namespace declarations as attributes)
		FNode Root;

		// Current state of the indentation
		FString Indent;
	};

} // namespace SLOwl