// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "UObject/NoExportTypes.h"
#include "SLOwl.generated.h"


/**
*
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlNsName
{
	GENERATED_USTRUCT_BODY()

	/** Namespace part (e.g. knowrob) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Ns;

	/** Name part (e.g. depthOfObject)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Name;
	
	// Default constructor
	FOwlNsName()
	{
	}

	// Constructor from Namespace and Name
	FOwlNsName(FString InNs, FString InName)
		: Ns(InNs), Name(InName)
	{
	}

	// Constructor from namespaced name
	FOwlNsName(FString NsName)
	{
		// Remove ampersand from namespace
		NsName.RemoveFromStart("&");
		// Split into Ns and Name
		NsName.Split(";", &Ns, &Name);
	}

	// Return as Ns + Name
	FString GetNsName()
	{
		return "&" + Ns + ";" + Name;
	}

	// Set from namespaced name
	bool Set(FString NsName)
	{
		// Remove ampersand from namespace
		if (NsName.RemoveFromStart("&"))
		{
			if (NsName.Split(";", &Ns, &Name))
			{
				return true;
			}
		}
		return false;
	}

	// Set from namespaced name
	bool Set(FString InNs, FString InName)
	{
		if (!InNs.IsEmpty() && !InName.IsEmpty())
		{
			Ns = InNs;
			Name = InName;
			return true;
		}
		return false;
	}
};

/**
*
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlObject
{
	GENERATED_USTRUCT_BODY()

	/** Namespace of the object (e.g. log)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Ns;

	/** Class of the object (e.g. UnrealExperiment)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Class;

	/** Unique Id of the object (e.g. 8uFQ)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Id;

	// Default constructor
	FOwlObject()
	{
	}

	// Constructor from Namespace, Class, and Id
	FOwlObject(FString InNs, FString InClass, FString InId)
		: Ns(InNs), Class(InClass), Id(InId)
	{
	}

	// Constructor from Namespace and Name (e.g. log, UnrealExperiment_8uFQ)
	FOwlObject(FString InNs, FString InName)
		: Ns(InNs)
	{
		InName.Split("_", &Class, &Id);		
	}

	// Constructor from FullName (e.g. &log;UnrealExperiment_8uFQ)
	FOwlObject(FString FullName)
	{
		// Remove ampersand from namespace
		FullName.RemoveFromStart("&");
		// Split into Ns and Name
		FString Name;
		FullName.Split(";", &Ns, &Name);
		// Split Name into Class and Id
		Name.Split("_", &Class, &Id);
	}

	// Return the object as Ns + Class + Id
	FString GetFullName()
	{
		return "&" + Ns + ";" + Class + "_" + Id;
	}

	// Return as Class + Id
	FString GetName()
	{
		return Class + "_" + Id;
	}

	// Set from full name
	bool Set(FString FullName)
	{
		// Remove ampersand from namespace
		if (FullName.RemoveFromStart("&"))
		{
			// Split into Ns and Name
			FString Name;
			if (FullName.Split(";", &Ns, &Name))
			{
				// Split Name into Class and Id
				if (Name.Split("_", &Class, &Id))
				{
					return true;
				}
			}
		}
		return false;
	}

	// Set from namespace and name
	bool Set(FString InNs, FString InName)
	{
		// Set the class 
		if (!InNs.IsEmpty() && InName.Split("_", &Class, &Id))
		{
			Ns = InNs;
			return true;
		}
		return false;
	}

	// Set from namespace, class and id
	bool Set(FString InNs, FString InClass, FString InId)
	{
		if (!InNs.IsEmpty() && !InClass.IsEmpty() && !InId.IsEmpty())
		{
			Ns = InNs;
			Class = InClass;
			Id = InId;
			return true;
		}
		return false;
	}
};

/**
*
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlTriple
{
	GENERATED_USTRUCT_BODY()

	/** Triple Subject, e.g. owl:ObjectProperty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Subject;

	/** Triple Predicate, e.g. rdf:about */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Predicate;

	/** Triple Object, e.g. &log;UnrealExperiment_8uFQ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Object;
	//FOwlObject Object;

	/** Triple Value, e.g. "<..>Grasp-LeftHand_BRmZ-Bowl3_9w2Y</..> */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Value;

	// Empty constructor.
	FOwlTriple() {}

	// Constructor.
	FOwlTriple(FString InSubject, FString InPredicate, FString InObject, FString InValue = "")
		: Subject(InSubject), Predicate(InPredicate), Object(InObject), Value(InValue)
	{}

	// Set subject and return self
	FOwlTriple& SetSubject(FString InSubject)
	{
		Subject = InSubject;
		return *this;
	}

	// Set predicate and return self
	FOwlTriple& SetPredicate(FString InPredicate)
	{
		Predicate = InPredicate;
		return *this;
	}

	// Set object and return self
	FOwlTriple& SetObject(FString InObject)
	{
		Object = InObject;
		return *this;
	}

	// Set value and return self
	FOwlTriple& SetValue(FString InValue)
	{
		Value = InValue;
		return *this;
	}

	// Write as XML String
	FString ToXmlString() const
	{
		FString XmlString;

		if (Value.IsEmpty())
		{
			XmlString += "<" + Subject + " " + Predicate + "=\"" + Object + "\"/>\n";
		}
		else
		{
			XmlString += "<" + Subject + " " + Predicate + "=\"" + Object + "\">" + Value + "</" + Subject + ">\n";
		}
		return XmlString;
	}
};

