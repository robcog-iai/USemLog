// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
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
#include "SLGraspListener.h"
#include "SLGoogleCharts.h"

// UUtils
#include "Ids.h"

#if SL_WITH_MC_GRASP
#include "MCFixationGrasp.h"
#endif // SL_WITH_MC_GRASP


// Constructor
USLEventLogger::USLEventLogger()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bWriteMetadata = false;
	bWriteTimelines = false;
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
	const FSLEventWriterParams& WriterParams,
	bool bInLogContactEvents,
	bool bInLogSupportedByEvents,
	bool bInLogGraspEvents,
	bool bInWriteTimelines,
	bool bInWriteMetadata)
{
	if (!bIsInit)
	{
		LogDirectory = WriterParams.Location;
		EpisodeId = WriterParams.EpisodeId;
		OwlDocTemplate = TemplateType;
		bWriteTimelines = bInWriteTimelines;
		bWriteMetadata = bInWriteMetadata;

		// Init the semantic mappings (if not already init)
		FSLEntitiesManager::GetInstance()->Init(GetWorld());

		// Create the document template
		ExperimentDoc = CreateEventsDocTemplate(TemplateType, EpisodeId);

		// TODO create one handler for each event type
		// bind all the objects to one handler
		// Instead of Init -> AddParent
		// Parent -> TArray Parents
		// rename FSLContactEventHandler,FSLSupportedByEventHandler,FSLGraspEventHandler -> Events

		// Init all contact trigger handlers
		for (TObjectIterator<USLOverlapShape> Itr; Itr; ++Itr)
		{
			// Make sure the object is in the world
			if (!GetWorld()->ContainsActor(Itr->GetOwner()))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not from this world.."),
					*FString(__func__), __LINE__, *Itr->GetName());
				continue;
			}

			// Skip objects that do not have a semantically annotated ancestor
			if (!FSLEntitiesManager::GetInstance()->GetValidAncestor(*Itr))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no semantically annotated ancestor.."),
					*FString(__func__), __LINE__, *Itr->GetName());
				continue;
			}

			// Init the semantic overlap area
			Itr->Init();

			// Store the semantic overlap areas
			OverlapShapes.Emplace(*Itr);

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

		// Init all grasp listeners
		for (TObjectIterator<USLGraspListener> Itr; Itr; ++Itr)
		{
			// Make sure the object is in the world
			if (!GetWorld()->ContainsActor(Itr->GetOwner()))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not from this world.."),
					*FString(__func__), __LINE__, *Itr->GetName());
				continue;
			}

			// Skip objects that do not have a semantically annotated ancestor
			if (!FSLEntitiesManager::GetInstance()->GetValidAncestor(*Itr))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no semantically annotated ancestor.."),
					*FString(__func__), __LINE__, *Itr->GetName());
				continue;
			}
			
			if (Itr->Init())
			{
				GraspListeners.Emplace(*Itr);
			}
		}

		// Init grasp handlers
		if (bInLogGraspEvents)
		{
#if SL_WITH_MC_GRASP
			for (TObjectIterator<UMCFixationGrasp> Itr; Itr; ++Itr)
			{
				// Make sure the object is in the world
				if (!GetWorld()->ContainsActor(Itr->GetOwner()))
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not from this world.."),
						*FString(__func__), __LINE__, *Itr->GetName());
					continue;
				}

				// Skip objects that do not have a semantically annotated ancestor
				if (!FSLEntitiesManager::GetInstance()->GetValidAncestor(*Itr))
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no semantically annotated ancestor.."),
						*FString(__func__), __LINE__, *Itr->GetName());
					continue;
				}

				// Create a grasp event handler 
				TSharedPtr<FSLGraspEventHandler> GraspEventHandler = MakeShareable(new FSLGraspEventHandler());
				GraspEventHandler->Init(*Itr);
				EventHandlers.Add(GraspEventHandler);
			}
#endif // SL_WITH_MC_GRASP
		}

		if (bWriteMetadata)
		{
			MetadataWriter.Init(WriterParams);
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
		for (auto& SLOverlapShape : OverlapShapes)
		{
			SLOverlapShape->Start();
		}

		// Start the grasp listeners
		for (auto& SLGraspListener : GraspListeners)
		{
			SLGraspListener->Start();
		}

		if (bWriteMetadata)
		{
			MetadataWriter.Start();
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
		for (auto& SLOverlapShape : OverlapShapes)
		{
			SLOverlapShape->Finish(bForced);
		}
		OverlapShapes.Empty();

		// Finish the grasp listeners
		for (auto& SLGraspListener : GraspListeners)
		{
			SLGraspListener->Finish(bForced);
		}
		GraspListeners.Empty();

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
		if (bWriteMetadata)
		{
			MetadataWriter.Finish();
		}

		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Called when a semantic event is done
void USLEventLogger::OnSemanticEvent(TSharedPtr<ISLEvent> Event)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s::%d %s"), *FString(__func__), __LINE__, *Event->ToString()));
	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), *FString(__func__), __LINE__, *Event->ToString());
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
