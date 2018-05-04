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
	FString NodeStr;
	// Add comment
	if (!Comment.IsEmpty())
	{
		NodeStr += FOwlDoc::Indent + TEXT("<!--") + Comment + TEXT("-->\n");
	}

	// Add node name
	NodeStr += FOwlDoc::Indent + TEXT("<") + Name.ToString();

	// Add attributes to tag
	for (const auto& AttrItr : Attributes)
	{
		NodeStr += TEXT(" ") + AttrItr.ToString();
	}

	// Check node data (children/value)
	bool bNoChildren = ChildNodes.Num() == 0;
	bool bNoValue = Value.IsEmpty();

	if (bNoChildren && bNoValue)
	{
		// No children or value, close tag
		NodeStr += TEXT("/>\n");
	}
	else if (!bNoValue)
	{
		// Node has a value, add value
		NodeStr += TEXT(">") + Value + TEXT("</") + Name.ToString() + TEXT(">");
	}
	else if(!bNoChildren)
	{
		// Node has children, add children
		NodeStr += TEXT(">\n");

		// Increase indentation
		FOwlDoc::Indent += INDENT_STEP;

		// Iterate and add nodes
		for (const auto& ChildItr : ChildNodes)
		{
			NodeStr += ChildItr.ToString();
		}

		// Decrease indentation
		FOwlDoc::Indent.RemoveFromEnd(INDENT_STEP);

		// Close tag
		NodeStr += TEXT(">") + Value + TEXT("</") + Name.ToString() + TEXT(">");
	}
	return NodeStr;
}