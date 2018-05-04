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

// Destructor
FOwlDoc::~FOwlDoc()
{
}

// Return document as string
FString FOwlDoc::ToString() const
{
	return FString();
}
