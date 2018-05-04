// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

namespace SLOwl
{
	/**
	 *  Example: "owl:Class", "rdf:resource"
	 *  Prefix:  "rdf", "owl" (expansion "http://www.w3.org/1999/02/22-rdf-syntax-ns#", "http://www.w3.org/2002/07/owl#")
	 *  Name: "Class", "subClassOf"
	 */
	struct FPrefixName
	{
		// Default constr
		FPrefixName() {}

		// Init constructor
		FPrefixName(const FString& InPrefix, const FString& InLocalName) : 
			Prefix(InPrefix), LocalName(InLocalName) {}
	
		// Init constructor without prefix
		FPrefixName(const FString& InLocalName) : LocalName(InLocalName) {}

		// Get prefix
		FString GetPrefix() const { return Prefix; }

		// Get local name
		FString GetLocalName() const { return LocalName; }

		// Get name as string
		FString ToString() const
		{
			return Prefix.IsEmpty() ? LocalName : FString(Prefix + TEXT(":") + LocalName);
		}

	private:
		// Prefix 
		FString Prefix;

		// Local name
		FString LocalName;
	};

	/**
	*  Example: "&log;RightHand_vqDR"
	*  Ns:  "log"
	*  Value: "RightHand_vqDR"
	*/
	struct FAttributeValue
	{
		// Default constr
		FAttributeValue() {}

		// Init constructor
		FAttributeValue(const FString& InNs, const FString& InLocalValue) :
			Ns(InNs), LocalValue(InLocalValue) {}

		// Init constructor without ns
		FAttributeValue(const FString& InLocalValue) : LocalValue(InLocalValue) {}

		// Get prefix
		FString GetNs() const { return Ns; }

		// Get local name
		FString GetLocalValue() const { return LocalValue; }

		// Get value as string
		FString ToString() const
		{
			return Ns.IsEmpty() ? LocalValue : FString(TEXT("\"&") + Ns + TEXT(";") + LocalValue);
		}

	private:
		// Namespace
		FString Ns;

		// Value
		FString LocalValue;
	};


	/**
	*  Example: "rdf:about="&log;RightHand_vqDR""
	*  Key:  ""rdf:about"
	*  Value: "&log;RightHand_vqDR"
	*/
	struct FAttribute
	{
		// Default constr
		FAttribute() {}

		// Init constr
		FAttribute(const FPrefixName& InKey, const FAttributeValue& InValue) :
			Key(InKey), Value(InValue) {}

		// Get attribute as string
		FString ToString() const 
		{
			return Key.ToString() + TEXT("=") + Value.ToString();
		}

	private:
		// Key 
		FPrefixName Key;

		// Value
		FAttributeValue Value;
	};

} // namespace SLOwl

