// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Private/SLUtils.h"

/**
* ObjectIndividual
*/
class FSLOwlObjectIndividual
{
public:
	// Constructor
	FSLOwlObjectIndividual(
		const FString Ns,
		const FString UniqName,
		const TArray<FSLUtils::SLOwlTriple>& Prop = TArray<FSLUtils::SLOwlTriple>(),
		const FString Subj = "owl:NamedIndividual",
		const FString Pred = "rdf:about") :
		Namespace(Ns),
		UniqueName(UniqName),
		Properties(Prop),
		Subject(Subj),
		Predicate(Pred)
	{
	};

	// Constructor with object name (namespace + unqiue name)
	FSLOwlObjectIndividual(
		const FString ObjectName,
		const TArray<FSLUtils::SLOwlTriple>& Prop = TArray<FSLUtils::SLOwlTriple>(),
		const FString Subj = "owl:NamedIndividual",
		const FString Pred = "rdf:about") :
		Properties(Prop),
		Subject(Subj),
		Predicate(Pred)
	{
		if (!ObjectName.Split(";", &Namespace, &UniqueName))
		{
			UE_LOG(SemLogEvent, Error, TEXT(" !! Could not split object %s into namespace and unique name."),
				*ObjectName);
		};
		// Namespace +=";";
	};

	// Add property
	void AddProperty(FSLUtils::SLOwlTriple Property) { Properties.Add(Property); };

	// Get properties
	TArray<FSLUtils::SLOwlTriple>& GetProperties() { return Properties; };

	// Get namespace
	FString GetNamespace() { return Namespace; };

	// Get unique name
	FString GetUniqueName() { return UniqueName; };

	// Add to document node
	void AddToDocumentNode(rapidxml::xml_document<>* Doc, rapidxml::xml_node<>* ParentNode)
	{
		// TODO write to node here, move all xml utils here, SemLogger and SemMapExporter only includes SLOwleEntities.h
	}

protected:
	// Subject
	FString Subject;

	// Predicate
	FString Predicate;

	// Named individual namsepace
	FString Namespace;

	// Named individual unique name
	FString UniqueName;

	// Named individual properties
	TArray<FSLUtils::SLOwlTriple> Properties;
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
		const TArray<FSLUtils::SLOwlTriple>& Prop = TArray<FSLUtils::SLOwlTriple>()) :
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
	void SetStartTime(float NewStartTime) { StartTime = StartTime; };

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
	FORCEINLINE FSLUtils::SLOwlTriple CreateTimeProperty(const float Timestamp)
	{
		// Append "timepoint_"
		const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(Timestamp);
		// Return triple
		return FSLUtils::SLOwlTriple("knowrob:startTime", "rdf:resource",
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

protected:
	// Subject
	FString Subject;

	// Predicate
	FString Predicate;

	// Objects
	TArray<FString> Objects;
};

/**
* OwlTriple // TODO remove dependency of SLUtils (have all OWL entities here)
*/
//struct SLOwlTriple
//{
//	SLOwlTriple(const char* Subj, const char* Pred, const char* Obj, const char* Val = "")
//		: Subject(Subj), Predicate(Pred), Object(Obj), Value(Val)
//	{}
//	const char* Subject;
//	const char* Predicate;
//	const char* Object;
//	const char* Value;
//};