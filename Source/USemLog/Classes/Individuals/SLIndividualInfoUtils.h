// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class USLIndividualInfoComponent;
class ASLIndividualManager;

/**
 * Static helpers functions the semantic individual annotation
 */
class USEMLOG_API FSLIndividualInfoUtils
{
public:
	/* Individuals Info */
	static class ASLIndividualInfoManager* GetOrCreateNewIndividualInfoManager(UWorld* World, bool bCreateNew = true);
	static int32 CreateIndividualInfoComponents(UWorld* World);
	static int32 CreateIndividualInfoComponents(const TArray<AActor*>& Actors);
	static int32 ClearIndividualInfoComponents(UWorld* World);
	static int32 ClearIndividualInfoComponents(const TArray<AActor*>& Actors);
	static int32 InitIndividualInfoComponents(UWorld* World, bool bReset);
	static int32 InitIndividualInfoComponents(const TArray<AActor*>& Actors, bool bReset);
	static int32 LoadIndividualInfoComponents(UWorld* World, bool bReset);
	static int32 LoadIndividualInfoComponents(const TArray<AActor*>& Actors, bool bReset);
	static int32 ConnectIndividualInfoComponents(UWorld* World);
	static int32 ConnectIndividualInfoComponents(const TArray<AActor*>& Actors);

	/* Functionalities */
	static int32 ToggleIndividualInfoComponentsVisibilty(UWorld* World);
	static int32 ToggleIndividualInfoComponentsVisibilty(const TArray<AActor*>& Actors);
private:
	/* Individuals Info Private */
	static USLIndividualInfoComponent* AddNewIndividualInfoComponent(AActor* Actor, bool bTryInitAndLoad = false);
	static bool CanHaveIndividualInfoComponent(AActor* Actor);
	static bool HasIndividualInfoComponent(AActor* Actor);
	static bool ClearIndividualInfoComponent(AActor* Actor);
	static bool InitIndividualInfoComponent(AActor* Actor, bool bReset);
	static bool LoadIndividualInfoComponent(AActor* Actor, bool bReset);
	static bool ConnectIndividualInfoComponent(AActor* Actor);

	/* Functionalities Private */
	static bool ToggleIndividualInfoComponentVisibilty(AActor* Actor);
};
