// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
struct FSLOwlPrefixName
{
public:
	// Prefix 
	FString Prefix;

	// Local name
	FString LocalName;

public:
	// Default constr
	FSLOwlPrefixName() {}

	// Init constructor
	FSLOwlPrefixName(const FString& InPrefix, const FString& InLocalName) : 
		Prefix(InPrefix), LocalName(InLocalName) {}
	
	// Init constructor without local name
	FSLOwlPrefixName(const FString& InPrefix) : Prefix(InPrefix) {}

	// Get name as string
	FString ToString() const
	{
		return LocalName.IsEmpty() ? Prefix : FString(Prefix + TEXT(":") + LocalName);
	}

	// True if all data is empty
	bool IsEmpty() const
	{
		return Prefix.IsEmpty() && LocalName.IsEmpty();
	}

	// Clear all data
	void Empty()
	{
		Prefix.Empty();
		LocalName.Empty();
	}
};

/**
*  Example: "&log;RightHand_vqDR"
*  Ns:  "log"
*  Value: "RightHand_vqDR"
*/
struct FSLOwlAttributeValue
{
public:
	// Namespace
	FString Ns;

	// Value
	FString LocalValue;

public:
	// Default constr
	FSLOwlAttributeValue() {}

	// Init constructor
	FSLOwlAttributeValue(const FString& InNs, const FString& InLocalValue) :
		Ns(InNs), LocalValue(InLocalValue) {}

	// Init constructor without ns
	FSLOwlAttributeValue(const FString& InLocalValue) : LocalValue(InLocalValue) {}

	// Get value as string
	FString ToString() const
	{
		return Ns.IsEmpty() ? TEXT("\"") + LocalValue + TEXT("\"")
			: FString(TEXT("\"&") + Ns + TEXT(";") + LocalValue + TEXT("\""));
	}

	// True if all data is empty
	bool IsEmpty() const
	{
		return Ns.IsEmpty() && LocalValue.IsEmpty();
	}

	// Clear all data
	void Empty()
	{
		Ns.Empty();
		LocalValue.Empty();
	}
};
	
/**
*  Example: "rdf:about="&log;RightHand_vqDR""
*  Key:  ""rdf:about"
*  Value: "&log;RightHand_vqDR"
*/
struct FSLOwlAttribute
{
public:
	// Key 
	FSLOwlPrefixName Key;

	// Value
	FSLOwlAttributeValue Value;

public:
	// Default constr
	FSLOwlAttribute() {}

	// Init constr
	FSLOwlAttribute(const FSLOwlPrefixName& InKey, const FSLOwlAttributeValue& InValue) :
		Key(InKey), Value(InValue) {}

	// Get attribute as string
	FString ToString() const 
	{
		return Key.ToString() + TEXT("=") + Value.ToString();
	}

	// True if all data is empty
	bool IsEmpty() const
	{
		return Key.IsEmpty() && Value.IsEmpty();
	}

	// Clear all data
	void Empty()
	{
		Key.Empty();
		Value.Empty();
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
struct FSLOwlEntityDTD
{
public:
	// Root name
	FSLOwlPrefixName Name;

	// Array of the "!ENTITY" Key-Value pairs
	TArray<TPairString> EntityPairs;

public:
	// Default constr
	FSLOwlEntityDTD() : Name(FSLOwlPrefixName("rdf", "RDF")) {}

	// Init constr with default name
	FSLOwlEntityDTD(const TArray<TPairString>& InEntityPairs) :
		Name(FSLOwlPrefixName("rdf", "RDF")),
		EntityPairs(InEntityPairs)
	{}

	// Init constr
	FSLOwlEntityDTD(const FSLOwlPrefixName& InName,
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
				TEXT(" \"") + EntityItr.Value + TEXT("\">\n");
		}
		DTDStr += TEXT("]>\n\n");
		return DTDStr;
	}

	// True if all data is empty
	bool IsEmpty() const
	{
		return Name.IsEmpty() && EntityPairs.Num() == 0;
	}

	// Clear all data
	void Clear()
	{
		Name.Empty();
		EntityPairs.Empty();
	}
};
