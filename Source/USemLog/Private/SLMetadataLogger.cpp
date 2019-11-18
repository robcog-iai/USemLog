// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMetadataLogger.h"
#include "SLEntitiesManager.h"
#include "Conversions.h"
#include "Tags.h"
#include "Components/SkeletalMeshComponent.h"

// Ctor
USLMetadataLogger::USLMetadataLogger()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLMetadataLogger::~USLMetadataLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}

	// Disconnect and clean db connection
	Disconnect();
}

// Init logger
void USLMetadataLogger::Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort, 
	bool bOverwrite, bool bScanItems, FSLItemScanParams ScanParams)
{
	if (!bIsInit)
	{
		if(!Connect(InTaskId, InServerIp, InServerPort, bOverwrite))
		{
			return;
		}

		if(bScanItems)
		{
			ItemsScanner = NewObject<USLItemScanner>(this);
			ItemsScanner->Init(InTaskId, InServerIp, InServerPort, ScanParams);
		}
		bIsInit = true;
	}
}

// Start logger
void USLMetadataLogger::Start(const FString& InTaskDescription)
{
	if (!bIsStarted && bIsInit)
	{
#if SL_WITH_LIBMONGO_C
		// Create the environment and task description document
		bson_oid_t oid;
		bson_t* doc = bson_new();

		bson_oid_init(&oid, NULL);
		BSON_APPEND_OID(doc, "_id", &oid);
		
		// Add data to the document
		AddTaskDescription(InTaskDescription, doc);
		AddEnvironmentData(doc);
		AddCameraViews(doc);

		// Write document to the collection
		bson_error_t error;
		if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
				*FString(__func__), __LINE__, *FString(error.message));
		}

		// Start the item scanner, for every finished scan it will trigger an update call on the logger
		if(ItemsScanner)
		{
			ItemsScanner->Start();
		}
		bson_destroy(doc);
		
		bIsStarted = true;
#endif //SL_WITH_LIBMONGO_C
	}
}

// Finish logger
void USLMetadataLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if(ItemsScanner)
		{
			ItemsScanner->Finish();
		}

		CreateIndexes();
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Connect to the database
bool USLMetadataLogger::Connect(const FString& DBName, const FString& ServerIp, uint16 ServerPort, bool bOverwrite)
{
	const FString MetaCollName = DBName + ".meta";
	
#if SL_WITH_LIBMONGO_C
	// Required to initialize libmongoc's internals	
	mongoc_init();

	// Stores any error that might appear during the connection
	bson_error_t error;

	// Safely create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);
	uri = mongoc_uri_new_with_error(TCHAR_TO_UTF8(*Uri), &error);
	if (!uri)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s; [Uri=%s]"),
			*FString(__func__), __LINE__, *FString(error.message), *Uri);
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create a mongo client.."), *FString(__func__), __LINE__);
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SL_" + MetaCollName)));
	
	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Give a warning if the collection already exists or not
	if (mongoc_database_has_collection(database, TCHAR_TO_UTF8(*MetaCollName), &error))
	{
		if(bOverwrite)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Meta collection %s already exists, will be removed and overwritten.."),
				*FString(__func__), __LINE__, *MetaCollName);
			if(!mongoc_collection_drop (mongoc_database_get_collection(database, TCHAR_TO_UTF8(*MetaCollName)), &error))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
					*FString(__func__), __LINE__, *FString(error.message));
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Meta collection %s already exists and should not be overwritten, skipping metadata logging.."),
				*FString(__func__), __LINE__, *MetaCollName);
			return false;
		}
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Meta collection %s does not exist, creating a new one.."),
			*FString(__func__), __LINE__, *MetaCollName);
	}

	collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*MetaCollName));


	// Create a gridfs handle prefixed by fs */
	gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*MetaCollName), &error);
	if (!gridfs)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *Uri, *FString(error.message));
		return false;
	}

	// Double check that the server is alive. Ping the "admin" database
	bson_t* server_ping_cmd;
	server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
	if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bson_destroy(server_ping_cmd);
		return false;
	}

	bson_destroy(server_ping_cmd);
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Disconnect from the database
void USLMetadataLogger::Disconnect()
{
#if SL_WITH_LIBMONGO_C
	// Release handles and clean up mongoc
	if(gridfs)
	{
		mongoc_gridfs_destroy(gridfs);
	}
	if(uri)
	{
		mongoc_uri_destroy(uri);
	}
	if(client)
	{
		mongoc_client_destroy(client);
	}
	if(database)
	{
		mongoc_database_destroy(database);
	}
	if(collection)
	{
		mongoc_collection_destroy(collection);
	}
	//if(scan_entry_doc)
	//{
	//	bson_destroy(scan_entry_doc);
	//}
	//if(scan_pose_arr)
	//{
	//	bson_destroy(scan_entry_doc);
	//}
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}

