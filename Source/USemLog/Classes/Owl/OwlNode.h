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
		FString GetName() const { return Name.ToString(); }

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
	};

} // namespace SLOwl