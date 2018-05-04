// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Owl/OwlDoc.h"

using namespace SLOwl;

// Init the document indent
FString FOwlDoc::Indent = TEXT("");

// Default constructor
FOwlDoc::FOwlDoc()
{
}

// Init constructor
FOwlDoc::FOwlDoc(const FString& InDeclaration,
	const FEntityDTD& InEntityDTD,
	const FOwlNode& InRoot) :
	Declaration(InDeclaration),
	EntityDTD(InEntityDTD),
	Root(InRoot)
{

}

// Destructor
FOwlDoc::~FOwlDoc()
{
}

// Return document as string
FString FOwlDoc::ToString() const
{
	FString Doc = Declaration + TEXT("\n\n");
	return Doc;
}
