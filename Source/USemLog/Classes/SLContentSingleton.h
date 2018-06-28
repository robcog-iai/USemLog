// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
 * Class storing and quickly accessing semantically annotated items
 */
class USEMLOG_API FSLContentSingleton
{
private:
	// Constructor
	FSLContentSingleton();

public:
	// Destructor
	~FSLContentSingleton();

	// Get singleton
	static FSLContentSingleton* GetInstance();

	// Delete instance
	static void DeleteInstance();
	
	// Init data
	void Init();

	// Check if data is already initialized
	bool IsInit() { return bIsInit; };

private:
	// Instance of the singleton
	static TSharedPtr<FSLContentSingleton> StaticInstance;

	// True if the data has been initialized
	bool bIsInit;

	// Data
	TArray<AActor*> Items;

	// Map of actors to unique ids
	TMap<AActor*, FString> ActorToIdMap;
};
