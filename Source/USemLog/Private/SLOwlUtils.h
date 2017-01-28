// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Private/SLUtils.h"
#include "rapidxml/rapidxml_print.hpp"


/**
* Convert FString to const char*
*/ 
static FORCEINLINE const char* FStringToChar(const FString FStr)
{
	const std::string str = TCHAR_TO_UTF8(*FStr);
	const size_t str_length = str.length() + 1;
	char *cstr = new char[str_length];
	//strcpy(cstr, str.c_str());
	strcpy_s(cstr, str_length + 1, str.c_str());
	return cstr;
}

// Add comment to node
static FORCEINLINE void AddNodeComment(
	rapidxml::xml_document<>* Doc,
	rapidxml::xml_node<>* ParentNode,
	const char* Comment)
{
	// Create comment node
	rapidxml::xml_node<> *CommentNode = Doc->allocate_node(rapidxml::node_comment, "", Comment);
	// Append comment node to parent
	ParentNode->append_node(CommentNode);
}

// Add attribute to node
static FORCEINLINE void AddNodeAttribute(
	rapidxml::xml_document<>* Doc,
	rapidxml::xml_node<>* ParentNode,
	const char* Name,
	const char* Value)
{
	ParentNode->append_attribute(Doc->allocate_attribute(
		Name, Value));
}

/**
* OwlTriple
*/
class FSLOwlTriple
{
public:
	// Constructor
	FSLOwlTriple(const char* Subj, const char* Pred, const char* Obj, const char* Val = "")
		: Subject(Subj), Predicate(Pred), Object(Obj), Value(Val)
	{};

	// Constructor
	FSLOwlTriple(const char* Subj, const char* Pred, const FString Obj, const char* Val = "")
		: Subject(Subj), Predicate(Pred), Object(FStringToChar(Obj)), Value(Val)
	{};

	// Constructor
	FSLOwlTriple(const FString Subj, const FString Pred, const FString Obj, const FString Val = "")
		: Subject(FStringToChar(Subj)),
		Predicate(FStringToChar(Pred)),
		Object(FStringToChar(Obj)),
		Value(FStringToChar(Val))
	{};

	
	// Add to XML document
	void AddToDocument(rapidxml::xml_document<>* Doc, rapidxml::xml_node<>* ParentNode)
	{
		// Create Triple node
		rapidxml::xml_node<> *TripleNode = Doc->allocate_node(rapidxml::node_element, Subject, Value);
		// Add predicate and object to Triple
		TripleNode->append_attribute(Doc->allocate_attribute(Predicate, Object));
		// Append triple to parent
		ParentNode->append_node(TripleNode);
	}

	// Subject e.g. owl:ObjectProperty
	const char* Subject;

	// Predicate e.g. rdf:about
	const char* Predicate;

	// Object e.g. &log;UnrealExperiment_8uFQ
	const char* Object;

	// Value e.g. >8uFQ<
	const char* Value;
};

/**
* ObjectIndividual
*/
class FSLOwlObjectIndividual
{
public:
	// Constructor
	FSLOwlObjectIndividual(
		const FString ObjNamespace,
		const FString ObjUniqueName,
		const TArray<FSLOwlTriple>& Prop = TArray<FSLOwlTriple>(),
		const FString Subj = "owl:NamedIndividual",
		const FString Pred = "rdf:about") :
		ObjectNamespace(ObjNamespace),
		ObjectUniqueName(ObjUniqueName),
		Properties(Prop),
		Subject(Subj),
		Predicate(Pred)
	{
		Object = ObjNamespace + ObjUniqueName;
	};

	// Constructor with object name (namespace + unqiue name)
	FSLOwlObjectIndividual(
		const FString Obj,
		const TArray<FSLOwlTriple>& Prop = TArray<FSLOwlTriple>(),
		const FString Subj = "owl:NamedIndividual",
		const FString Pred = "rdf:about") :
		Object(Obj),
		Properties(Prop),
		Subject(Subj),
		Predicate(Pred)
	{
		if (!Obj.Split(";", &ObjectNamespace/*+";"*/, &ObjectUniqueName))
		{
			UE_LOG(SemLogEvent, Error, TEXT(" !! Could not split object %s into namespace and unique name."),
				*Obj);
		};
		// Namespace +=";";
	};

	FSLOwlObjectIndividual(
		const FString Subj,
		const FString Pred,
		const FString Obj,
		const TArray<FSLOwlTriple>& Prop = TArray<FSLOwlTriple>()) :
		Subject(Subj),
		Predicate(Pred),
		Object(Obj),
		Properties(Prop)
	{
		if (!Obj.Split(";", &ObjectNamespace/*+";"*/, &ObjectUniqueName))
		{
			UE_LOG(SemLogEvent, Error, TEXT(" !! Could not split object %s into namespace and unique name."),
				*Obj);
		};
		// Namespace +=";";
	};

	// Add property
	void AddProperty(FSLOwlTriple Property) { Properties.Add(Property); };

