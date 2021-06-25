// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SLOwlStructs.h"

/**
* Owl/Xml node
* node will not have Value and Children
*/
struct FSLOwlNode
{
public:
	// Node prefixed name
	FSLOwlPrefixName Name;

	// Node value
	FString Value;

	// Attributes
	TArray<FSLOwlAttribute> Attributes;

	// Nodes
	TArray<FSLOwlNode> ChildNodes;

	// Comment
	FString Comment;

public:
	// Default constructor (emtpy node)
	FSLOwlNode() {}

	// Init constructor, NO Value, Attributes or Children
	FSLOwlNode(const FSLOwlPrefixName& InName) : Name(InName) {}

	// Init constructor, NO Value or Attributes 
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const TArray<FSLOwlNode>& InChildNodes) :
		Name(InName),
		ChildNodes(InChildNodes)
	{}

	// Init constructor, NO Value and Children
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const TArray<FSLOwlAttribute>& InAttributes) :
		Name(InName),
		Attributes(InAttributes)
	{}

	// Init constructor, NO Value and Children, one attribute
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const FSLOwlAttribute& InAttribute) :
		Name(InName)
	{
		Attributes.Add(InAttribute);
	}

	// Init constructor, NO Value
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const TArray<FSLOwlAttribute>& InAttributes,
		const TArray<FSLOwlNode>& InChildNodes) :
		Name(InName),
		Attributes(InAttributes),
		ChildNodes(InChildNodes)
	{}

	// Init constructor, NO Value, one attribute
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const FSLOwlAttribute& InAttribute,
		const TArray<FSLOwlNode>& InChildNodes) :
		Name(InName),
		ChildNodes(InChildNodes)
	{
		Attributes.Add(InAttribute);
	}

	// Init constructor, NO Children
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const TArray<FSLOwlAttribute>& InAttributes,
		const FString& InValue) :
		Name(InName),
		Value(InValue),
		Attributes(InAttributes)
	{}

	// Init constructor, NO Children, one attribute
	FSLOwlNode(const FSLOwlPrefixName& InName,
		const FSLOwlAttribute& InAttribute,
		const FString& InValue) :
		Name(InName),
		Value(InValue)
	{
		Attributes.Add(InAttribute);
	}

	// Add child node
	void AddChildNode(const FSLOwlNode& InChildNode)
	{
		ChildNodes.Add(InChildNode);
	}

	// Add child nodes
	void AddChildNodes(const TArray<FSLOwlNode>& InChildNodes)
	{
		ChildNodes.Append(InChildNodes);
	}

	// Add attribute
	void AddAttribute(const FSLOwlAttribute& InAttribute)
	{
		Attributes.Add(InAttribute);
	}

	// Add attributes
	void AddAttributes(const TArray<FSLOwlAttribute>& InAttributes)
	{
		Attributes.Append(InAttributes);
	}

	// Set the comment
	void SetComment(const FString& InComment)
	{
		Comment = InComment;
	}

	// True if all data is empty
	bool IsEmpty() const
	{
		return Name.IsEmpty() &&
			Value.IsEmpty() &&
			Attributes.Num() == 0 &&
			ChildNodes.Num() == 0 &&
			Comment.IsEmpty();
	}

	// Clear all data
	void Clear()
	{
		Name.Empty();
		Value.Empty();
		Attributes.Empty();
		ChildNodes.Empty();
		Comment.Empty();
	}
	
	// Destructor
	~FSLOwlNode() {}

	// Return node as string
	FString ToString(FString& Indent) const
	{
		FString NodeStr;
		// Add comment
		if (!Comment.IsEmpty())
		{
			NodeStr += TEXT("\n") + Indent + TEXT("<!-- ") + Comment + TEXT(" -->\n");
		}

		// Comment only OR empty node
		if (Name.ToString().IsEmpty())
		{
			return NodeStr;
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

	/* Static helper functions */
	// Create class property
	static FSLOwlNode CreateResourceProperty(const FString& Ns, const FString& Value)
	{
		const FSLOwlPrefixName RdfResource("rdf", "resource");
		const FSLOwlPrefixName RdfType("rdf", "type");
		return FSLOwlNode(RdfType, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(Ns, Value)));
	}
};

// Overloads the constructor to set directly the comment
struct FOwlCommentNode : public FSLOwlNode
{
	// Default constructor
	FOwlCommentNode(const FString& InComment)
	{
		Comment = InComment;
	}
};

