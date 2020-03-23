// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLDataComponent.h"

// Sets default values for this component's properties
USLDataComponent::USLDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USLDataComponent::BeginPlay()
{
	Super::BeginPlay();
	ToString();
	// ...
	
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLDataComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Logger Properties */
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bUseCustomEpisodeId))
	//{
	//	if (bUseCustomEpisodeId) { EpisodeId = FIds::NewGuidInBase64Url(); }
	//	else { EpisodeId = TEXT(""); };
	//}

}
FString USLDataComponent::ToString() const
{
	for (TFieldIterator<UProperty> It(GetClass()); It; ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_Transient))
		{
			continue;
		}

	

		UE_LOG(LogTemp, Warning, TEXT("%s::%d UP=%s"), *FString(__FUNCTION__), __LINE__, *It->GetName());
		//It->HasMetaData
	}
	return FString();
}
#endif // WITH_EDITOR