void USLMetadataLogger::CreateIndexes()
{
	if(!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Try again when initialized.."), *FString(__func__), __LINE__);
		return;
	}
	
#if SL_WITH_LIBMONGO_C
	bson_t idx_task;
	bson_init(&idx_task);
	BSON_APPEND_INT32(&idx_task, "task_description", 1);
	char* idx_task_str = mongoc_collection_keys_to_index_string(&idx_task);

	bson_t idx_cls;
	bson_init(&idx_cls);
	BSON_APPEND_INT32(&idx_cls, "class", 1);
	char* idx_cls_str = mongoc_collection_keys_to_index_string(&idx_cls);

	bson_t* index_command;
	bson_error_t error;

	index_command = BCON_NEW("createIndexes",
		BCON_UTF8(mongoc_collection_get_name(collection)),
		"indexes",
		"[",
			"{",
				"key",
				BCON_DOCUMENT(&idx_task),
				"name",
				BCON_UTF8(idx_task_str),
				//"unique",
				//BCON_BOOL(false),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_cls),
				"name",
				BCON_UTF8(idx_cls_str),
				//"unique",
				//BCON_BOOL(false),
			"}",
		"]");

	if (!mongoc_collection_write_command_with_opts(collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(idx_task_str);
	bson_free(idx_cls_str);
#endif //SL_WITH_LIBMONGO_C
}

// Create the scan entry bson document
void USLMetadataLogger::StartScanEntry(const FString& Class, int32 ResX, int32 ResY)
{
#if SL_WITH_LIBMONGO_C
	if(scan_entry_doc || scan_pose_arr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Scan entry should have been un-initialized.. aborting.."), *FString(__func__), __LINE__);
		return;
	}
	scan_entry_doc = bson_new();
	scan_pose_arr = bson_new();
	scan_pose_arr_idx = 0;

	BSON_APPEND_UTF8(scan_entry_doc, "class", TCHAR_TO_UTF8(*Class));

	bson_t res_obj;
	BSON_APPEND_DOCUMENT_BEGIN(scan_entry_doc, "res", &res_obj);
		BSON_APPEND_INT32(&res_obj, "x", ResX);
		BSON_APPEND_INT32(&res_obj, "y", ResY);
	bson_append_document_end(scan_entry_doc, &res_obj);

/*
 * <-- BEGIN "scans" array -->
 */
	BSON_APPEND_ARRAY_BEGIN(scan_entry_doc, "scans", scan_pose_arr);
#endif //SL_WITH_LIBMONGO_C
}

// Add pose scan data
void USLMetadataLogger::AddScanPoseEntry(const FSLScanPoseData& ScanPoseData)
{
#if SL_WITH_LIBMONGO_C
	if(!scan_entry_doc)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Scan entry is not initialized.. aborting.."), *FString(__func__), __LINE__);
		return;
	}

	if(ScanPoseData.NumPixels == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Scan entry has no item pixels, skipping entry.."), *FString(__func__), __LINE__);
		return;
	}

	bson_t scan_pose_doc;
	bson_t scan_img_arr;
	bson_t scan_img_arr_obj;
	bson_oid_t file_oid;

	char pose_key_str[16];
	const char *pose_key;
	
	uint32_t img_arr_idx = 0;
	char img_key_str[16];
	const char *img_key;

	bson_uint32_to_string(scan_pose_arr_idx, &pose_key, pose_key_str, sizeof pose_key_str);
	BSON_APPEND_DOCUMENT_BEGIN(scan_pose_arr, pose_key, &scan_pose_doc);

		BSON_APPEND_INT32(&scan_pose_doc, "num_pixels", ScanPoseData.NumPixels);
		AddPoseChild(ScanPoseData.Pose.GetLocation(), ScanPoseData.Pose.GetRotation(), &scan_pose_doc);
		AddImgBBChild(ScanPoseData.BBMin, ScanPoseData.BBMax, &scan_pose_doc);

		// "images" array containing the pointer to the gridfs file and the rendering type
		BSON_APPEND_ARRAY_BEGIN(&scan_pose_doc, "images", &scan_img_arr);
		for(const auto& Pair : ScanPoseData.Images)
		{
			AddToGridFs(Pair.Value, &file_oid);
			bson_uint32_to_string(img_arr_idx, &img_key, img_key_str, sizeof img_key_str);
			BSON_APPEND_DOCUMENT_BEGIN(&scan_img_arr, img_key, &scan_img_arr_obj);
				BSON_APPEND_UTF8(&scan_img_arr_obj, "type", TCHAR_TO_UTF8(*Pair.Key));
				BSON_APPEND_OID(&scan_img_arr_obj, "file_id", (const bson_oid_t*)&file_oid);
			bson_append_document_end(&scan_img_arr, &scan_img_arr_obj);
			img_arr_idx++;
		}
		bson_append_array_end(&scan_pose_doc, &scan_img_arr);
	
	bson_append_document_end(scan_pose_arr,&scan_pose_doc);
	scan_pose_arr_idx++;
	
#endif //SL_WITH_LIBMONGO_C
}

// Write and clear the scan entry to the database
void USLMetadataLogger::FinishScanEntry()
{
#if SL_WITH_LIBMONGO_C
	if(scan_entry_doc && scan_pose_arr)
	{
		// End the scan camera poses array
		bson_append_array_end(scan_entry_doc, scan_pose_arr);
/*
 * <-- END "scans" array -->
 */
		
		// Write entry to the collection
		bson_error_t error;
		if (!mongoc_collection_insert_one(collection, scan_entry_doc, NULL, NULL, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
				*FString(__func__), __LINE__, *FString(error.message));
		}
		bson_clear(&scan_pose_arr);
		bson_clear(&scan_entry_doc);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Scan entry NOT initialized.. aborting.."), *FString(__func__), __LINE__);
	}
#endif //SL_WITH_LIBMONGO_C
}

#if SL_WITH_LIBMONGO_C
// Add image to gridfs, output the oid
void USLMetadataLogger::AddToGridFs(const TArray<uint8>& CompressedBitmap, bson_oid_t* out_oid)
{
	mongoc_gridfs_file_t *file;
	mongoc_gridfs_file_opt_t file_opt = { 0 };
	const bson_value_t* file_id_val;
	mongoc_iovec_t iov;
	bson_error_t error;

	//bson_t* metadata_doc;
	//metadata_doc = BCON_NEW(
	//	"type", BCON_UTF8(TCHAR_TO_UTF8(*ImgData.RenderType))
	//);
	//file_opt.filename = "no_name";
	//file_opt.metadata = metadata_doc;

	// Create new file
	file = mongoc_gridfs_create_file(gridfs, &file_opt);

	// Set data binary and length
	iov.iov_base = (char*)(CompressedBitmap.GetData());
	iov.iov_len = CompressedBitmap.Num();

	// Write data to gridfs
	if (iov.iov_len != mongoc_gridfs_file_writev(file, &iov, 1, 0))
	{
		if (mongoc_gridfs_file_error(file, &error))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
				*FString(__func__), __LINE__, *FString(error.message));
		}
		mongoc_gridfs_file_destroy(file);
		return;
	}

	// Saves modifications to file to the MongoDB server
	if (!mongoc_gridfs_file_save(file))
	{
		mongoc_gridfs_file_error(file, &error);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		mongoc_gridfs_file_destroy(file);
		return;
	}

	// Set the out oid
	file_id_val = mongoc_gridfs_file_get_id(file);
	bson_oid_copy(&file_id_val->value.v_oid, out_oid);

	// Clean up
	mongoc_gridfs_file_destroy(file);
}

