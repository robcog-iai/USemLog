// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Demo/SLDataVisQueries.h"

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
	void Init(const TArray<USLDataVisQueries*>& InQueries);

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
	// Query trigger
	void Trigger();

	// Setup the user input bindings
	bool SetupUserInput(const FName& UserInputActionName);

	// Execute the query type
	bool SetNextQuery();

	// Set the next query from the array
	bool SetNextActiveQueries();

private:
	// Clean world from unnecesary actors/components
	void CleanWorld();

	// Spawn marker viz manager actor
	bool SpawnVizMarkerManager();

	// Spawn world viz manager actor
	bool SpawnVizWorldManager();

	// Execute active query
	void ExecuteQuery(const FSLVisQuery& Query);

#if SL_WITH_DATA_VIS
	// Pre-load workl states for the selected episode
	void PreLoadWorldStates(const TArray<FMQWorldStateFrame>& WorldStates);

	// Convert from EVizMarkerTypeVQ to EVizMarkerType
	FORCEINLINE EVizMarkerType ToOrigPrimitiveMarkerType(EVizMarkerTypeVQ VQType);
#endif //SL_WITH_DATA_VIS

	/* Query cases */
	void EntityPoseQuery(const FSLVisQuery& Query);
	void EntityTrajQuery(const FSLVisQuery& Query);
	void BonePoseQuery(const FSLVisQuery& Query);
	void BoneTrajQuery(const FSLVisQuery& Query);
	void SkelPoseQuery(const FSLVisQuery& Query);
	void SkelTrajQuery(const FSLVisQuery& Query);
	void GazePoseQuery(const FSLVisQuery& Query);
	void GazeTrajQuery(const FSLVisQuery& Query);
	void WorldStateQuery(const FSLVisQuery& Query);
	void AllWorldStatesQuery(const FSLVisQuery& Query);

	/* Action cases */

protected:
	// Set when initialized
	bool bIsInit;

	// Can be set after init
	bool bIsStarted;

	// Can be set after init, or start
	bool bIsFinished;

private:
	// Queries to be visualized
	USLDataVisQueries* ActiveQueries;

	// Array of queries
	TArray<USLDataVisQueries*> QueriesArray;

	// Current query array index
	int32 QueryArrayIdx;

	// Current query index
	int32 ActiveQueryIdx;

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
