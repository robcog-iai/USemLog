// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataVis/SLDataVisQueries.h"

#if SL_WITH_DATA_VIS
#include "MongoQA.h"
#include "VizMarkerManager.h"
#include "VizWorldManager.h"
#endif //SL_WITH_DATA_VIS

#include "SLDataVisualizer.generated.h"

/**
 * Visualizes the logged data 
 */
UCLASS()
class USEMLOG_API USLDataVisualizer : public UObject
{
	GENERATED_BODY()
	
public:
	// Constructor
	USLDataVisualizer();

	// Destructor
	~USLDataVisualizer();

	// Init vis
	void Init(USLDataVisQueries* InQueries);

	// Start logger
	void Start(const FName& UserInputActionName);

	// Finish logger
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Setup the user input bindings
	bool SetupUserInput(const FName& UserInputActionName);

	// Execute the query type
	void Query();

private:
	// Clean world from unnecesary actors/components
	void CleanWorld();

	// Spawn marker viz manager actor
	bool SpawnVizMarkerManager();

	// Spawn world viz manager actor
	bool SpawnVizWorldManager();

#if SL_WITH_DATA_VIS
	// Pre-load workl states for the selected episode
	void PreLoadWorldStates(const TArray<FMQWorldStateFrame>& WorldStates);
#endif //SL_WITH_DATA_VIS

	/* Query cases */
	void EntityPoseQuery();
	void EntityTrajQuery();
	void BonePoseQuery();
	void BoneTrajQuery();
	void SkelPoseQuery();
	void SkelTrajQuery();
	void GazePoseQuery();
	void GazeTrajQuery();
	void WorldStateQuery();
	void AllWorldStatesQuery();

protected:
	// Set when initialized
	bool bIsInit;

	// Can be set after init
	bool bIsStarted;

	// Can be set after init, or start
	bool bIsFinished;

private:
	// Queries to be visualized
	USLDataVisQueries* VisQueries;

	// Current query index
	int32 QueryIdx;

	// Previous database
	FString PrevDBName;

	// Previous collection
	FString PrevCollName;

#if SL_WITH_DATA_VIS
	// Mongo query handler
	FMongoQA QAHandler;

	// Creates and keeps track of markers
	AVizMarkerManager* VizMarkerManager;

	// World visuals (highlits/clones) manager
	AVizWorldManager* VizWorldManager;
#endif //SL_WITH_DATA_VIS



	// TESTS
	void SMTest();

	void MarkerTests();
};
