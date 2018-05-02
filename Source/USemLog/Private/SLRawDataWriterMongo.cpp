// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataWriterMongo.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"
//#include "bson.h"
//#include "mongoc.h"

// Constr
FSLRawDataWriterMongo::FSLRawDataWriterMongo()
{
}

// Constr with Init
FSLRawDataWriterMongo::FSLRawDataWriterMongo(FSLRawDataAsyncWorker* InWorkerParent,
	const FString& InLogDB,
	const FString& InEpisodeId,
	const FString& InMongoIP,
	uint16 MongoPort)
{
	Init(InWorkerParent, InLogDB, InEpisodeId, InMongoIP, MongoPort);
}

// Destr
FSLRawDataWriterMongo::~FSLRawDataWriterMongo()
{
	// Disconnect mongo
}

// Init
void FSLRawDataWriterMongo::Init(FSLRawDataAsyncWorker* InWorkerParent,
	const FString& InLogDB,
	const FString& InEpisodeId,
	const FString& InMongoIP,
	uint16 MongoPort)
{
	WorkerParent = InWorkerParent;
	ConnectToMongo(InLogDB, InEpisodeId, InMongoIP, MongoPort);
}

// Called to write the data
void FSLRawDataWriterMongo::WriteData()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);

	//bson_t bson;

	//bson_init(&bson);
	//BSON_APPEND_UTF8(&bson, "0", "foo");
	//BSON_APPEND_UTF8(&bson, "1", "bar");

	//str = bson_as_json(&bson, NULL);
	///* Prints
	//* { "0" : "foo", "1" : "bar" }
	//*/
	//printf("%s\n", str);
	//bson_free(str);

	//bson_destroy(&bson);
	

	//// bson root object
	//bson_t root = BSON_INITIALIZER;

	//// bson entities array
	//bson_t entities_arr = BSON_INITIALIZER;

	//BSON_APPEND_UTF8(&entities_arr, "id", "dummyid1");
	//BSON_APPEND_UTF8(&entities_arr, "id", "dummyid2");
	//BSON_APPEND_UTF8(&entities_arr, "id", "dummyid3");
	//


	//BSON_APPEND_DOUBLE(&root, "timestamp", WorkerParent->World->GetTimeSeconds());
	//BSON_APPEND_ARRAY(&root, "entities", &entities_arr);

	//char *str = bson_as_json(&root, NULL);

	//FString BsonStr = FString(TCHAR_TO_UTF8(str));

	//UE_LOG(LogTemp, Error, TEXT("[%s][%d] BsonStr=%s"), TEXT(__FUNCTION__), __LINE__, *BsonStr);
	


	//bson_writer_t *writer;
	//uint8_t *buf = NULL;
	//size_t buflen = 0;
	
	//writer = bson_writer_new(&buf, &buflen, 0, bson_realloc_ctx, NULL);

	//for (int i = 0; i < 1000; i++) {
	//	bson_writer_begin(writer, root);
	//	bson_writer_end(writer);
	//}
	//bson_writer_destroy(writer);
	//bson_free(buf);
	//bson_writer_end(writer);
}

// Set the file handle for the logger
bool FSLRawDataWriterMongo::ConnectToMongo(const FString& InLogDB,
	const FString& InEpisodeId,
	const FString& InMongoIP,
	uint16 MongoPort)
{
	return true;
}

