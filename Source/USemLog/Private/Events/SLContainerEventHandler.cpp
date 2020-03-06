// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContainerEventHandler.h"
#include "SLEntitiesManager.h"
#include "SLContainerListener.h"
#include "Events/SLContainerEvent.h"

// UUtils
#include "Ids.h"


// Set parent
void FSLContainerEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Make sure the mappings singleton is initialized (the handler uses it)
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(InParent->GetWorld());
		}

		// Check if parent is of right type
		Parent = Cast<USLContainerListener>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLContainerEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe to the forwarded semantically annotated grasping broadcasts
		Parent->OnContainerManipulation.AddRaw(this, &FSLContainerEventHandler::OnContainerManipulation);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLContainerEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{

		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Event called when a semantic grasp happens
void FSLContainerEventHandler::OnContainerManipulation(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime, const FString& Type)
{
	// Check that the objects are semantically annotated
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLContainerEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem, Type)));
	}
}
