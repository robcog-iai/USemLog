// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Owl/Node.h"
#include "Owl/Doc.h"

using namespace SLOwl;

// Default constructor
FNode::FNode()
{
}

// Init constructor, NO Value and Children
FNode::FNode(const FPrefixName& InName,
	const TArray<FAttribute>& InAttributes) :
	Name(InName),
	Attributes(InAttributes)
{
}

// Init constructor, NO Value and Children, one attribute
FNode::FNode(const FPrefixName& InName,
	const FAttribute& InAttribute) :
	Name(InName)
{
	Attributes.Add(InAttribute);
}


// Init constructor, NO Value
FNode::FNode(const FPrefixName& InName,
	const TArray<FAttribute>& InAttributes,
	const TArray<FNode>& InChildNodes) :
	Name(InName),
	Attributes(InAttributes),
	ChildNodes(InChildNodes)
{
}

// Init constructor, NO Value, one attribute
FNode::FNode(const FPrefixName& InName,
	const FAttribute& InAttribute,
	const TArray<FNode>& InChildNodes) :
	Name(InName),
	ChildNodes(InChildNodes)
{
	Attributes.Add(InAttribute);
}

// Init constructor, NO Children
FNode::FNode(const FPrefixName& InName,
	const TArray<FAttribute>& InAttributes,
	const FString& InValue) :
	Name(InName),
	Attributes(InAttributes),
	Value(InValue)
{
}

// Init constructor, NO Children, one attribute
FNode::FNode(const FPrefixName& InName,
	const FAttribute& InAttribute,
	const FString& InValue) :
	Name(InName),
	Value(InValue)
{
	Attributes.Add(InAttribute);
}

// Destructor
FNode::~FNode()
{
}

// Return node as string
FString FNode::ToString() const
{
	FString NodeStr;
	// Add comment
	if (!Comment.IsEmpty())
	{
		NodeStr += FDoc::Indent + TEXT("<!--") + Comment + TEXT("-->\n");
	}

	// Add node name
	NodeStr += FDoc::Indent + TEXT("<") + Name.ToString();

	// Add attributes to tag
	for (const auto& AttrItr : Attributes)
	{
		NodeStr += TEXT(" ") + AttrItr.ToString();
	}

	// Check node data (children/value)
	bool bHasChildren = ChildNodes.Num() != 0;
	bool bHasValue = !Value.IsEmpty();

	// Node cannot have value and children
	if (!bHasChildren && !bHasValue)
	{
		// No children nor value, close tag
		NodeStr += TEXT("/>\n\n");
	}
	else if (bHasValue)
	{
		// Node has a value, add value
		NodeStr += TEXT(">") + Value + TEXT("</") + Name.ToString() + TEXT(">\n\n");
	}
	else if(bHasChildren)
	{
		// Node has children, add children
		NodeStr += TEXT(">\n");

		// Increase indentation
		FDoc::Indent += INDENT_STEP;

		// Iterate and add nodes
		for (const auto& ChildItr : ChildNodes)
		{
			NodeStr += ChildItr.ToString();
		}

		// Decrease indentation
		FDoc::Indent.RemoveFromEnd(INDENT_STEP);

		// Close tag
		NodeStr += FDoc::Indent + Value + TEXT("</") + Name.ToString() + TEXT(">\n");
	}
	return NodeStr;
}