// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

// Pair of strings typedef
typedef TPair<FString, FString> TPairString;

// Indent step
static const FString INDENT_STEP = TEXT("\t");

/**
	*  Example: "owl:Class", "rdf:resource"
	*  Prefix:  "rdf", "owl" (expansion "http://www.w3.org/1999/02/22-rdf-syntax-ns#", "http://www.w3.org/2002/07/owl#")
	*  Name: "Class", "subClassOf"
	*/
struct FOwlPrefixName
{
public:
	// Prefix 
	FString Prefix;

	// Local name
	FString LocalName;

public:
	// Default constr
	FOwlPrefixName() {}

	// Init constructor
	FOwlPrefixName(const FString& InPrefix, const FString& InLocalName) : 
		Prefix(InPrefix), LocalName(InLocalName) {}
	
	// Init constructor without local name
	FOwlPrefixName(const FString& InPrefix) : Prefix(InPrefix) {}

	// Get name as string
	FString ToString() const
	{
		return LocalName.IsEmpty() ? Prefix : FString(Prefix + TEXT(":") + LocalName);
	}
};

/**
*  Example: "&log;RightHand_vqDR"
*  Ns:  "log"
*  Value: "RightHand_vqDR"
*/
struct FOwlAttributeValue
{
public:
	// Namespace
	FString Ns;

	// Value
	FString LocalValue;

public:
	// Default constr
	FOwlAttributeValue() {}

	// Init constructor
	FOwlAttributeValue(const FString& InNs, const FString& InLocalValue) :
		Ns(InNs), LocalValue(InLocalValue) {}

	// Init constructor without ns
	FOwlAttributeValue(const FString& InLocalValue) : LocalValue(InLocalValue) {}

	// Get value as string
	FString ToString() const
	{
		return Ns.IsEmpty() ? TEXT("\"") + LocalValue + TEXT("\"")
			: FString(TEXT("\"&") + Ns + TEXT(";") + LocalValue + TEXT("\""));
	}
};
	
/**
*  Example: "rdf:about="&log;RightHand_vqDR""
*  Key:  ""rdf:about"
*  Value: "&log;RightHand_vqDR"
*/
struct FOwlAttribute
{
public:
	// Key 
	FOwlPrefixName Key;

	// Value
	FOwlAttributeValue Value;

public:
	// Default constr
	FOwlAttribute() {}

	// Init constr
	FOwlAttribute(const FOwlPrefixName& InKey, const FOwlAttributeValue& InValue) :
		Key(InKey), Value(InValue) {}

	// Get attribute as string
	FString ToString() const 
	{
		return Key.ToString() + TEXT("=") + Value.ToString();
	}
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
struct FOwlEntityDTD
{
public:
	// Root name
	FOwlPrefixName Name;

	// Array of the "!ENTITY" Key-Value pairs
	TArray<TPairString> EntityPairs;

public:
	// Default constr
	FOwlEntityDTD() : Name(FOwlPrefixName("rdf", "RDF")) {}

	// Init constr with default name
	FOwlEntityDTD(const TArray<TPairString>& InEntityPairs) :
		Name(FOwlPrefixName("rdf", "RDF")),
		EntityPairs(InEntityPairs)
	{}

	// Init constr
	FOwlEntityDTD(const FOwlPrefixName& InName,
		const TArray<TPairString>& InEntityPairs) :
		Name(InName),
		EntityPairs(InEntityPairs)
	{}

	// Add pair
	void AddPair(const TPairString& InPair)
	{
		EntityPairs.Add(InPair);
	}

	// Add pairs
	void AddPairs(const TArray<TPairString>& InPairs)
	{
		EntityPairs.Append(InPairs);
	}

	// Get entity declaration string
	FString ToString() const
	{
		if (EntityPairs.Num() == 0)
			return FString();

		FString DTDStr = TEXT("<!DOCTYPE ") + Name.ToString() + TEXT("[\n");
		for (const auto& EntityItr : EntityPairs)
		{
			DTDStr += INDENT_STEP + TEXT("<!ENTITY ") + EntityItr.Key +
				TEXT("\"") + EntityItr.Value + TEXT("\">\n");
		}
		DTDStr += TEXT("]>\n\n");
		return DTDStr;
	}
};
