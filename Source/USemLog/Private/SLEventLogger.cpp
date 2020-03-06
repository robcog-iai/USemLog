// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventLogger.h"
#include "Events/SLContactEventHandler.h"
#include "Events/SLManipulatorContactEventHandler.h"
//#include "Events/SLSupportedByEventHandler.h"
#include "Events/SLGraspEventHandler.h"
#include "Events/SLReachEventHandler.h"
#include "Events/SLPickAndPlaceEventsHandler.h"
#include "Events/SLContainerEventHandler.h"
#include "Monitors/SLContactShapeInterface.h"
#include "Monitors/SLManipulatorListener.h"
#include "Monitors/SLReachListener.h"
#include "Monitors/SLPickAndPlaceListener.h"
#include "Monitors/SLContainerListener.h"
#include "Events/SLGoogleCharts.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"

// OWL
#include "SLOwlExperimentStatics.h"

// UUtils
#include "Ids.h"
#include "SLEntitiesManager.h"

#if SL_WITH_MC_GRASP
#include "Events/SLFixationGraspEventHandler.h"
#include "MCGraspFixation.h"
#endif // SL_WITH_MC_GRASP

#if SL_WITH_SLICING
#include "Events/SLSlicingEventHandler.h"
#include "SlicingBladeComponent.h"
#endif // SL_WITH_SLICING

// Constructor
USLEventLogger::USLEventLogger()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bWriteTimelines = false;
}

// Destructor
USLEventLogger::~USLEventLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(-1.f, true);
	}
}

// Init Logger
void USLEventLogger::Init(ESLOwlExperimentTemplate TemplateType,
	const FSLEventWriterParams& WriterParams,
	bool bInLogContactEvents,
	bool bInLogSupportedByEvents,
	bool bInLogGraspEvents,
	bool bInPickAndPlaceEvents,
	bool bInLogSlicingEvents,
	bool bInWriteTimelines)
{
	if (!bIsInit)
	{
		LogDirectory = WriterParams.TaskId;
		EpisodeId = WriterParams.EpisodeId;
		OwlDocTemplate = TemplateType;
		bWriteTimelines = bInWriteTimelines;

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
		for (TObjectIterator<UShapeComponent> Itr; Itr; ++Itr)
		{
			//if (Itr->GetClass()->ImplementsInterface(USLContactShapeInterface::StaticClass()))
			if (ISLContactShapeInterface* ContactShape = Cast<ISLContactShapeInterface>(*Itr))
			{
				if (IsValidAndAnnotated(*Itr))
				{
					ContactShape->Init(bInLogSupportedByEvents);

					ContactShapes.Emplace(ContactShape);

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
			for (TObjectIterator<UMCGraspFixation> Itr; Itr; ++Itr)
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

			// Init all reach listeners
			for (TObjectIterator<USLReachListener> Itr; Itr; ++Itr)
			{
				if (IsValidAndAnnotated(*Itr))
				{
					if (Itr->Init())
					{
						ReachListeners.Emplace(*Itr);
						TSharedPtr<FSLReachEventHandler> REHandler = MakeShareable(new FSLReachEventHandler());
						REHandler->Init(*Itr);
						if (REHandler->IsInit())
						{
							EventHandlers.Add(REHandler);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
								*FString(__func__), __LINE__, *Itr->GetName());
						}
					}

				}
			}

			// Init all container manipulation listeners
			for (TObjectIterator<USLContainerListener> Itr; Itr; ++Itr)
			{
				if (IsValidAndAnnotated(*Itr))
				{
					if (Itr->Init())
					{
						ContainerListeners.Emplace(*Itr);
						TSharedPtr<FSLContainerEventHandler> Handler = MakeShareable(new FSLContainerEventHandler());
						Handler->Init(*Itr);
						if (Handler->IsInit())
						{
							EventHandlers.Add(Handler);
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

		if(bInPickAndPlaceEvents)
		{
			// Init all pick and place listeners
			for (TObjectIterator<USLPickAndPlaceListener> Itr; Itr; ++Itr)
			{
				if (IsValidAndAnnotated(*Itr))
				{
					if (Itr->Init())
					{
						PickAndPlaceListeners.Emplace(*Itr);
						TSharedPtr<FSLPickAndPlaceEventsHandler> PAPHandler = MakeShareable(new FSLPickAndPlaceEventsHandler());
						PAPHandler->Init(*Itr);
						if (PAPHandler->IsInit())
						{
							EventHandlers.Add(PAPHandler);
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

		// Mark as initialized
		bIsInit = true;
	}
}

// Check if the component is valid in the world and has a semantically annotated owner
bool USLEventLogger::IsValidAndAnnotated(UActorComponent* Comp) const
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
		for (auto& Listener : ContactShapes)
		{
			Listener->Start();
		}

		// Start the grasp listeners
		for (auto& Listener : GraspListeners)
		{
			Listener->Start();
		}

		// Start the pick and place listeners
		for (auto& Listener : PickAndPlaceListeners)
		{
			Listener->Start();
		}

		// Start the reach listeners
		for (auto& Listener : ReachListeners)
		{
			Listener->Start();
		}

		// Start the container listeners
		for (auto& Listener : ContainerListeners)
		{
			Listener->Start();
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
		for (auto& SLContactShape : ContactShapes)
		{
			SLContactShape->Finish();
		}
		ContactShapes.Empty();

		// Finish the grasp listeners
		for (auto& SLManipulatorListener : GraspListeners)
		{
			SLManipulatorListener->Finish(bForced);
		}
		GraspListeners.Empty();

		// Start the reach listeners
		for (auto& SLReachListener : ReachListeners)
		{
			SLReachListener->Start();
		}
		ReachListeners.Empty();

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
		WriteToFile();

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
	FString FullFilePath = FPaths::ProjectDir() + "/SemLog/" +
		LogDirectory /*+ TEXT("/Episodes/")*/+ "/" + EpisodeId + TEXT("_ED.owl");
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
