// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLVizQBase.generated.h"

// Forward declaration
class ASLKnowrobManager;

/**
 * Base class for viz queries
 */
UCLASS()
class USLVizQBase : public UDataAsset
{
	GENERATED_BODY()

public:
	// Public execute function
	void Execute(ASLKnowrobManager* KRManager);

protected:
#if WITH_EDITOR
	// Execute function called from the editor, references need to be set manually
	void ManualExecute();

	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// Check if the references are set for calling the execute function from the editor
	bool ReadyForManualExecution() const;
#endif // WITH_EDITOR

	// Execute batch command if any
	void ExecuteChildren(ASLKnowrobManager* KRManager);

	// Virtual implementation of the execute function
	virtual void ExecuteImpl(ASLKnowrobManager* KRManager);

protected:
	/* Children to be called in a batch */
	UPROPERTY(EditAnywhere, Category = "Children")
	TArray<USLVizQBase*> Children;

	UPROPERTY(EditAnywhere, Category = "Children")
	bool bExecuteChildrenFirst = false;


	/* Manual interaction properties button */
	UPROPERTY(EditAnywhere, Category = "Manual Interaction")
	TSoftObjectPtr<ASLKnowrobManager> KnowrobManager = nullptr;

	UPROPERTY(EditAnywhere, Category = "Manual Interaction")
	bool bManualExecuteButton = false;


	/* Description of the query */
	UPROPERTY(EditAnywhere, Category = "Description")
	FString Description;
};



///////////////////////////////////////// RM
///**
// *
// */
//UCLASS()
//class USLVizQGotoFrame : public USLVizQBase
//{
//	GENERATED_BODY()
//
//public:
//	virtual void Execute(ASLKnowrobManager* KRManager) override;
//
//private:
//	UPROPERTY(EditAnywhere, Category = "Id")
//	FString EpisodeId;
//
//	// For negative value it will start from first frame
//	UPROPERTY(EditAnywhere, Category = "Time")
//	float Timestamp = -1.f;
//};
//
///**
// *
// */
//UCLASS()
//class USLVizQReplayEpisode : public USLVizQBase
//{
//	GENERATED_BODY()
//
//public:
//	virtual void Execute(ASLKnowrobManager* KRManager) override;
//
//private:
//	UPROPERTY(EditAnywhere, Category = "Id")
//	FString EpisodeId;
//
//	// For negative value it will start from first frame
//	UPROPERTY(EditAnywhere, Category = "Time")
//	float StartTime = -1.f;
//
//	// For negative value it will run until the last frame
//	UPROPERTY(EditAnywhere, Category = "Time")
//	float EndTime = -1.f;
//
//	// Repeat replay after finishing
//	UPROPERTY(EditAnywhere, Category = "Properties")
//	bool bLoop = true;
//
//	// How quickly to move to the next frame (if negative, it will calculate an average update rate from the episode data)
//	UPROPERTY(EditAnywhere, Category = "Properties")
//	float UpdateRate = -1.f;
//
//	// How many steps to update every frame
//	UPROPERTY(EditAnywhere, Category = "Properties")
//	int32 StepSize = 1;
//};
//
//
//
///**
// *
// */
//UCLASS()
//class USLVizQCacheEpisodes : public USLVizQBase
//{
//	GENERATED_BODY()
//
//public:
//	virtual void Execute(ASLKnowrobManager* KRManager) override;
//
//private:
//	UPROPERTY(EditAnywhere, Category = "VizQ")
//	FString TaskId;
//
//	UPROPERTY(EditAnywhere, Category = "VizQ")
//	TArray<FString> EpisodeIds;
//};
//
