// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQBase.h"
#include "Knowrob/SLKnowrobManager.h"

#if WITH_EDITOR
#include "Editor.h"	// GEditor
#endif // WITH_EDITOR

// Public execute function
void USLVizQBase::Execute(ASLKnowrobManager* KRManager)
{
	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is set to be ignored, skipping execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!KRManager || !KRManager->IsValidLowLevel() || KRManager->IsPendingKillOrUnreachable() || !KRManager->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s knowrob manager is not valid/init, aborting execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (bExecuteChildrenFirst)
	{
		ExecuteChildren(KRManager);
		ExecuteImpl(KRManager);
	}
	else
	{
		ExecuteImpl(KRManager);
		ExecuteChildren(KRManager);
	}
}

#if WITH_EDITOR
// Execute function called from the editor, references need to be set manually
void USLVizQBase::ManualExecute()
{
	if (IsReadyForManualExecution())
	{
		Execute(KnowrobManager.Get());
	}
}

// Called when a property is changed in the editor
void USLVizQBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is set to be ignored, ignoring property change events.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQBase, bManualExecuteButton))
	{
		bManualExecuteButton = false;
		ManualExecute();
	}
}

// Check if the references are set for calling the execute function from the editor
bool USLVizQBase::IsReadyForManualExecution() const
{
	if (KnowrobManager.IsValid() 
		&& KnowrobManager->IsValidLowLevel() 
		&& !KnowrobManager->IsPendingKillOrUnreachable())
	{

		/*
		* World types info:
		*
		* GIsEditor						-  True if we are in the editor. (True also when Play in Editor (PIE), or Simulating in Editor (SIE))
		*
		* EWorldType::None				- An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels
		* EWorldType::Game				- The game world
		* EWorldType::Editor			- A world being edited in the editor
		* EWorldType::PIE				- A Play In Editor world
		* EWorldType::EditorPreview		- A preview world for an editor tool
		* EWorldType::GamePreview		- A preview world for a game
		* EWorldType::GameRPC			- A minimal RPC world for a game
		* EWorldType::Inactive			- An editor world that was loaded but not currently being edited in the level editor
		*
		* GEditor->PlayWorld				- A pointer to a UWorld that is the duplicated/saved-loaded to be played in with "Play From Here"
		* GEditor->bIsSimulatingInEditor	- True if we're Simulating In Editor, as opposed to Playing In Editor.  In this mode, simulation takes place right the level editing environment
		* GIsPlayInEditorWorld				- Whether GWorld points to the play in editor world
		* GWorld->HasBegunPlay()			- True if gamplay has started
		*/

		// If we are running in the editor, make sure we are running a duplicated world (not the editor active one)
		if (GIsEditor && !GEditor->PlayWorld)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s is in editor world, skipping manual execution.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return false;
		}
		
		//// Check that the referenced pointer is in the same world
		//if (GWorld && GWorld == KnowrobManager->GetWorld())
		//{
		//	UE_LOG(LogTemp, Log, TEXT("%s::%d %s's GWorld[%s;%p] == KnowrobManager->GetWorld()[%s;%p] .."),	*FString(__FUNCTION__), __LINE__,
		//		*GetName(), *GWorld->GetName(), GWorld, *KnowrobManager->GetWorld()->GetName(), KnowrobManager->GetWorld());
		//	return true;
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d %s's %s's GWorld[%s;%p] != KnowrobManager->GetWorld()[%s;%p] .."), *FString(__FUNCTION__), __LINE__,
		//		*GetName(), *GWorld->GetName(), GWorld, *KnowrobManager->GetWorld()->GetName(), KnowrobManager->GetWorld());
		//	return false;
		//}

		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's KnowrobManager's soft reference is not valid.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
}
#endif // WITH_EDITOR

// Execute batch command if any
void USLVizQBase::ExecuteChildren(ASLKnowrobManager* KRManager)
{
	for (const auto C : Children)
	{
		C->Execute(KRManager);
	}
}

// Virtual implementation of the execute function
void USLVizQBase::ExecuteImpl(ASLKnowrobManager* KRManager)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d %'s execution"), *FString(__FUNCTION__), __LINE__);
}
