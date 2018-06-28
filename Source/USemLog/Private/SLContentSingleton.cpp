// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContentSingleton.h"


TSharedPtr<FSLContentSingleton> FSLContentSingleton::StaticInstance;

// Constructor
FSLContentSingleton::FSLContentSingleton() : bIsInit(false)
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Destructor
FSLContentSingleton::~FSLContentSingleton()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Get singleton
FSLContentSingleton* FSLContentSingleton::GetInstance()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FSLContentSingleton());
	}
	return StaticInstance.Get();
}

// Delete instance
void FSLContentSingleton::DeleteInstance()
{
	StaticInstance.Reset();
}

// Init data
void FSLContentSingleton::Init()
{
	if (bIsInit) return;

	bIsInit = true;
}