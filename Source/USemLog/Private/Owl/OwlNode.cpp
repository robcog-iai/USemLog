// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Owl/OwlNode.h"
#include "Owl/OwlDoc.h"

using namespace SLOwl;

// Default constructor
FOwlNode::FOwlNode()
{
}

// Init constructor, NO Value and Children
FOwlNode::FOwlNode(const FPrefixName& InName,
	const TArray<FAttribute>& InAttributes) :
	Name(InName),
	Attributes(InAttributes)
{
}


// Init constructor, NO Value
FOwlNode::FOwlNode(const FPrefixName& InName,
	const TArray<FAttribute>& InAttributes,
	const TArray<FOwlNode>& InChildNodes) :
	Name(InName),
	Attributes(InAttributes),
	ChildNodes(InChildNodes)
{
}

// Init constructor, NO Children
FOwlNode::FOwlNode(const FPrefixName& InName,
	const TArray<FAttribute>& InAttributes,
	const FString& InValue) :
	Name(InName),
	Attributes(InAttributes),
	Value(InValue)
{
}

// Destructor
FOwlNode::~FOwlNode()
{
}

// Return node as string
FString FOwlNode::ToString() const
{
	FString NodeStr = FOwlDoc::Indent + TEXT("<") + Name.ToString();

	for (const auto& AttrItr : Attributes)
	{
		NodeStr += TEXT(" ") + AttrItr.ToString();
	}

	bool bNoChildren = ChildNodes.Num() == 0;
	bool bNoValue = Value.IsEmpty();

	if (bNoChildren && bNoValue)
	{
		// If no children and no value
		NodeStr += TEXT("/>\n");
		return NodeStr;
	}
	else if (!bNoValue)
	{
		NodeStr += TEXT(">") + Value + TEXT("</") + Name.ToString() + TEXT(">");
	}
	else if(!bNoChildren)
	{
		NodeStr += TEXT(">\n");
		FOwlDoc::Indent += INDENT_STEP;
		for (const auto& ChildItr : ChildNodes)
		{
			NodeStr += ChildItr.ToString();
		}
		FOwlDoc::Indent.RemoveFromEnd(INDENT_STEP);
		NodeStr += TEXT(">") + Value + TEXT("</") + Name.ToString() + TEXT(">");
	}
	return NodeStr;
}