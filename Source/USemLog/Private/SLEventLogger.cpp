// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"

#include "Events/SLContactEventHandler.h"
#include "Events/SLSupportedByEventHandler.h"
#include "Events/SLFixationGraspEventHandler.h"
#include "Events/SLSlicingEventHandler.h"
#include "SLOwlExperimentStatics.h"
#include "SLContactBox.h"
#include "SLContactSphere.h"
#include "SLContactCapsule.h"
#include "SLManipulatorListener.h"
#include "SLGoogleCharts.h"

// UUtils
#include "Ids.h"

#if SL_WITH_MC_GRASP
#include "MCFixationGrasp.h"
#endif // SL_WITH_MC_GRASP

#if SL_WITH_SLICING
#include "SlicingBladeComponent.h"
#endif // SL_WITH_SLICING

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
	bool bInLogSlicingEvents,
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
		// rename FSLContactEventHandler,FSLSupportedByEventHandler,FSLFixationGraspEventHandler -> Events

		// Init all contact trigger handlers
		for (TObjectIterator<USLContactBox> Itr; Itr; ++Itr)
		{
			if (IsValidAndAnnotated(*Itr))
			{
				// Init the semantic overlap area
				Itr->Init();

				// Store the semantic overlap areas
				ContactBoxes.Emplace(*Itr);

				if (bInLogContactEvents)
				{
					// Create a contact event handler 
					TSharedPtr<FSLContactEventHandler> CEHandler = MakeShareable(new FSLContactEventHandler());
					CEHandler->Init(*Itr);
					if (CEHandler->IsInit())
					{
						EventHandlers.Add(CEHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
							*FString(__func__), __LINE__, *Itr->GetName());
					}
				}

				if (bInLogSupportedByEvents)
				{
					// Create a supported-by event handler
					TSharedPtr<FSLSupportedByEventHandler> SBEHandler = MakeShareable(new FSLSupportedByEventHandler());
					SBEHandler->Init(*Itr);
					if (SBEHandler->IsInit())
					{
						EventHandlers.Add(SBEHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
							*FString(__func__), __LINE__, *Itr->GetName());
					}
				}
			}
		}

		// Init fixation or normal grasp handlers
		if (bInLogGraspEvents || bInLogContactEvents)
		{
			// Init all grasp listeners
			for (TObjectIterator<USLManipulatorListener> Itr; Itr; ++Itr)
			{
				if (IsValidAndAnnotated(*Itr))
				{
					if (Itr->Init(bInLogGraspEvents, bInLogContactEvents))
					{
						GraspListeners.Emplace(*Itr);
						TSharedPtr<FSLGraspEventHandler> GEHandler = MakeShareable(new FSLGraspEventHandler());
						GEHandler->Init(*Itr);
						if (GEHandler->IsInit())
						{
							EventHandlers.Add(GEHandler);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
								*FString(__func__), __LINE__, *Itr->GetName());
						}

						// The grasp listener can also publish contact events
						if (bInLogContactEvents)
						{
							TSharedPtr<FSLManipulatorContactEventHandler> MCEHandler = MakeShareable(new FSLManipulatorContactEventHandler());
							MCEHandler->Init(*Itr);
							if (MCEHandler->IsInit())
							{
								EventHandlers.Add(MCEHandler);
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
									*FString(__func__), __LINE__, *Itr->GetName());
							}
						}
					}

				}
			}


#if SL_WITH_MC_GRASP
			// Init fixation grasp listeners
			for (TObjectIterator<UMCFixationGrasp> Itr; Itr; ++Itr)
			{
				if (IsValidAndAnnotated(*Itr))
				{
					// Create a grasp event handler 
					TSharedPtr<FSLFixationGraspEventHandler> FGEHandler = MakeShareable(new FSLFixationGraspEventHandler());
					FGEHandler->Init(*Itr);
					if (FGEHandler->IsInit())
					{
						EventHandlers.Add(FGEHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
							*FString(__func__), __LINE__, *Itr->GetName());
					}
				}
			}
#endif // SL_WITH_MC_GRASP
		}

		// Init Slicing handlers
		if (bInLogSlicingEvents)
		{
#if SL_WITH_SLICING
			for (TObjectIterator<USlicingBladeComponent> Itr; Itr; ++Itr)
			{
				// Make sure the object is in the world
				if (IsValidAndAnnotated(*Itr))
				{
					// Create a Slicing event handler 
					TSharedPtr<FSLSlicingEventHandler> SEHandler = MakeShareable(new FSLSlicingEventHandler());
					SEHandler->Init(*Itr);
					if (SEHandler->IsInit())
					{
						EventHandlers.Add(SEHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
							*FString(__func__), __LINE__, *Itr->GetName());
					}
				}
			}
#endif // SL_WITH_SLICING
		}

		if (bWriteMetadata)
		{
			MetadataWriter.Init(WriterParams);
		}

		// Mark as initialized
		bIsInit = true;
	}
}

// Check if the component is valid in the world and has a semantically annotated owner
bool USLEventLogger::IsValidAndAnnotated(UActorComponent* Comp)
{
	// Make sure the object is in the world
	if (!GetWorld()->ContainsActor(Comp->GetOwner()))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not from this world.."),
		//	*FString(__func__), __LINE__, *Comp->GetName());
		return false;
	}

	// Skip objects that do not have a semantically annotated ancestor
	if (!FSLEntitiesManager::GetInstance()->GetValidAncestor(Comp))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no semantically annotated ancestor.."),
		//	*FString(__func__), __LINE__, *Comp->GetName());
		return false;
	}
	return true;
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
		for (auto& SLContactBox : ContactBoxes)
		{
			SLContactBox->Start();
		}

		// Start the grasp listeners
		for (auto& SLManipulatorListener : GraspListeners)
		{
			SLManipulatorListener->Start();
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
		for (auto& SLContactBox : ContactBoxes)
		{
			SLContactBox->Finish(bForced);
		}
		ContactBoxes.Empty();

		// Finish the grasp listeners
		for (auto& SLManipulatorListener : GraspListeners)
		{
			SLManipulatorListener->Finish(bForced);
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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s::%d %s"), *FString(__func__), __LINE__, *Event->ToString()));
	//UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), *FString(__func__), __LINE__, *Event->ToString());
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
