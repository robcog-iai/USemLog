// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
 * Singleton storing Map between the unreal objects and the semantic data
 */
class USEMLOGMAP_API FSLMap
{
private:
	// Constructor
	FSLMap();

public:
	// Destructor
	~FSLMap();

	// Get singleton
	static FSLMap* GetInstance();

	// Delete instance
	static void DeleteInstance();

	// Init data / load Map
	void LoadData(UWorld* World);

	// Get semantic id from unique id
	FString GetSemanticId(uint32 UniqueId) const;

	// Get semantic class from unique id
	FString GetSemanticClass(uint32 UniqueId) const;

	// Check if data is loaded
	bool IsInit() const { return bIsInit; }

private:
	// Instance of the singleton
	static TSharedPtr<FSLMap> StaticInstance;

	// Flag showing the data has been init
	bool bIsInit;

	// Unreal unique id to SemLog unique id map
	TMap<uint32, FString> IdSemIdMap;

	// Unreal unique id to semantic class map
	TMap<uint32, FString> IdClassMap;
};
