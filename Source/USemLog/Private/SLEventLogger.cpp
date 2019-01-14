// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"

#include "Events/SLContactEventHandler.h"
#include "Events/SLSupportedByEventHandler.h"
#include "Events/SLGraspEventHandler.h"
#include "SLOwlExperimentStatics.h"
#include "SLOverlapShape.h"
#include "SLGoogleCharts.h"

// UUtils
#include "Ids.h"

#if WITH_MC_GRASP
#include "MCFixationGrasp.h"
#endif // WITH_MC_GRASP


// Constructor
USLEventLogger::USLEventLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
}

// Destructor
USLEventLogger::~USLEventLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		USLEventLogger::Finish(-1.f, true);
	}
}

// Init Logger
void USLEventLogger::Init(ESLOwlExperimentTemplate TemplateType,
	const FString& InLocation,
	const FString& InEpisodeId,
	bool bInLogContactEvents,
	bool bInLogSupportedByEvents,
	bool bInLogGraspEvents,
	bool bInWriteTimelines)
{
	if (!bIsInit)
	{
		LogDirectory = InLocation;
		EpisodeId = InEpisodeId;
		OwlDocTemplate = TemplateType;
		bWriteTimelines = bInWriteTimelines;

		// Init the semantic mappings (if not already init)
		FSLMappings::GetInstance()->Init(GetWorld());

		// Create the document template
		ExperimentDoc = CreateEventsDocTemplate(TemplateType, InEpisodeId);

		// TODO create one handler for each event type
		// bind all the objects to one handler
		// Instead of Init -> AddParent
		// Parent -> TArray Parents
		// rename FSLContactEventHandler,FSLSupportedByEventHandler,FSLGraspEventHandler -> Events

		// Init all contact trigger handlers
		for (TObjectIterator<USLOverlapShape> Itr; Itr; ++Itr)
		{
			// Skip objects that do not have a semantically annotated ancestor
			if (!FSLMappings::GetInstance()->HasValidAncestor(*Itr))
			{
				continue;
			}

			// Init the semantic overlap area
			Itr->Init();

			// Store the semantic overlap areas
			SemanticOverlapAreas.Add(*Itr);

			if (bInLogContactEvents)
			{
				// Create a contact event handler 
				TSharedPtr<FSLContactEventHandler> ContactEventHandler = MakeShareable(new FSLContactEventHandler());
				ContactEventHandler->Init(*Itr);
				EventHandlers.Add(ContactEventHandler);
			}

			if (bInLogSupportedByEvents)
			{
				// Create a supported-by event handler
				TSharedPtr<FSLSupportedByEventHandler> SupportedByEventHandler = MakeShareable(new FSLSupportedByEventHandler());
				SupportedByEventHandler->Init(*Itr);
				EventHandlers.Add(SupportedByEventHandler);
			}
		}

		// Init grasp handlers
		if (bInLogGraspEvents)
		{
#if WITH_MC_GRASP
			for (TObjectIterator<UMCFixationGrasp> Itr; Itr; ++Itr)
			{
				// Skip objects that do not have a semantically annotated ancestor
				if (!FSLMappings::GetInstance()->HasValidAncestor(*Itr))
				{
					continue;
				}

				// Create a grasp event handler 
				TSharedPtr<FSLGraspEventHandler> GraspEventHandler = MakeShareable(new FSLGraspEventHandler());
				GraspEventHandler->Init(*Itr);
				EventHandlers.Add(GraspEventHandler);
			}
#endif // WITH_MC_GRASP
		}

		// Mark as initialized
		bIsInit = true;
	}
}

// Start logger
void USLEventLogger::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Start handlers
		for (auto& EvHandler : EventHandlers)
		{
			// Subscribe for given semantic events
			EvHandler->Start();

			// Bind resulting events
			EvHandler->OnSemanticEvent.BindUObject(
				this, &USLEventLogger::OnSemanticEvent);
		}

		// Start the semantic overlap areas
		for (auto& SLOverlapShape : SemanticOverlapAreas)
		{
			SLOverlapShape->Start();
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Finish logger
void USLEventLogger::Finish(const float Time, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Finish handlers pending events
		for (auto& EvHandler : EventHandlers)
		{
			EvHandler->Finish(Time, bForced);
		}
		EventHandlers.Empty();

		// Finish semantic overlap events publishing
		for (auto& SLOverlapShape : SemanticOverlapAreas)
		{
			SLOverlapShape->Finish(bForced);
		}
		SemanticOverlapAreas.Empty();

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;

		// Create the experiment owl doc
		if (!ExperimentDoc.IsValid())
			return;

		// Add finished events to doc
		for (const auto& Ev : FinishedEvents)
		{
			Ev->AddToOwlDoc(ExperimentDoc.Get());
		}

		// Add stored unique timepoints to doc
		ExperimentDoc->AddTimepointIndividuals();

		// Add stored unique objects to doc
		ExperimentDoc->AddObjectIndividuals();

		// Add experiment individual to doc
		ExperimentDoc->AddExperimentIndividual();

		// Write events to file
		USLEventLogger::WriteToFile();
	}
}

// Called when a semantic event is done
void USLEventLogger::OnSemanticEvent(TSharedPtr<ISLEvent> Event)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->ToString()));
	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->ToString());
	FinishedEvents.Add(Event);
}

// Write to file
bool USLEventLogger::WriteToFile()
{
	// Write events timelines to file
	if (bWriteTimelines)
	{
		FSLGoogleChartsParameters Params;
		Params.bTooltips = true;
		FSLGoogleCharts::WriteTimelines(FinishedEvents, LogDirectory, EpisodeId, Params);
	}

	if (!ExperimentDoc.IsValid())
		return false;

	// Write experiment to file
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/Episodes/") + EpisodeId + TEXT("_ED.owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(ExperimentDoc->ToString(), *FullFilePath);
}

// Create events doc (experiment) template
TSharedPtr<FSLOwlExperiment> USLEventLogger::CreateEventsDocTemplate(ESLOwlExperimentTemplate TemplateType, const FString& InDocId)
{
	// Create unique semlog id for the document
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	// Fill document with template values
	if (TemplateType == ESLOwlExperimentTemplate::Default)
	{
		return FSLOwlExperimentStatics::CreateDefaultExperiment(DocId);
	}
	else if (TemplateType == ESLOwlExperimentTemplate::IAI)
	{
		return FSLOwlExperimentStatics::CreateUEExperiment(DocId);
	}
	return MakeShareable(new FSLOwlExperiment());
}
