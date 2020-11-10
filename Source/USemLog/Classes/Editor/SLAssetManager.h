// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Editor/SLAssetDBHandler.h"
#include "SLAssetManager.generated.h"

/**
 * Handles the storing and retrieving of the assets used in the given episode
 */
UCLASS()
class USLAssetManager : public UObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLAssetManager();

	// Dtor
	~USLAssetManager();

	// Init asset manager (connect to db, chech paths etc.)
	void Init(const FString& TaskId, const FString& ServerIp,
		uint16 ServerPort, ESLAssetAction Action, bool bOverwrite = false);

	// Start the action (move/upload/download assets)
	void Start();

	// Finish manger
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Move assets to folder
	bool MoveCurrentLevelAssets(const FString& Path);

	// return the current level name 
	FName GetCurrentLevel();

	// Get the referenced assets and move them
	void MoveReferencedObjects(FName PackageName, const FString& SourceBasePath, const FString& DestBasePath);

	// Move the assets to destination path
	void MoveAssets(TArray<FAssetData> AssetList, const FString& DestPath, const FString& SourcePath);

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// Database handler
	FSLAssetDBHandler DBHandler;

	// Action to be executed
	ESLAssetAction Action;

};