/**
*
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlNode
{
	GENERATED_USTRUCT_BODY()

	/** Node name, e.g. owl:NamedIndividual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Name;

	/** Node attribute, e.g. rdf:about*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Attribute;

	/** Attribute value, e.g. "&u-map;USemMap_mhpb"*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString AttributeValue;

	/** Node triples */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	TArray<FOwlTriple> Properties;

	/** Node name, e.g. owl:NamedIndividual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Comment;

	// Empty constructor
	FOwlNode()
	{}

	// Constructor with name and attribute
	FOwlNode(FString InName, FString InAttribute, FString InAttributeValue, FString InComment = "")
		: Name(InName), Attribute(InAttribute), AttributeValue(InAttributeValue), Comment(InComment)
	{}

	// Constructor with properties
	FOwlNode(FString InName, FString InAttribute, FString InAttributeValue, const TArray<FOwlTriple>& InProperties, FString InComment = "")
		: Name(InName), Attribute(InAttribute), AttributeValue(InAttributeValue), Properties(InProperties), Comment(InComment)
	{}
		
	// Set name and return self
	FOwlNode& SetName(FString InName)
	{
		Name = InName;
		return *this;
	}

	// Set attribute and return self
	FOwlNode& SetAttribute(FString InAttribute)
	{
		Attribute = InAttribute;
		return *this;
	}

	// Set attribute and return self
	FOwlNode& SetAttributeValue(FString InAttributeValue)
	{
		Attribute = InAttributeValue;
		return *this;
	}

	// Set properties and return self
	FOwlNode& SetProperties(const TArray<FOwlTriple>& InProperties)
	{
		Properties = InProperties;
		return *this;
	}

	// Set comment and return self
	FOwlNode& SetComment(FString InComment)
	{
		Comment = InComment;
		return *this;
	}

	// Write as XML String
	FString ToXmlString() const
	{
		FString XmlString;

		if (!Comment.IsEmpty())
		{
			XmlString += "\n\t<!-- " + Comment + " -->\n";
		}

		if (Properties.Num() > 0)
		{
			// Start node
			XmlString += "\t<" + Name + " " + Attribute + "=\"" + AttributeValue + "\">\n";
			// Add triple properties
			for (const auto& Triple : Properties)
			{
				XmlString += "\t\t" + Triple.ToXmlString();
			}
			// Close node
			XmlString += "\t</" + Name + ">\n";
		}
		else
		{
			XmlString += "\t<" + Name + " " + Attribute + "=\"" + AttributeValue + "\"/>\n";
		}
		return XmlString;
	}
};

/**
*
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlDocument
{
	GENERATED_USTRUCT_BODY()

	/** Doctype attributes string array*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	TMap<FString, FString> DoctypeAttributes;

	/** Doctype attributes string array*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	TMap<FString, FString> RdfAttributes;

	/** Nodes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	TArray<FOwlNode> Nodes;

	// Constructor.
	FOwlDocument()
	{
	}
	
	// Constructor.
	FOwlDocument(
		const TMap<FString, FString>& InDoctypeAttributes,
		const TMap<FString, FString>& InRdfAttributes,
		const TArray<FOwlNode>& InNodes = TArray<FOwlNode>())
		: DoctypeAttributes(InDoctypeAttributes),
		RdfAttributes(InRdfAttributes),
		Nodes(InNodes)
	{
	}

	// Write as XML String
	FString ToXmlString() const
	{
		FString XmlString;

		// Filetype
		XmlString += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n";

		// Doctype
		XmlString += "<!DOCTYPE rdf:RDF[\n";
		for (const auto& DocTypeAttrItr : DoctypeAttributes)
		{
			XmlString += "\t<!ENTITY " 
				+ DocTypeAttrItr.Key 
				+ " \"" + DocTypeAttrItr.Value + "\"" 
				+ ">\n";
		}
		XmlString += "]>\n\n";

		// Start rdf document
		XmlString += "<rdf:RDF \n";
		for (const auto& RdfAttrItr : RdfAttributes)
		{
			XmlString += "\t" + RdfAttrItr.Key +"=\"" + RdfAttrItr.Value + "\"\n";
		}
		XmlString += ">\n";

		// Add nodes
		for (const auto& NodeItr : Nodes)
		{
			XmlString += NodeItr.ToXmlString();
		}

		// End rdf document
		XmlString += "\n</rdf:RDF>\n";

		return XmlString;
	}
};
