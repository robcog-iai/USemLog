// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

namespace SLOwl
{
	// Indent step
	static const FString INDENT_STEP = TEXT("\t");

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

		// Get name as string
		FString ToString() const
		{
			return Prefix.IsEmpty() ? LocalName : FString(Prefix + TEXT(":") + LocalName);
		}

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

		// Get value as string
		FString ToString() const
		{
			return Ns.IsEmpty() ? LocalValue : FString(TEXT("\"&") + Ns + TEXT(";") + LocalValue);
		}

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

		// Key 
		FPrefixName Key;

		// Value
		FAttributeValue Value;
	};
	
	/**
	* Document Type Definition (DTD) for Entity Declaration
	*  Example:
	*	"<!DOCTYPE rdf:RDF [
	*	<!ENTITY rdf "http://www.w3.org/1999/02/22-rdf-syntax-ns">
	*	<!ENTITY rdfs "http://www.w3.org/2000/01/rdf-schema">
	*	<!ENTITY owl "http://www.w3.org/2002/07/owl">
	*	]>"
	*  Root:  "rdf:RDF"
	*  Key1: "rdf"
	*  Value1: "http://www.w3.org/1999/02/22-rdf-syntax-ns"
	*/
	struct FEntityDTD
	{
		// Default constr
		FEntityDTD() {}

		// Init constr
		FEntityDTD(const FPrefixName& InRoot,
			const TMap<FString, FString>& InEntityPairs) :
			Root(InRoot),
			EntityPairs(InEntityPairs)
		{}

		// Get entity declaration string
		FString ToString() const
		{
			FString DTDStr = TEXT("<!DOCTYPE ") + Root.ToString() + TEXT("[\n");
			for (const auto& EntityItr : EntityPairs)
			{
				DTDStr += INDENT_STEP + TEXT("<!ENTITY ") + EntityItr.Key +
					TEXT("\"") + EntityItr.Value + TEXT("\">\n");
			}
			DTDStr += TEXT("]>\n");
			return DTDStr;
		}

		// Root
		FPrefixName Root;

		// Array of the "!ENTITY" Key-Value pairs
		TMap<FString, FString> EntityPairs;
	};

} // namespace SLOwl