// Write the task description to the document
void USLMetadataLogger::AddTaskDescription(const FString& InTaskDescription, bson_t* doc)
{
	BSON_APPEND_UTF8(doc, "task_description", TCHAR_TO_UTF8(*InTaskDescription));
}

// Write the environment data
void USLMetadataLogger::AddEnvironmentData(bson_t* doc)
{
	bson_t arr;
	bson_t sk_arr;
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;
	uint32_t idx = 0;

	// Add entities to array
	BSON_APPEND_ARRAY_BEGIN(doc, "entities", &arr);
	// Iterate non skeletal semantic entities
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
	{
		const FSLEntity SemEntity = Pair.Value;

		// Ignore skeletal entities
		if (Cast<ASkeletalMeshActor>(SemEntity.Obj) || Cast<USkeletalMeshComponent>(SemEntity.Obj))
		{
			continue;
		}

		// Start array doc
		bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&arr, idx_key, &arr_obj);

		BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*Pair.Value.Id));
		BSON_APPEND_UTF8(&arr_obj, "class", TCHAR_TO_UTF8(*Pair.Value.Class));

		FString ColorHex = FTags::GetValue(Pair.Value.Obj, "SemLog", "VisMask");
		if (!ColorHex.IsEmpty())
		{
			BSON_APPEND_UTF8(&arr_obj, "mask_hex", TCHAR_TO_UTF8(*ColorHex));
		}

		// Check if location data is available
		if (AActor* ObjAsAct = Cast<AActor>(SemEntity.Obj))
		{
			const FVector Loc = ObjAsAct->GetActorLocation();
			const FQuat Quat = ObjAsAct->GetActorQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}
		else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(SemEntity.Obj))
		{
			const FVector Loc = ObjAsSceneComp->GetComponentLocation();
			const FQuat Quat = ObjAsSceneComp->GetComponentQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}

		// Finish array doc
		bson_append_document_end(&arr, &arr_obj);
		idx++;
	}
	bson_append_array_end(doc, &arr);

	// Add skel entities to array
	BSON_APPEND_ARRAY_BEGIN(doc, "skel_entities", &sk_arr);
	// Reset array index
	idx=0;
	// Iterate skeletal semantic entities
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSkeletalSemanticData())
	{
		USLSkeletalDataComponent* SkelDataComp = Pair.Value;
		USkeletalMeshComponent* SkMComp = SkelDataComp->SkeletalMeshParent;
		const FSLEntity OwnerSemData = SkelDataComp->OwnerSemanticData;
		UObject* SemOwner = SkelDataComp->SemanticOwner;

		bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&sk_arr, idx_key, &arr_obj);

		BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*OwnerSemData.Id));
		BSON_APPEND_UTF8(&arr_obj, "class", TCHAR_TO_UTF8(*OwnerSemData.Class));

		// Add semantic owner (component or actor) location
		if (AActor* ObjAsAct = Cast<AActor>(SemOwner))
		{
			const FVector Loc = ObjAsAct->GetActorLocation();
			const FQuat Quat = ObjAsAct->GetActorQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}
		else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(SemOwner))
		{
			const FVector Loc = ObjAsSceneComp->GetComponentLocation();
			const FQuat Quat = ObjAsSceneComp->GetComponentQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}

		// Check if the skeletal mesh is valid
		if (SkMComp)
		{
			bson_t bones_arr;
			bson_t bones_arr_obj;
			char jdx_str[16];
			const char *jdx_key;
			uint32_t jdx = 0;

			BSON_APPEND_ARRAY_BEGIN(&arr_obj, "bones", &bones_arr);
			// Create array of all the bones (with and without(empty strings) semantic data)
			for (const auto& BoneNameToDataPair : SkelDataComp->AllBonesData)
			{
				const FName BoneName = BoneNameToDataPair.Key;
				const FSLBoneData BoneData = BoneNameToDataPair.Value;
				const FVector BoneLoc = SkMComp->GetBoneLocation(BoneName);
				const FQuat BoneQuat = SkMComp->GetBoneQuaternion(BoneName);

				bson_uint32_to_string(jdx, &jdx_key, jdx_str, sizeof jdx_str);
				BSON_APPEND_DOCUMENT_BEGIN(&bones_arr, jdx_key, &bones_arr_obj);

				BSON_APPEND_UTF8(&bones_arr_obj, "name", TCHAR_TO_UTF8(*BoneName.ToString()));

				if (!BoneData.Class.IsEmpty())
				{
					BSON_APPEND_UTF8(&bones_arr_obj, "class", TCHAR_TO_UTF8(*BoneData.Class));

					if (!BoneData.VisualMask.IsEmpty())
					{
						BSON_APPEND_UTF8(&bones_arr_obj, "mask_hex", TCHAR_TO_UTF8(*BoneData.VisualMask));
					}
				}

				AddPoseChild(BoneLoc, BoneQuat, &bones_arr_obj);

				bson_append_document_end(&bones_arr, &bones_arr_obj);
				jdx++;
			}
			// Add the created array to the semantic item
			bson_append_array_end(&arr_obj, &bones_arr);
		}

		// Add the semantic item to the array
		bson_append_document_end(&sk_arr, &arr_obj);
		idx++;
	}
	bson_append_array_end(doc, &sk_arr);
}

