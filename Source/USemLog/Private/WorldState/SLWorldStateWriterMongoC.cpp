// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterMongoC.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"
#if SL_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START

THIRD_PARTY_INCLUDES_END
#endif //SL_WITH_LIBMONGO

// Constr
FSLWorldStateWriterMongoC::FSLWorldStateWriterMongoC()
{
	bIsInit = false;
}

FSLWorldStateWriterMongoC::FSLWorldStateWriterMongoC(const FSLWorldStateWriterParams& InParams)
{
	bIsInit = false;
	FSLWorldStateWriterMongoC::Init(InParams);
}


// Destr
FSLWorldStateWriterMongoC::~FSLWorldStateWriterMongoC()
{
}

// Init
void FSLWorldStateWriterMongoC::Init(const FSLWorldStateWriterParams& InParams)
{
	MinLinearDistanceSquared = InParams.LinearDistanceSquared;
	MinAngularDistance = InParams.AngularDistance;
	bIsInit = FSLWorldStateWriterMongoC::Connect(InParams.Location, InParams.EpisodeId, InParams.ServerIp, InParams.ServerPort);
}

// Finish
void FSLWorldStateWriterMongoC::Finish()
{
	if (bIsInit)
	{
		FSLWorldStateWriterMongoC::CreateIndexes();
		bIsInit = false;
	}
}

// Called to write the data
void FSLWorldStateWriterMongoC::Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	float Timestamp)
{
#if SL_WITH_LIBMONGO
#endif //SL_WITH_LIBMONGO
}

// Connect to the database
bool FSLWorldStateWriterMongoC::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Params: DBName=%s; Collection=%s; IP=%s; Port=%d;"),
		TEXT(__FUNCTION__), __LINE__, *DBName, *EpisodeId, *ServerIp, ServerPort);
#if SL_WITH_LIBMONGO
	return false;
#else
	return false;
#endif //SL_WITH_LIBMONGO
}

// Create indexes from the logged data, usually called after logging
bool FSLWorldStateWriterMongoC::CreateIndexes()
{
	if (!bIsInit)
	{
		return false;
	}
#if SL_WITH_LIBMONGO
	return false;
#else
	return false;
#endif //SL_WITH_LIBMONGO
}

#if SL_WITH_LIBMONGO

#endif //SL_WITH_LIBMONGO
