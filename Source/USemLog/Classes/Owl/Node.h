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
	class FNode
	{
	public:
		// Default constructor
		FNode();

		// Init constructor, NO Value and Children
		FNode(const FPrefixName& InName,
			const TArray<FAttribute>& InAttributes);

		// Init constructor, NO Value and Children, one attribute
		FNode(const FPrefixName& InName,
			const FAttribute& InAttribute);

		// Init constructor, NO Value
		FNode(const FPrefixName& InName,
			const TArray<FAttribute>& InAttributes,
			const TArray<FNode>& InChildNodes);

		// Init constructor, NO Value, one attribute
		FNode(const FPrefixName& InName,
			const FAttribute& InAttribute,
			const TArray<FNode>& InChildNodes);

		// Init constructor, NO Children
		FNode(const FPrefixName& InName,
			const TArray<FAttribute>& InAttributes,
			const FString& InValue);

		// Init constructor, NO Children, one attribute
		FNode(const FPrefixName& InName,
			const FAttribute& InAttribute,
			const FString& InValue);

		// Destructor
		~FNode();

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

		// Add attributes
		void AddAttributes(const TArray<FAttribute>& InAttributes) { Attributes.Append(InAttributes); }

		// Get child nodes
		const TArray<FNode>& GetChildNodes() const { return ChildNodes; }

		// Add child node
		void AddChildNode(const FNode& InChildNode) { ChildNodes.Add(InChildNode); }

		// Add child nodes
		void AddChildNodes(const TArray<FNode>& InChildNodes) { ChildNodes.Append(InChildNodes); }

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
		TArray<FNode> ChildNodes;

		// Comment
		FString Comment;
	};

} // namespace SLOwl