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
		const TArray<FSLUtils::SLOwlTriple>& Prop = TArray<FSLUtils::SLOwlTriple>()) :
		Namespace(Ns),
		UniqueName(UniqName),
		Properties(Prop)
	{
	};

	// Constructor with object name (namespace + unqiue name)
	FSLOwlObjectIndividual(
		const FString ObjectName,
		const TArray<FSLUtils::SLOwlTriple>& Prop = TArray<FSLUtils::SLOwlTriple>()) :
		Properties(Prop)
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

protected:
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
