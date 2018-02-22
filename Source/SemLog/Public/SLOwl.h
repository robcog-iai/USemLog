// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SLOwl.generated.h"

/**
*	FOwlPrefixName, e.g. rdf:type
*   "Prefix + : + Name"
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlPrefixName
{
	GENERATED_USTRUCT_BODY()

	/** Prefix (e.g. rdf)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Prefix;

	/** Name (e.g. type)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Name;

	// Default constructor
	FOwlPrefixName()
	{}
	
	// Constructor from Prefix, Name
	FOwlPrefixName(const FString InPrefix, const FString InName)
		: Prefix(InPrefix), Name(InName)
	{}
	
	// Constructor from FullName (e.g. rdf:type)
	FOwlPrefixName(const FString FullName)
	{
		// Split into Ns and Class
		FullName.Split(":", &Prefix, &Name);
	}
	
	// Return the object as Prefix + Name
	FString GetFullName() const
	{
		return Prefix + ":" + Name;
	}
	
	// Set from full name
	void Set(const FString FullName)
	{
		// Split into Prefix and Class
		FullName.Split(":", &Prefix, &Name);
	}
	
	// Set from namespace and name
	void Set(const FString InPrefix, const FString InName)
	{
		Prefix = InPrefix;
		Name = InName;
	}
};

/**
*	FOwlClass, e.g. &log;SemanticMapPerception
*   "& + Namespace + ; + Class"
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlClass
{
	GENERATED_USTRUCT_BODY()

	/** Namespace of the object (e.g. log)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Ns;

	/** Class of the object (e.g. UnrealExperiment)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Class;

	// Default constructor
	FOwlClass()
	{}

	// Constructor from Namespace, Class, and Id
	FOwlClass(const FString InNs, const FString InClass)
		: Ns(InNs), Class(InClass)
	{}

	// Constructor from FullName (e.g. &log;UnrealExperiment)
	FOwlClass(FString FullName)
	{
		// Remove ampersand from namespace
		FullName.RemoveFromStart("&");
		// Split into Ns and Class
		FullName.Split(";", &Ns, &Class);
	}

	// Return the object as Ns + Class
	FString GetFullName() const
	{
		return "&" + Ns + ";" + Class;
	}

	// Set from full name (e.g &log;SemanticMapPerception)
	void Set(FString FullName)
	{
		// Remove ampersand from namespace
		FullName.RemoveFromStart("&");
		// Split into Ns and Class
		FullName.Split(";", &Ns, &Class);
	}

	// Set from namespace and class
	void Set(const FString InNs, const FString InClass)
	{
		Ns = InNs;
		Class = InClass;
	}
};

/**
*	IndividualName, e.g. &log;SemanticMapPerception_XWRm
*   "& + Namespace + ; + Class + _ + Id"
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlIndividualName : public FOwlClass
{
	GENERATED_USTRUCT_BODY()

	/** Unique Id of the object (e.g. 8uFQ)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Id;

	// Default constructor
	FOwlIndividualName()
	{}

	// Constructor from Namespace, Class, and Id
	FOwlIndividualName(const FString InNs, const FString InClass, const FString InId)
		: FOwlClass(InNs, InClass), Id(InId)
	{}

	// Constructor from Namespace and Name (e.g. log, UnrealExperiment_8uFQ)
	FOwlIndividualName(const FString InNs, const FString InName)
	{
		Ns = InNs;
		InName.Split("_", &Class, &Id);
	}

	// Constructor from FullName (e.g. &log;UnrealExperiment_8uFQ)
	FOwlIndividualName(FString FullName)
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
	FString GetFullName() const
	{
		return "&" + Ns + ";" + Class + "_" + Id;
	}

	// Return as Class + Id
	FString GetName() const
	{
		return Class + "_" + Id;
	}

	// Return the object as Ns + Class
	FString GetClassName() const
	{
		return "&" + Ns + ";" + Class;
	}

	// Set from full name
	void Set(FString FullName)
	{
		// Remove ampersand from namespace
		FullName.RemoveFromStart("&");
		// Split into Ns and Name
		FString Name;
		FullName.Split(";", &Ns, &Name);
		// Split Name into Class and Id
		Name.Split("_", &Class, &Id);
	}

	// Set from namespace and name
	void Set(FString InNs, FString InName)
	{
		Ns = InNs;
		InName.Split("_", &Class, &Id);
	}

	// Set from namespace, class and id
	void Set(const FString InNs, const FString InClass, const FString InId)
	{
		Ns = InNs;
		Class = InClass;
		Id = InId;
	}
};

/**
*	OwlTriple, e.g. <knowrob:objectActedOn rdf:resource="&log;ButtonOven_5CRX"/>
*   Subject - Predicate - Object
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

	/** Triple Value, e.g. "<..>Grasp-LeftHand_BRmZ-Bowl3_9w2Y</..> */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Value;

	// Empty constructor.
	FOwlTriple() {}

	// Constructor.
	FOwlTriple(FString InSubject, FString InPredicate, FString InObject, FString InValue = "")
		: Subject(InSubject)
		, Predicate(InPredicate)
		, Object(InObject)
		, Value(InValue)
	{}

	// Constructor using FOwlPrefixName and FOwlIndividualName
	FOwlTriple(FOwlPrefixName InSubject, FOwlPrefixName InPredicate, FOwlIndividualName InObject, FString InValue = "")
		: Subject(InSubject.GetFullName())
		, Predicate(InPredicate.GetFullName())
		, Object(InObject.GetFullName())
		, Value(InValue)
	{}

	// Constructor using FOwlPrefixName and FOwlClass
	FOwlTriple(FOwlPrefixName InSubject, FOwlPrefixName InPredicate, FOwlClass InObject, FString InValue = "")
		: Subject(InSubject.GetFullName())
		, Predicate(InPredicate.GetFullName())
		, Object(InObject.GetFullName())
		, Value(InValue)
	{}

	// Set subject and return self
	FOwlTriple& SetSubject(FString InSubject)
	{
		Subject = InSubject;
		return *this;
	}

	// Set subject as FOwlPrefixName and return self
	FOwlTriple& SetSubject(FOwlPrefixName InSubject)
	{
		Subject = InSubject.GetFullName();
		return *this;
	}

	// Set predicate and return self
	FOwlTriple& SetPredicate(FString InPredicate)
	{
		Predicate = InPredicate;
		return *this;
	}

	// Set predicate as FOwlPrefixName and return self
	FOwlTriple& SetPredicate(FOwlPrefixName InPredicate)
	{
		Predicate = InPredicate.GetFullName();
		return *this;
	}

	// Set object and return self
	FOwlTriple& SetObject(FString InObject)
	{
		Object = InObject;
		return *this;
	}

	// Set object as FOwlIndividualName  and return self
	FOwlTriple& SetObject(FOwlIndividualName InObject)
	{
		Object = InObject.GetFullName();
		return *this;
	}

	// Set object as FOwlClass  and return self
	FOwlTriple& SetObject(FOwlClass InObject)
	{
		Object = InObject.GetFullName();
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
*	OwlNode e.g. Subject - Predicate - Object - Properties
*	<owl:NamedIndividual rdf:about="&u-map;SemanticMapPerception_XAL4">
*		<rdf:type rdf:resource="&knowrob;SemanticMapPerception"/>
*	</owl:NamedIndividual>
*/
USTRUCT(BlueprintType)
struct SEMLOG_API FOwlNode
{
	GENERATED_USTRUCT_BODY()

	/** Node subject name, e.g. owl:NamedIndividual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Subject;

	/** Node predicate name, e.g. rdf:about*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Predicate;

	/** Attribute value, e.g. "&u-map;USemMap_mhpb"*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Object;

	/** Node triples */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	TArray<FOwlTriple> Properties;

	/** Node name, e.g. owl:NamedIndividual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OWL)
	FString Comment;

	// Empty constructor
	FOwlNode()
	{}

	// Constructor only with comment
	FOwlNode(FString InComment) : Comment(InComment)
	{}

	// Constructor with name and attribute
	FOwlNode(FString InSubject, FString InPredicate, FString InObject, FString InComment = "")
		: Subject(InSubject)
		, Predicate(InPredicate)
		, Object(InObject)
		, Comment(InComment)
	{}

	// Constructor with name and attribute using FOwlPrefixName and FOwlIndividualName
	FOwlNode(FOwlPrefixName InSubject, FOwlPrefixName InPredicate, FOwlIndividualName InObject, FString InComment = "")
		: Subject(InSubject.GetFullName())
		, Predicate(InPredicate.GetFullName())
		, Object(InObject.GetFullName())
		, Comment(InComment)
	{}

	// Constructor with properties
	FOwlNode(FString InSubject, FString InPredicate, FString InObject, const TArray<FOwlTriple>& InProperties, FString InComment = "")
		: Subject(InSubject)
		, Predicate(InPredicate)
		, Object(InObject)
		, Properties(InProperties)
		, Comment(InComment)
	{}

	// Constructor with properties using FOwlPrefixName and FOwlIndividualName
	FOwlNode(FOwlPrefixName InSubject, FOwlPrefixName InPredicate, FOwlIndividualName InObject, const TArray<FOwlTriple>& InProperties, FString InComment = "")
		: Subject(InSubject.GetFullName())
		, Predicate(InPredicate.GetFullName())
		, Object(InObject.GetFullName())
		, Properties(InProperties)
		, Comment(InComment)
	{}
		
	// Set name and return self
	FOwlNode& SetName(FString InSubject)
	{
		Subject = InSubject;
		return *this;
	}

	// Set name and return self using FOwlPrefixName
	FOwlNode& SetName(FOwlPrefixName InSubject)
	{
		Subject = InSubject.GetFullName();
		return *this;
	}

	// Set attribute and return self
	FOwlNode& SetAttribute(FString InPredicate)
	{
		Predicate = InPredicate;
		return *this;
	}

	// Set attribute and return self using FOwlPrefixName
	FOwlNode& SetAttribute(FOwlPrefixName InPredicate)
	{
		Predicate = InPredicate.GetFullName();
		return *this;
	}

	// Set attribute and return self
	FOwlNode& SetAttributeValue(FString InObject)
	{
		Predicate = InObject;
		return *this;
	}

	// Set attribute and return self using FOwlIndividualName
	FOwlNode& SetAttributeValue(FOwlIndividualName InObject)
	{
		Predicate = InObject.GetFullName();
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
			XmlString += "\t<" + Subject + " " + Predicate + "=\"" + Object + "\">\n";
			// Add triple properties
			for (const auto& Triple : Properties)
			{
				XmlString += "\t\t" + Triple.ToXmlString();
			}
			// Close node
			XmlString += "\t</" + Subject + ">\n";
		}
		else if(!Subject.IsEmpty())
		{
			XmlString += "\t<" + Subject + " " + Predicate + "=\"" + Object + "\"/>\n";
		}
		return XmlString;
	}
};

/**
* OwlDocument
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
	TArray<TSharedPtr<FOwlNode>> Nodes;

	// Constructor.
	FOwlDocument()
	{}
	
	// Constructor.
	FOwlDocument(
		const TMap<FString, FString>& InDoctypeAttributes,
		const TMap<FString, FString>& InRdfAttributes,
		const TArray<TSharedPtr<FOwlNode>> & InNodes = TArray<TSharedPtr<FOwlNode>>())
		: DoctypeAttributes(InDoctypeAttributes)
		, RdfAttributes(InRdfAttributes)
		, Nodes(InNodes)
	{}

	// Append nodes
	void AppendNodes(const TArray<TSharedPtr<FOwlNode>>& InNodes, const FString Comment = "")
	{
		if (!Comment.IsEmpty())
		{
			// Add comment before appending the array
			Nodes.Add(MakeShareable(new FOwlNode(Comment)));
		}
		// Append nodes array
		Nodes.Append(InNodes);
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
			XmlString += NodeItr->ToXmlString();
		}

		// End rdf document
		XmlString += "\n</rdf:RDF>\n";

		return XmlString;
	}
};