	// Get object
	FString GetObject() { return Object; };

	// Get properties
	TArray<FSLOwlTriple>& GetProperties() { return Properties; };

	// Get object namespace
	FString GetObjectNamespace() { return ObjectNamespace; };

	// Get object unique name
	FString GetObjectUniqueName() { return ObjectUniqueName; };

	// Add to XML document
	void AddToDocument(rapidxml::xml_document<>* Doc, rapidxml::xml_node<>* ParentNode)
	{
		// Create the entity node
		rapidxml::xml_node<> *EntityNode = Doc->allocate_node(rapidxml::node_element,
			FStringToChar(Subject), "");
		// Add predicate and object to entity node
		EntityNode->append_attribute(Doc->allocate_attribute(
			FStringToChar(Predicate), FStringToChar(Object)));
		// Add property triples to entity node
		for (const auto PropertyItr : Properties)
		{
			// Create Triple node
			rapidxml::xml_node<> *TripleNode = Doc->allocate_node(rapidxml::node_element,
				PropertyItr.Subject, PropertyItr.Value);
			// Add predicate and object to Triple
			TripleNode->append_attribute(Doc->allocate_attribute(
				PropertyItr.Predicate, PropertyItr.Object));
			// Append triple to parent
			EntityNode->append_node(TripleNode);
		}
		// Append entity to parent
		ParentNode->append_node(EntityNode);
	}

protected:
	// Subject
	FString Subject;

	// Predicate
	FString Predicate;

	// Object (Namespace + UniqueName)
	FString Object;

	// Named individual namsepace
	FString ObjectNamespace;

	// Named individual unique name
	FString ObjectUniqueName;

	// Named individual properties
	TArray<FSLOwlTriple> Properties;
};

/**
* EventIndividual
*/
class FSLOwlEventIndividual : public FSLOwlObjectIndividual
{
public:
	// Constructor
	FSLOwlEventIndividual(
		const FString Ns,
		const FString UniqName,
		float Start = -1.0f,
		float End = -1.0f,
		const TArray<FSLOwlTriple>& Prop = TArray<FSLOwlTriple>()) :
		FSLOwlObjectIndividual(Ns, UniqName, Prop),
		StartTime(Start),
		EndTime(End)
	{
		if (StartTime > -1.f)
		{
			Properties.Add(FSLOwlEventIndividual::CreateTimeProperty(StartTime));
			bStartPropertySet = true;
		}
		bStartPropertySet = false;

		if (EndTime > -1.f)
		{
			Properties.Add(FSLOwlEventIndividual::CreateTimeProperty(EndTime));
			bEndPropertySet = true;
		}
		bEndPropertySet = false;
	};

	// Set end time
	void SetStartTime(float NewStartTime) { StartTime = NewStartTime; };

	// Get start time
	float GetStartTime() { return StartTime; };

	// Set end time
	void SetEndTime(float NewEndTime) { EndTime = NewEndTime; };

	// Get end time
	float GetEndTime() { return EndTime; };


protected:
	// Start time of the event
	float StartTime;

	// End time of the event
	float EndTime;

	// Shows that the start time has been set
	bool bStartPropertySet;

	// Shows that the end time has been set
	bool bEndPropertySet;

private:
	// Create time property
	FORCEINLINE FSLOwlTriple CreateTimeProperty(const float Timestamp)
	{
		// Append "timepoint_"
		const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(Timestamp);
		// Return triple
		return FSLOwlTriple("knowrob:startTime", "rdf:resource",
			FSLUtils::FStringToChar("&log;" + TimepointStr));
	};
};

/**
* Definitions
*/
class FSLOwlDefinitions
{
public:
	// Constructor
	FSLOwlDefinitions(
		const FString Subj,
		const FString Pred,
		const TArray<FString>& Objs = TArray<FString>()) :
		Subject(Subj),
		Predicate(Pred),
		Objects(Objs)
	{
	};

	// Add property
	void AddObject(const FString Property) { Objects.Add(Property); };

	// Get properties
	TArray<FString>& GetObjects() { return Objects; };

	// Get namespace
	FString GetSubject() { return Subject; };

	// Get unique name
	FString GetPredicate() { return Predicate; };

	// Add to XML document
	void AddToDocument(rapidxml::xml_document<>* Doc, rapidxml::xml_node<>* ParentNode)
	{
		const char* Sub = FStringToChar(Subject);
		const char* Pred = FStringToChar(Predicate);
		for (const auto& ObjItr : Objects)
		{
			// Create Triple node
			rapidxml::xml_node<> *TripleNode = Doc->allocate_node(rapidxml::node_element, Sub, "");
			// Add predicate and object to Triple
			TripleNode->append_attribute(Doc->allocate_attribute(
				Pred, FStringToChar(ObjItr)));
			// Append triple to parent
			ParentNode->append_node(TripleNode);
		}
	}

protected:
	// Subject
	FString Subject;

	// Predicate
	FString Predicate;

	// Objects
	TArray<FString> Objects;
};
