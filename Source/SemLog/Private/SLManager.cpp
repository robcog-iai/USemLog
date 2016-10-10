// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"
#include "SLUtils.h"
#include "SLManager.h"

// Sets default values
ASLManager::ASLManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Actor hidden in game
	SetActorHiddenInGame(true);

	// Log directory name
	LogRootDirectoryName = "SemLogs";

	// Episode unique tag
	EpisodeUniqueTag = FSLUtils::GenerateRandomFString(4);

	// Default flag values
	bLogRawData = true;
	bLogSemanticMap = true;
	bLogSemanticEvents = true;

	// Default distance threshold for logging raw data
	DistanceThreshold = 0.1;
}

// Actor initialization, log items init
void ASLManager::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Level directory path
	LevelPath = LogRootDirectoryName + "/" + GetWorld()->GetName();
	// Episode directory path
	EpisodePath = LevelPath + "/Episodes/" + "rcg_" + FDateTime::Now().ToString();
	// Raw data directory path
	RawDataPath = LevelPath + "/RawData/";
	// Create the directory paths
	ASLManager::CreateDirectoryPath(EpisodePath);
	ASLManager::CreateDirectoryPath(RawDataPath);

	// Init items that should be logged
	ASLManager::InitLogItems();
	// Check if unique names already generated (past episodes)
	if (!ASLManager::ReadPrevUniqueNames(LevelPath + "/MetaData.json"))
	{
		// Generate new unique names if not generated or out of sync
		ASLManager::GenerateNewUniqueNames();
		// Save unique names to file (for future use)
		ASLManager::StoreNewUniqueNames(LevelPath + "/MetaData.json");
	}

	// Log Semantic map
	if (bLogSemanticMap)
	{
		// Semantic map path
		const FString SemMapPath = LevelPath + "/SemanticMap.owl";
		// Chek if semantic map is not already created
		if (!IFileManager::Get().FileExists(*SemMapPath))
		{
			// Create sem map exporter
			SemMapExporter = new FSLMapExporter();
		}
	}

	// Init raw data logger
	if (bLogRawData)
	{
		// Path to the json file
		const FString RawFilePath = RawDataPath + "/RawData_" + EpisodeUniqueTag + ".json";
		// Init raw data exporter
		RawDataExporter = new FSLRawDataExporter(DistanceThreshold, RawFilePath);
	}

	// Init semantic events logger
	if (bLogSemanticEvents)
	{
		SemEventsExporter = new FSLEventsExporter(
			EpisodeUniqueTag,
			ActorToUniqueName,
			ActorToSemLogInfo,
			GetWorld()->GetTimeSeconds());
	}
}

// Called when the game starts or when spawned
void ASLManager::BeginPlay()
{
	Super::BeginPlay();

	// Disable tick for now
	SetActorTickEnabled(false);
}

// Called when the game is terminated
void ASLManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Write events
	if (SemEventsExporter)
	{
		SemEventsExporter->WriteEvents(EpisodePath,	GetWorld()->GetTimeSeconds());
	}
}

// Called every frame
void ASLManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
		
	// Log raw data
	if (RawDataExporter)
	{
		RawDataExporter->Update(GetWorld()->GetTimeSeconds());
	}
}

// Init exporters, write initial states
void ASLManager::Init()
{
	// Generate and write level semantic map
	if (SemMapExporter)
	{
		// Semantic map file path
		const FString SemMapPath = LevelPath + "/SemanticMap.owl";
		// Write map to path
		SemMapExporter->WriteSemanticMap(ActorToUniqueName, ActorToSemLogInfo, SemMapPath);
	}

	// Initial raw data log (static objects are stored once)
	if (RawDataExporter)
	{
		RawDataExporter->WriteInit(
			ActorToUniqueName,
			ActorToSemLogInfo,
			GetWorld()->GetTimeSeconds());
	}

	// Enable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(true);
	}
}

// Start logging by enabling tick
void ASLManager::Start()
{
	// Enable tick
	SetActorTickEnabled(true);

	// Enable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(true);
	}

}

// Pause logging by disabling tick
void ASLManager::Pause()
{
	// Disable tick
	SetActorTickEnabled(false);

	// Disable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(false);
	}
}

// Create directory path for logging
void ASLManager::CreateDirectoryPath(FString Path)
{
	// Create array of the directory names
	TArray<FString> DirNames;
	Path.ParseIntoArray(DirNames, TEXT("/"), true);

	// Get platform file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Current path
	FString CurrPath;

	// Create directory path
	for (const auto DirName : DirNames)
	{
		// Append current directory name to the path
		CurrPath.Append(DirName + "/");
		// Create directory if not existent
		if (!PlatformFile.DirectoryExists(*CurrPath))
		{
			PlatformFile.CreateDirectory(*CurrPath);
		}
	}
}

