// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Owl.h"

namespace SLOwl
{
	/**
	 * Owl/Xml node
	 * node will not have Value and Children
	 */
	class FOwlNode
	{
	public:
		// Default constructor
		FOwlNode();

		// Init constructor, NO Value and Children
		FOwlNode(const FPrefixName& InName,
			const TArray<FAttribute>& InAttributes);

		// Init constructor, NO Value
		FOwlNode(const FPrefixName& InName,
			const TArray<FAttribute>& InAttributes,
			const TArray<FOwlNode>& InChildNodes);

		// Init constructor, NO Children
		FOwlNode(const FPrefixName& InName,
			const TArray<FAttribute>& InAttributes,
			const FString& InValue);

		// Destructor
		~FOwlNode();

		// Get node name
		FPrefixName GetName() const { return Name; }

		// Set node name
		void SetName(const FPrefixName& InName) { Name = InName; }

		// Get node value
		FString GetValue () const { return Value; }

		// Set node value
		void SetName(const FString& InValue) { Name = InValue; }

		// Get attributes
		const TArray<FAttribute>& GetAttributes() const { return Attributes; }

		// Add attribute
		void AddAttribute(const FAttribute& InAttribute) { Attributes.Add(InAttribute); }

		// Get child nodes
		const TArray<FOwlNode>& GetChildNodes() const { return ChildNodes; }

		// Add child node
		void AddChildNode(const FOwlNode& InChildNode) { ChildNodes.Add(InChildNode); }

		// Get comment
		FString GetComment() const { return Comment; }

		// Set comment
		void SetComment(const FString& InComment) { Comment = InComment; }

		// Return node as string
		FString ToString() const;

	private:
		// Node prefixed name
		FPrefixName Name;

		// Node value
		FString Value;

		// Attributes
		TArray<FAttribute> Attributes;

		// Nodes
		TArray<FOwlNode> ChildNodes;

		// Comment
		FString Comment;
	};

} // namespace SLOwl