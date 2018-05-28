// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlStructs.h"

/**
* Owl/Xml node
* node will not have Value and Children
*/
struct FOwlNode
{
public:
	// Default constructor
	FOwlNode() {}

	// Init constructor, NO Value and Children
	FOwlNode(const FOwlPrefixName& InName,
		const TArray<FOwlAttribute>& InAttributes) :
		Name(InName),
		Attributes(InAttributes)
	{}

	// Init constructor, NO Value and Children, one attribute
	FOwlNode(const FOwlPrefixName& InName,
		const FOwlAttribute& InAttribute) :
		Name(InName)
	{
		Attributes.Add(InAttribute);
	}

	// Init constructor, NO Value
	FOwlNode(const FOwlPrefixName& InName,
		const TArray<FOwlAttribute>& InAttributes,
		const TArray<FOwlNode>& InChildNodes) :
		Name(InName),
		Attributes(InAttributes),
		ChildNodes(InChildNodes)
	{}

	// Init constructor, NO Value, one attribute
	FOwlNode(const FOwlPrefixName& InName,
		const FOwlAttribute& InAttribute,
		const TArray<FOwlNode>& InChildNodes) :
		Name(InName),
		ChildNodes(InChildNodes)
	{
		Attributes.Add(InAttribute);
	}

	// Init constructor, NO Children
	FOwlNode(const FOwlPrefixName& InName,
		const TArray<FOwlAttribute>& InAttributes,
		const FString& InValue) :
		Name(InName),
		Attributes(InAttributes),
		Value(InValue)
	{}

	// Init constructor, NO Children, one attribute
	FOwlNode(const FOwlPrefixName& InName,
		const FOwlAttribute& InAttribute,
		const FString& InValue) :
		Name(InName),
		Value(InValue)
	{
		Attributes.Add(InAttribute);
	}

	// Destructor
	~FOwlNode() {}

	// Return node as string
	FString ToString(FString& Indent)
	{
		FString NodeStr;
		// Add comment
		if (!Comment.IsEmpty())
		{
			NodeStr += TEXT("\n") + Indent + TEXT("<!-- ") + Comment + TEXT(" -->\n");
		}

		// Add node name
		NodeStr += Indent + TEXT("<") + Name.ToString();

		// Add attributes to tag
		for (int32 i = 0; i < Attributes.Num(); ++i)
		{
			if (Attributes.Num() == 1)
			{
				NodeStr += TEXT(" ") + Attributes[i].ToString();
			}
			else
			{
				if (i < (Attributes.Num() - 1))
				{
					NodeStr += TEXT(" ") + Attributes[i].ToString() + TEXT("\n") + Indent + INDENT_STEP;
				}
				else
				{
					// Last attribute does not have new line
					NodeStr += TEXT(" ") + Attributes[i].ToString();
				}
			}
		}	
			
		// Check node data (children/value)
		bool bHasChildren = ChildNodes.Num() != 0;
		bool bHasValue = !Value.IsEmpty();
			
		// Node cannot have value and children
		if (!bHasChildren && !bHasValue)
		{
			// No children nor value, close tag
			NodeStr += TEXT("/>\n");
		}
		else if (bHasValue)
		{
			// Node has a value, add value
			NodeStr += TEXT(">") + Value + TEXT("</") + Name.ToString() + TEXT(">\n");
		}
		else if(bHasChildren)
		{
			// Node has children, add children
			NodeStr += TEXT(">\n");
			
			// Increase indentation
			Indent += INDENT_STEP;
			
			// Iterate children and add nodes
			for (auto& ChildItr : ChildNodes)
			{
				NodeStr += ChildItr.ToString(Indent);
			}
			
			// Decrease indentation
			Indent.RemoveFromEnd(INDENT_STEP);
			
			// Close tag
			NodeStr += Indent + Value + TEXT("</") + Name.ToString() + TEXT(">\n");
		}
		return NodeStr;
	}

	// Node prefixed name
	FOwlPrefixName Name;

	// Node value
	FString Value;

	// Attributes
	TArray<FOwlAttribute> Attributes;

	// Nodes
	TArray<FOwlNode> ChildNodes;
		
	// Comment
	FString Comment;
};