// Set items to be logged (from tags)
void ASLManager::InitLogItems()
{
	UE_LOG(SemLog, Log, TEXT(" ** Init items to log: "));
	// Iterate through the static mesh actors and check tags to see which objects should be logged
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Iterate throught the tags
		for (const auto TagItr : ActItr->Tags)
		{
			// Copy of the current tag
			FString CurrTag = TagItr.ToString();

			// Check if the tag describes the semantic logging description
			if (CurrTag.RemoveFromStart("SemLog:"))
			{
				UE_LOG(SemLog, Log, TEXT(" \t %s: "), *ActItr->GetName());
				// Array of key value pairs representing the semantic log info
				TArray<TPair<FString, FString>> SemLogInfoArr;

				// parse tag string into array of strings reprsenting comma separated key-value pairs
				TArray<FString> TagKeyValueArr;
				CurrTag.ParseIntoArray(TagKeyValueArr, TEXT(";"));

				// Iterate the array of key-value strings and add them to the map
				for (const auto TagKeyValItr : TagKeyValueArr)
				{
					// Split string and add the key-value to the string pair
					TPair<FString, FString> KeyValPair;
					TagKeyValItr.Split(TEXT(","), &KeyValPair.Key, &KeyValPair.Value);

					// Add key-val info to the array
					SemLogInfoArr.Add(KeyValPair);
					UE_LOG(SemLog, Log, TEXT(" \t\t %s : %s"), *KeyValPair.Key, *KeyValPair.Value);
				}

				// Add actor and the semantic log info to the map
				ActorToSemLogInfo.Add(*ActItr, SemLogInfoArr);

				// Semlog info found, stop searching in other tags.
				break;
			}
		}
	}
}

// Read previously stored unique names from file
bool ASLManager::ReadPrevUniqueNames(const FString Path)
{
	UE_LOG(SemLog, Log, TEXT(" ** Reading unique names:"));
	// Check if file exists, and see if it is in sync with the level
	if (IFileManager::Get().FileExists(*Path))
	{
		// Read string from file		
		FString JsonString;
		FFileHelper::LoadFileToString(JsonString, *Path);

		// Create a json object from the read string
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(JsonString);
		TSharedPtr<FJsonObject> JsonObject;
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			// Get the actor to unique name array
			TArray< TSharedPtr<FJsonValue> > JsonArr = JsonObject->GetArrayField("level_unique_names");

			// Map that will store all the names and unique names from the json file
			TMap<FString, FString> NameToUniqueNameMap;
			// Iterate the json array to read the names and the unique names
			for (const auto JsonItr : JsonArr)
			{
				NameToUniqueNameMap.Add(*JsonItr->AsObject()->GetStringField("name"),
					*JsonItr->AsObject()->GetStringField("unique_name"));
			}

			// Check if the curretn items to be logged are stored in the json array
			for (const auto ActToInfoItr : ActorToSemLogInfo)
			{
				// Local copy of actor name
				const FString ActName = ActToInfoItr.Key->GetName();
				if (NameToUniqueNameMap.Contains(ActName))
				{
					ActorToUniqueName.Add(ActToInfoItr.Key, NameToUniqueNameMap[ActName]);
					UE_LOG(SemLog, Log, TEXT(" \t %s --> %s"), *ActName, *NameToUniqueNameMap[ActName]);
				}
				else
				{
					UE_LOG(SemLog, Error, TEXT("Previous unique names not in sync! %s not found!"), *ActName);
				}

			}
		}
		else
		{
			UE_LOG(SemLog, Error, TEXT("Unique names cannot be read! Json string: %s"), *JsonString);
			return false;
		}

		// Succesfully read all the unique values
		return true;
	}
	else
	{
		UE_LOG(SemLog, Warning,
			TEXT("No previous level unique names found at: %s, generating new ones!"), *Path);
		return false;
	}
}

// Generate items unique names
void ASLManager::GenerateNewUniqueNames()
{
	UE_LOG(SemLog, Log, TEXT(" ** Generating new unique names:"));
	
	// Generate unqiue names for the actors
	for (const auto ActToInfoItr : ActorToSemLogInfo)
	{
		FString UName = ActToInfoItr.Key->GetName();
		UName += (UName.Contains("_")) ? FSLUtils::GenerateRandomFString(4)
			: "_" + FSLUtils::GenerateRandomFString(4);
		ActorToUniqueName.Add(ActToInfoItr.Key, UName);
		UE_LOG(SemLog, Log, TEXT(" \t %s --> %s"), *ActToInfoItr.Key->GetName(), *UName);
	}
}

// Write generated unique names to file
void ASLManager::StoreNewUniqueNames(const FString Path)
{
	UE_LOG(SemLog, Log, TEXT(" ** Writing unique names to [%s]:"), *Path);
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Json array of actors
	TArray< TSharedPtr<FJsonValue> > JsonUniqueNamesArr;

	// Add actors to be logged to the Json array
	for (const auto ActToInfoItr : ActorToSemLogInfo)
	{
		// Local copy of the actor name
		FString ActName = ActToInfoItr.Key->GetName();
		// Json location object
		TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
		// Add fields
		JsonObj->SetStringField("name", ActName);
		JsonObj->SetStringField("unique_name", ActorToUniqueName[ActToInfoItr.Key]);
		// Add actor to Json array
		JsonUniqueNamesArr.Add(MakeShareable(new FJsonValueObject(JsonObj)));
		UE_LOG(SemLog, Log, TEXT(" \t %s --> %s"), *ActName, *ActorToUniqueName[ActToInfoItr.Key]);
	}
	
	// Add actors to Json root
	JsonRootObj->SetArrayField("level_unique_names", JsonUniqueNamesArr);

	// Transform to string
	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

	// Write string to file
	FFileHelper::SaveStringToFile(JsonOutputString, *Path);
}


