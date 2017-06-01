// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SemLogEdMode.h"
#include "SemLogEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

//#include "SemLogEdModule.h"
//#include "SemLogEdMode.h"
//#include "SemLogEdModeToolkit.h"
//#include "Toolkits/ToolkitManager.h"

const FEditorModeID FSemLogEdMode::EM_SemLogEdModeId = TEXT("EM_SemLogEdMode");

FSemLogEdMode::FSemLogEdMode()
{

}

FSemLogEdMode::~FSemLogEdMode()
{

}

void FSemLogEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FSemLogEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	UE_LOG(LogTemp, Warning, TEXT(" ** SemLogEd Enter"));
}

void FSemLogEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();

	UE_LOG(LogTemp, Warning, TEXT(" ** SemLogEd Exit"));
}

bool FSemLogEdMode::UsesToolkits() const
{
	return true;
}




