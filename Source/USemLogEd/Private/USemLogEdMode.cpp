// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "USemLogEd.h"
#include "USemLogEdMode.h"
#include "USemLogEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"

const FEditorModeID FUSemLogEdMode::EM_USemLogEdModeId = TEXT("EM_USemLogEdMode");

FUSemLogEdMode::FUSemLogEdMode()
{

}

FUSemLogEdMode::~FUSemLogEdMode()
{

}

void FUSemLogEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FUSemLogEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	UE_LOG(LogTemp, Warning, TEXT(" ** USemLogEd Enter"));
}

void FUSemLogEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();

	UE_LOG(LogTemp, Warning, TEXT(" ** USemLogEd Exit"));
}

bool FUSemLogEdMode::UsesToolkits() const
{
	return true;
}




