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
	#if PLATFORM_LINUX
	  strcpy(cstr, str.c_str());
	#elif PLATFORM_WINDOWS
	  strcpy_s(cstr, str_length + 1, str.c_str());
	#endif
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
	// Default constr
	FSLOwlTriple() 
	{ 
		Subject = "";
		Predicate = "";
		Object = "";
	};

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

	// Get the subject as FString
	const FString GetRdfSubject() { return FString(ANSI_TO_TCHAR(Subject)); };

	// Get the predicate as FString
	const FString GetRdfPredicate() { return FString(ANSI_TO_TCHAR(Predicate)); };

	// Get the predicate as FString
	const FString GetRdfObject() { return FString(ANSI_TO_TCHAR(Object)); };

	// Get the predicate as FString
	const FString GetRdfValue() { return FString(ANSI_TO_TCHAR(Value)); };
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
		}
		else 
		{
			UE_LOG(SemLogEvent, Error, 
				TEXT(" !! split object %s into namespace %s and unique name %s"),
				*Obj, *ObjectNamespace, *ObjectUniqueName);
		}
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
	FString GetRdfObject() { return Object; };

	// Get properties
	TArray<FSLOwlTriple>& GetProperties() { return Properties; };

	// Get object namespace
	FString GetRdfObjectNamespace() { return ObjectNamespace; };

	// Get object unique name
	FString GetRdfObjectUniqueName() { return ObjectUniqueName; };

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
			const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(StartTime);
			StartTimeProperty = FSLOwlTriple("knowrob:startTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" + TimepointStr));
			Properties.Add(StartTimeProperty);
		}

		if (EndTime > -1.f)
		{
			const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(EndTime);
			EndTimeProperty = FSLOwlTriple("knowrob:endTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" + TimepointStr));
			Properties.Add(EndTimeProperty);
		}
	};

	// Set end time
	void SetStartTime(float NewStartTime) 
	{
		if (StartTime < 0.f)
		{
			StartTime = NewStartTime;
			// Append "timepoint_"
			const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(StartTime);
			StartTimeProperty = FSLOwlTriple("knowrob:startTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" + TimepointStr));
			Properties.Add(StartTimeProperty);
		}
	};

	// Get start time
	float GetStartTime() { return StartTime; };
	
	// Get start time property
	FSLOwlTriple GetStartTimeProperty() { return StartTimeProperty; };

	// Set end time
	void SetEndTime(float NewEndTime) 
	{
		if (EndTime < 0.f)
		{
			EndTime = NewEndTime;
			// Append "timepoint_"
			const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(EndTime);
			EndTimeProperty = FSLOwlTriple("knowrob:endTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" + TimepointStr));
			Properties.Add(EndTimeProperty);
		}
	};

	// Get end time
	float GetEndTime() { return EndTime; };

	// Get end time property
	FSLOwlTriple GetEndTimeProperty() { return EndTimeProperty; };


protected:
	// Start time of the event
	float StartTime;

	// End time of the event
	float EndTime;

	// Start time property
	FSLOwlTriple StartTimeProperty;

	// End time property
	FSLOwlTriple EndTimeProperty;
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
	TArray<FString>& GetRdfObjects() { return Objects; };

	// Get namespace
	FString GetRdfSubject() { return Subject; };

	// Get unique name
	FString GetRdfPredicate() { return Predicate; };

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