// Add actors
void FSLRawDataWriterMongo::AddActors(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr)
{
	// Iterate actors
	for (auto RawDataActItr(WorkerParent->RawDataActors.CreateIterator()); RawDataActItr; ++RawDataActItr)
	{
		// Check if pointer is valid
		if (RawDataActItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataActItr->Entity->GetActorLocation();
			const FQuat CurrQuat = RawDataActItr->Entity->GetActorQuat();
			if (FVector::DistSquared(CurrLoc, RawDataActItr->PrevLoc) > WorkerParent->DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataActItr->PrevLoc = CurrLoc;

				// Get current entry as Bson object
				TSharedPtr<FJsonObject> BsonActorEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
					RawDataActItr->Id, RawDataActItr->Class, CurrLoc, CurrQuat);

				// If actor is skeletal, save bones data as well
				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(RawDataActItr->Entity))
				{
					// Bson array of bones
					TArray<TSharedPtr<FJsonValue>> BsonBonesArr;

					// Get skeletal mesh component
					USkeletalMeshComponent* SkelComp = SkelAct->GetSkeletalMeshComponent();

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						TSharedPtr<FJsonObject> BsonBoneEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Bson array
						BsonBonesArr.Add(MakeShareable(new FJsonValueObject(BsonBoneEntry)));
					}
					// Add bones to Bson actor
					BsonActorEntry->SetArrayField("bones", BsonBonesArr);
				}
				// Add entity to Bson array
				OutBsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(BsonActorEntry)));
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}
}

// Add components
void FSLRawDataWriterMongo::AddComponents(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr)
{
	// Iterate components
	for (auto RawDataCompItr(WorkerParent->RawDataComponents.CreateIterator()); RawDataCompItr; ++RawDataCompItr)
	{
		if (RawDataCompItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataCompItr->Entity->GetComponentLocation();
			const FQuat CurrQuat = RawDataCompItr->Entity->GetComponentQuat();
			if (FVector::DistSquared(CurrLoc, RawDataCompItr->PrevLoc) > WorkerParent->DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataCompItr->PrevLoc = CurrLoc;

				// Get current entry as Bson object
				TSharedPtr<FJsonObject> BsonCompEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
					RawDataCompItr->Id, RawDataCompItr->Class, CurrLoc, CurrQuat);

				// If comp is skeletal, save bones data as well
				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(RawDataCompItr->Entity))
				{
					// Bson array of bones
					TArray<TSharedPtr<FJsonValue>> BsonBonesArr;

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						TSharedPtr<FJsonObject> BsonBoneEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Bson array
						BsonBonesArr.Add(MakeShareable(new FJsonValueObject(BsonBoneEntry)));
					}
					// Add bones to Bson actor
					BsonCompEntry->SetArrayField("bones", BsonBonesArr);
				}
				// Add entity to Bson array
				OutBsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(BsonCompEntry)));
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
}

// Get entry as bson object
TSharedPtr<FJsonObject> FSLRawDataWriterMongo::GetAsBsonEntry(const FString& InId,
	const FString& InClass,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// New Bson entity object
	TSharedPtr<FJsonObject> BsonObj = MakeShareable(new FJsonObject);

	// Add "id" field if available (bones have no separate ids)
	if (!InId.IsEmpty())
	{
		BsonObj->SetStringField("id", InId);
	}

	// Add "class" field
	BsonObj->SetStringField("class", InClass);

	// Create and add "loc" field
	TSharedPtr<FJsonObject> LocObj = MakeShareable(new FJsonObject);
	LocObj->SetNumberField("x", ROSLoc.X);
	LocObj->SetNumberField("y", ROSLoc.Y);
	LocObj->SetNumberField("z", ROSLoc.Z);
	BsonObj->SetObjectField("loc", LocObj);

	// Create and add "rot" field
	TSharedPtr<FJsonObject> QuatObj = MakeShareable(new FJsonObject);
	QuatObj->SetNumberField("x", ROSQuat.X);
	QuatObj->SetNumberField("y", ROSQuat.Y);
	QuatObj->SetNumberField("z", ROSQuat.Z);
	QuatObj->SetNumberField("w", ROSQuat.W);
	BsonObj->SetObjectField("rot", QuatObj);

	return BsonObj;
}

// Write entry to db
void FSLRawDataWriterMongo::WriteToMongo(const TSharedPtr<FJsonObject>& InRootObj)
{
	// Transform to string
	FString BsonString;
	TSharedRef<TJsonWriter<>> BsonWriter = TJsonWriterFactory<>::Create(&BsonString);
	FJsonSerializer::Serialize(InRootObj.ToSharedRef(), BsonWriter);

	// Write to db
}