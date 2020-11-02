// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQSemMap.h"
#include "Viz/SLVizSemMapManager.h"
#include "Knowrob/SLKnowrobManager.h"

// Show ids
#if WITH_EDITOR
// Called when a property is changed in the editor
void USLVizQSemMap::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQSemMap, bLoadButton))
	{
		bLoadButton = false;
		if (bOverwrite)
		{
			ArrayToLoadFromString.ParseIntoArray(Ids, TEXT(";"), true);
		}
		else
		{
			TArray<FString> NewIds;
			ArrayToLoadFromString.ParseIntoArray(NewIds, TEXT(";"), true);
			Ids.Append(NewIds);
		}
		ArrayToLoadFromString.Empty();
	}
}
#endif // WITH_EDITOR

void USLVizQSemMap::Execute(ASLKnowrobManager* KRManager)
{
	if (bAllIndividuals)
	{
		KRManager->GetVizSemMapManager()->SetAllIndividualsHidden(bHide);
	}
	else
	{
		KRManager->GetVizSemMapManager()->SetIndividualsHidden(Ids, bHide, bIterate, IterateInterval);
	}
	Super::Execute(KRManager);
}
