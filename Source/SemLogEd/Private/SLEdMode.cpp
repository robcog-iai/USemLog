// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdMode.h"
#include "SLEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FSLEdMode::EM_SLEdModeId = TEXT("EM_SLEdMode");

FSLEdMode::FSLEdMode()
{

}

FSLEdMode::~FSLEdMode()
{

}

void FSLEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FSLEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FSLEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FSLEdMode::UsesToolkits() const
{
	return true;
}