// Add camera views
void USLMetadataLogger::AddCameraViews(bson_t* doc)
{
	bson_t arr;
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;
	uint32_t idx = 0;

	// Add entities to array
	BSON_APPEND_ARRAY_BEGIN(doc, "camera_views", &arr);

	// Iterate non skeletal semantic entities
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetCameraViewsSemanticData())
	{
		const FSLEntity SemEntity = Pair.Value;

		if (Cast<ASkeletalMeshActor>(SemEntity.Obj) || Cast<USkeletalMeshComponent>(SemEntity.Obj))
		{
			continue;
		}

		// Start array doc
		bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&arr, idx_key, &arr_obj);

		BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*Pair.Value.Id));
		BSON_APPEND_UTF8(&arr_obj, "class", TCHAR_TO_UTF8(*Pair.Value.Class));
		
		// Check if location data is available
		if (AActor* ObjAsAct = Cast<AActor>(SemEntity.Obj))
		{
			const FVector Loc = ObjAsAct->GetActorLocation();
			const FQuat Quat = ObjAsAct->GetActorQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}
		else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(SemEntity.Obj))
		{
			const FVector Loc = ObjAsSceneComp->GetComponentLocation();
			const FQuat Quat = ObjAsSceneComp->GetComponentQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}

		// Finish array doc
		bson_append_document_end(&arr, &arr_obj);
		idx++;
	}

	bson_append_array_end(doc, &arr);
}

