// Copyright 2020 Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#pragma once

#include "CoreMinimal.h"

/**
* Abstract class to handle Prolog Queries;
*/
class FSLQueryHandler
{
public:

	// Default constructor
	FSLQueryHandler() { SolutionsCount = 0; };
	
	// Init constructor
	FSLQueryHandler(FString InQuery, bool InbExhaustSolutions, int InSolutionsLimit = 20)
		: Query(InQuery), bExhaustSolutions(InbExhaustSolutions), SolutionsLimit(InSolutionsLimit) { SolutionsCount = 0; };

	// Virtual destructor
	virtual ~FSLQueryHandler() {};

	// Virtual Callback Function
	virtual void SolutionCallback(int status, FString Msg) {
		UE_LOG(LogTemp, Warning, TEXT("Query: [%s] \n Status = %d, Msg = %s"), *Query, status, *Msg);
	};

public:

	// Recorded query
	FString Query;

	// Finish on first solution if false, otherwise get all solutions until status is 0
	bool bExhaustSolutions;

	// Count received solutions
	int SolutionsCount;

	// Limit the solutions to get
	int SolutionsLimit;

};