// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualBase.h"

// Ctor
USLIndividualBase::USLIndividualBase()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t %s"), *FString(__FUNCTION__), __LINE__, *GetName());
}

// Dtor
USLIndividualBase::~USLIndividualBase()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d \t %s"), *FString(__FUNCTION__), __LINE__, *GetName());
}