// Add pose to document
void USLMetadataLogger::AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* doc)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	bson_t child_obj_loc;
	bson_t child_obj_rot;

	BSON_APPEND_DOCUMENT_BEGIN(doc, "loc", &child_obj_loc);
	BSON_APPEND_DOUBLE(&child_obj_loc, "x", ROSLoc.X);
	BSON_APPEND_DOUBLE(&child_obj_loc, "y", ROSLoc.Y);
	BSON_APPEND_DOUBLE(&child_obj_loc, "z", ROSLoc.Z);
	bson_append_document_end(doc, &child_obj_loc);

	BSON_APPEND_DOCUMENT_BEGIN(doc, "rot", &child_obj_rot);
	BSON_APPEND_DOUBLE(&child_obj_rot, "x", ROSQuat.X);
	BSON_APPEND_DOUBLE(&child_obj_rot, "y", ROSQuat.Y);
	BSON_APPEND_DOUBLE(&child_obj_rot, "z", ROSQuat.Z);
	BSON_APPEND_DOUBLE(&child_obj_rot, "w", ROSQuat.W);
	bson_append_document_end(doc, &child_obj_rot);
}

// Add image bounding box to document
void USLMetadataLogger::AddImgBBChild(const FIntPoint& Min, const FIntPoint& Max, bson_t* doc)
{
	bson_t bb;
	bson_t child_min_bb;
	bson_t child_max_bb;

	BSON_APPEND_DOCUMENT_BEGIN(doc, "img_bb", &bb);
		BSON_APPEND_DOCUMENT_BEGIN(&bb, "min", &child_min_bb);
			BSON_APPEND_DOUBLE(&child_min_bb, "x", Min.X);
			BSON_APPEND_DOUBLE(&child_min_bb, "y", Min.Y);
		bson_append_document_end(&bb, &child_min_bb);
		BSON_APPEND_DOCUMENT_BEGIN(&bb, "max", &child_max_bb);
			BSON_APPEND_DOUBLE(&child_max_bb, "x", Max.X);
			BSON_APPEND_DOUBLE(&child_max_bb, "y", Max.Y);
		bson_append_document_end(&bb, &child_max_bb);
	bson_append_document_end(doc, &bb);
}
#endif //SL_WITH_LIBMONGO_C
