// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Owl/Doc.h"

using namespace SLOwl;

// Init the document indent
FString FDoc::Indent = TEXT("");

// Default constructor
FDoc::FDoc()
{
}

// Init constructor
FDoc::FDoc(const FString& InDeclaration,
	const FEntityDTD& InEntityDTD,
	const FNode& InRoot) :
	Declaration(InDeclaration),
	EntityDTD(InEntityDTD),
	Root(InRoot)
{

}

// Destructor
FDoc::~FDoc()
{
}

// Return document as string
FString FDoc::ToString() const
{
	FString Doc = Declaration + TEXT("\n\n");

	Doc += EntityDTD.ToString();

	Doc += Root.ToString();

	return Doc;
}
