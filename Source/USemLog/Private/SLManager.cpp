// Fill out your copyright notice in the Description page of Project Settings.

#include "USemLogPrivatePCH.h"
#include "InstancedFoliageActor.h"
#include "Landscape.h"
#include "Components/SplineMeshComponent.h"
#include "SLUtils.h"
//#include "SLRawDataExporter.h"
//#include "SLMapExporter.h"
//#include "SLEventsExporter.h"
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
	bStartLoggingAtLoadTime = false;
	bLogRawData = true;
	bLogSemanticMap = true;
	bLogSemanticEvents = true;
	bLogLandscapeComponents = true;
	bFirstEpisode = false;

	// Default distance threshold for logging raw data
	DistanceThreshold = 0.1;

	// Delay for starting logging at load time
	LoadTimeLoggingDelay = 0.f;

	// External time
	ExternalInitTime = 0.f;

	// Set the manager state to un initialized
	ManagerState = ESLManagerState::UnInit;
}

// Actor initialization, log items init, called before BeginPlay
void ASLManager::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Run pre init only if loggins should happen at load time without a delay
	if (bStartLoggingAtLoadTime && !(LoadTimeLoggingDelay > 0.f))
	{
		ASLManager::PreInit();
	}
}

// Called when the game starts or when spawned
void ASLManager::BeginPlay()
{
	Super::BeginPlay();

	// Disable tick for now
	SetActorTickEnabled(false);

	// Check if logging should start at load time, and if with a delay
	if (bStartLoggingAtLoadTime)
	{
		if (!(LoadTimeLoggingDelay > 0.f))
		{
			ASLManager::Init();
			ASLManager::Activate();
		}
		else
		{
			UE_LOG(SemLog, Log, TEXT(" ** Semantic logging will start after %f seconds."), LoadTimeLoggingDelay);
			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(
				UnusedHandle, this, &ASLManager::StartLoggingWithDelay, LoadTimeLoggingDelay, false);			
		}
	}
}

// Called when the game is terminated
void ASLManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!ASLManager::IsUnInit() && !ASLManager::IsFinished())
	{
		ASLManager::Finish();
	}
}

// Called every frame
void ASLManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Log raw data
	if (RawDataExporter)
	{
		RawDataExporter->Update(GetWorld()->GetTimeSeconds() + ExternalInitTime);
	}
}

// Before init exporters, setup folders, read previous values
bool ASLManager::PreInit(const float ExternalTime)
{
	if (!ASLManager::IsUnInit())
	{
		UE_LOG(SemLog, Error, TEXT(" !! Cannot PreInit SLManager, current state is not UnInit."));
		return false;
	}
	UE_LOG(SemLog, Log, TEXT(" ** PreInit **"));

	// Set the external init time
	ExternalInitTime = ExternalTime;

	// Level directory path
	LevelPath = LogRootDirectoryName + "/" + GetWorld()->GetName();
	// Init items that should be logged
	ASLManager::InitLogItems();

	// Check if unique names are already generated AND in sync (past episodes)
	if (ASLManager::ShouldGenerateNewUniqueNames(LevelPath + "/MetaData.json"))
	{
		// Mark as first episode
		bFirstEpisode = true;
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
		// Raw data directory path
		RawDataPath = LevelPath + "/RawData/";
		// Create the raw data directory path
		ASLManager::CreateDirectoryPath(RawDataPath);
		// Path to the json file
		const FString RawFilePath = RawDataPath + "/RawData_" + EpisodeUniqueTag + ".json";
		// Init raw data exporter
		RawDataExporter = new FSLRawDataExporter(DistanceThreshold, RawFilePath);
	}

	// Init semantic events logger
	if (bLogSemanticEvents)
	{
		// Init semantic events exporter
		SemEventsExporter = new FSLEventsExporter(
			EpisodeUniqueTag,
			ActorToUniqueName,
			ActorToSemLogInfo,
			GetWorld()->GetTimeSeconds() + ExternalInitTime);
	}

	// Set the manager state to pre initialized
	ManagerState = ESLManagerState::PreInit;
	
	return true;
}

// Init exporters, write initial states
bool ASLManager::Init()
{
	if (!ASLManager::IsPreInit())
	{
		UE_LOG(SemLog, Error, TEXT(" !! Cannot Init SLManager, current state is not PreInit."));
		return false;
	}
	UE_LOG(SemLog, Log, TEXT(" ** Init **"));

	// Generate and write level semantic map
	if (SemMapExporter)
	{
		// Semantic map file path
		const FString SemMapPath = LevelPath + "/SemanticMap.owl";
		// Write map to path
		SemMapExporter->WriteSemanticMap(
			ActorToUniqueName,
			ActorToSemLogInfo,
			FoliageClassNameToComponent,
			FoliageComponentToUniqueNameArray,
			RoadCompNameToComponent,
			RoadComponentNameToUniqueName,
			RoadUniqueName,
			SemMapPath);
	}

	// Initial raw data log (static objects are stored once)
	if (RawDataExporter)
	{		
		RawDataExporter->WriteInit(
			ActorToUniqueName,
			ActorToSemLogInfo,
			GetWorld()->GetTimeSeconds() + ExternalInitTime);
	}

	// Enable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(true);
	}

	// Set the manager state to initialized
	ManagerState = ESLManagerState::Init;

	return true;
}

// Activate logging
bool ASLManager::Activate()
{
	if (!ASLManager::IsInit() && !ASLManager::IsPaused())
	{
		UE_LOG(SemLog, Error, TEXT(" !! Cannot Start SLManager, current state is not Init or Paused."));
		return false;
	}
	UE_LOG(SemLog, Log, TEXT(" ** Active **"));

	// Enable tick
	SetActorTickEnabled(true);

	// Enable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(true);
	}

	// Set the manager state to active
	ManagerState = ESLManagerState::Active;

	return true;
}

// Pause logging by disabling tick
bool ASLManager::Pause()
{
	if (!ASLManager::IsActive())
	{
		UE_LOG(SemLog, Error, TEXT(" !! Cannot Pause SLManager, current state is not Active."));
		return false;
	}
	UE_LOG(SemLog, Log, TEXT(" ** Paused **"));

	// Disable tick
	SetActorTickEnabled(false);

	// Disable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(false);
	}

	// Set the manager state to paused
	ManagerState = ESLManagerState::Paused;

	return true;
}

// Stop the loggers, save states to file
bool ASLManager::Finish()
{
	if (ASLManager::IsUnInit() || ASLManager::IsFinished())
	{
		UE_LOG(SemLog, Error, TEXT(" !! Cannot Finish SLManager, current state is UnInit or already Finished."));
		return false;
	}
	UE_LOG(SemLog, Log, TEXT(" ** Finished **"));

	// Write events
	if (SemEventsExporter)
	{
		// Episode directory path
		EpisodePath = LevelPath + "/Episodes/" + "rcg_" + FDateTime::Now().ToString();
		// Create the directory paths
		ASLManager::CreateDirectoryPath(EpisodePath);
		// Write semantic events
		SemEventsExporter->WriteEvents(EpisodePath,
			GetWorld()->GetTimeSeconds() + ExternalInitTime);
	}

	// Set the manager state to paused
	ManagerState = ESLManagerState::Finished;

	return true;
}

// Cancel logging, remove generated files
bool ASLManager::Cancel()
{
	if (ASLManager::IsUnInit())
	{
		UE_LOG(SemLog, Error, TEXT(" !! Cannot Cancel SLManager, current state is UnInit."));
		return false;
	}
	UE_LOG(SemLog, Log, TEXT(" ** Cancelled ** "));
	
	if (bFirstEpisode)
	{
		// TODO remove everything
		//LevelPath
	}
	else
	{
		// TODO remove raw and events
		if (bLogRawData)
		{
			const FString RawFilePath = RawDataPath + "/RawData_" + EpisodeUniqueTag + ".json";
			if (!IFileManager::Get().FileExists(*RawFilePath))
			{
				// RM
			}
		}
	
		if (bLogSemanticEvents)
		{
			if (!IFileManager::Get().FileExists(*EpisodePath))
			{
				// RM
			}
		}
	}

	// Set the manager state to paused
	ManagerState = ESLManagerState::Cancelled;

	return true;
}

// Add finished semantic event
bool ASLManager::AddFinishedSemanticEvent(
	const FString EventNamespace,
	const FString EventName,
	const float StartTime,
	const float EndTime,
	const TArray<FSLOwlTriple>& Properties)
{
	if (SemEventsExporter && ASLManager::IsActive())
	{
		SemEventsExporter->AddFinishedEventIndividual(
			EventNamespace,	EventName, StartTime, EndTime, Properties);
		return true;
	}
	return false;
}

// Start logging with delay
void ASLManager::StartLoggingWithDelay()
{
	ASLManager::PreInit();
	ASLManager::Init();
	ASLManager::Activate();
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
	for (const auto& DirName : DirNames)
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
	UE_LOG(SemLog, Log, TEXT(" \t Init actors: "));
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Iterate throught the tags
		for (const auto& TagItr : ActItr->Tags)
		{
			// Copy of the current tag
			FString CurrTag = TagItr.ToString();

			// Check if the tag describes the semantic logging description
			if (CurrTag.RemoveFromStart("SemLog:"))
			{
				UE_LOG(SemLog, Log, TEXT(" \t\t %s: "), *ActItr->GetName());
				// Array of key value pairs representing the semantic log info
				TArray<TPair<FString, FString>> SemLogInfoArr;

				// parse tag string into array of strings reprsenting comma separated key-value pairs
				TArray<FString> TagKeyValueArr;
				CurrTag.ParseIntoArray(TagKeyValueArr, TEXT(";"));

				// Iterate the array of key-value strings and add them to the map
				for (const auto& TagKeyValItr : TagKeyValueArr)
				{
					// Split string and add the key-value to the string pair
					TPair<FString, FString> KeyValPair;
					TagKeyValItr.Split(TEXT(","), &KeyValPair.Key, &KeyValPair.Value);

					// Add key-val info to the array
					SemLogInfoArr.Add(KeyValPair);
					UE_LOG(SemLog, Log, TEXT(" \t\t\t %s : %s"), *KeyValPair.Key, *KeyValPair.Value);
				}

				// Add actor and the semantic log info to the map
				ActorToSemLogInfo.Add(*ActItr, SemLogInfoArr);

				// Semlog info found, stop searching in other tags.
				break;
			}
		}
	}

	// Check if lanscape components should be logged
	if (bLogLandscapeComponents)
	{
		// Check if foliage should be logged
		AInstancedFoliageActor* Foliage = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(GetWorld());
		if (LogFoliageClasses.Num() > 0 && Foliage)
		{
			UE_LOG(SemLog, Log, TEXT(" \t Init foliage: "));

			// Get and iterate through the foliage components
			TInlineComponentArray<UInstancedStaticMeshComponent*> FoliageComponents;
			Foliage->GetComponents<UInstancedStaticMeshComponent>(FoliageComponents);
			for (const auto& FoliageCompItr : FoliageComponents)
			{
				// Check if one of the given foliage class names is present in the static mesh name
				for (const auto& FoliageClassNameItr : LogFoliageClasses)
				{
					if (FoliageCompItr->GetStaticMesh()->GetName().Contains(FoliageClassNameItr))
					{
						UE_LOG(SemLog, Log, TEXT(" \t\t %s --> bodies count: %i"), *FoliageClassNameItr, FoliageCompItr->InstanceBodies.Num())
							// Add the class name to the component map
							FoliageClassNameToComponent.Add(FoliageClassNameItr, FoliageCompItr);
						// Foliage class name found in the components, break
						break;
					}
				}
			}
		}

		// Check for roads
		TActorIterator<ALandscape> LandscapeIterator(GetWorld());
		if (LandscapeIterator)
		{
			ALandscape* MyLandscape = *LandscapeIterator;
			UE_LOG(SemLog, Log, TEXT(" \t Init road: "));			

			TSet<UActorComponent*> LandscapeComponents = MyLandscape->GetComponents();
			for (auto const& LandscapeCompItr : LandscapeComponents)
			{
				// Check if spline component (road)
				if (LandscapeCompItr->IsA(USplineMeshComponent::StaticClass()))
				{
					// Cast to spline component
					USplineMeshComponent* SplineComp = Cast<USplineMeshComponent>(LandscapeCompItr);
					UE_LOG(SemLog, Log, TEXT(" \t\t road segment: %s"), *SplineComp->GetName());
					RoadCompNameToComponent.Add(SplineComp->GetName(), SplineComp);
				}
			}
		}
	}
}

// Read previously stored unique names from file
bool ASLManager::ShouldGenerateNewUniqueNames(const FString Path)
{
	// Check if file exists, and see if it is in sync with the level
	if (IFileManager::Get().FileExists(*Path))
	{
		UE_LOG(SemLog, Log, TEXT(" ** Reading unique names:"));
		// Read string from file		
		FString JsonString;
		FFileHelper::LoadFileToString(JsonString, *Path);

		// Create a json object from the read string
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(JsonString);
		TSharedPtr<FJsonObject> RootJsonObject;
		if (FJsonSerializer::Deserialize(Reader, RootJsonObject))
		{
			UE_LOG(SemLog, Log, TEXT(" \t Reading actor unique names:"));
			// Get the actor to unique name array
			TArray< TSharedPtr<FJsonValue> > JsonArr = RootJsonObject->GetArrayField("actor_unique_names");
			// Check that the two arrays are of the same size
			if (ActorToSemLogInfo.Num() != JsonArr.Num())
			{
				UE_LOG(SemLog, Error, 
					TEXT(" !! Actor unique names not in sync! Current to previous actor number differ!"));
				ASLManager::CancelLogging();
				return false;
			}

			// Map that will store all the names and unique names from the json file
			TMap<FString, FString> NameToUniqueNameMap;
			// Iterate the json array to read the names and the unique names
			for (const auto& JsonItr : JsonArr)
			{
				NameToUniqueNameMap.Add(*JsonItr->AsObject()->GetStringField("name"),
					*JsonItr->AsObject()->GetStringField("unique_name"));
			}

			// Check if the current items to be logged are stored in the json array
			for (const auto& ActToInfoItr : ActorToSemLogInfo)
			{
				// Local copy of actor name
				const FString ActName = ActToInfoItr.Key->GetName();
				if (NameToUniqueNameMap.Contains(ActName))
				{
					ActorToUniqueName.Add(ActToInfoItr.Key, NameToUniqueNameMap[ActName]);
					UE_LOG(SemLog, Log, TEXT(" \t\t %s --> %s"), *ActName, *NameToUniqueNameMap[ActName]);
				}
				else
				{
					UE_LOG(SemLog, Error, TEXT(" !! Actor unique names not in sync! %s not found!"), *ActName);
					ASLManager::CancelLogging();
					return false;
				}
			}


			// Check if foliage is present
			if (FoliageClassNameToComponent.Num() > 0)			
			{
				UE_LOG(SemLog, Log, TEXT(" \t Reading foliage unique names:"));

				// Get the json foliage type array
				TArray< TSharedPtr<FJsonValue> > FoliageJsonArr = RootJsonObject->GetArrayField("foliage_unique_names");
				// Check that the two arrays are of the same size
				if (FoliageClassNameToComponent.Num() != FoliageJsonArr.Num())
				{
					UE_LOG(SemLog, Error, TEXT(" !! Foliage unique names not in sync! Number of types differ."));
					ASLManager::CancelLogging();
					return false;
				}

				// Get the foliage class names and their count
				TMap<FString, uint32> ClassToCount;
				for (const auto& FoliageJsonItr : FoliageJsonArr)
				{
					// Get the class name with the count
					const FString CurrClassName = FoliageJsonItr->AsObject()->GetStringField("class");
					const uint32 CurrCount = FoliageJsonItr->AsObject()->GetNumberField("count");
					ClassToCount.Add(CurrClassName, CurrCount);
					
					// Flag checking if the class and the count is in sync
					bool bIsNameAndCountInSync = false;

					// Check if class name is present in the current foliage
					for (const auto& FoliageClassNameToCompItr : FoliageClassNameToComponent)
					{
						// Get the foliage bodies
						TArray<FBodyInstance*> Bodies = FoliageClassNameToCompItr.Value->InstanceBodies;

						if (FoliageClassNameToCompItr.Value->GetStaticMesh()->GetName().Contains(CurrClassName) &&
							Bodies.Num() == CurrCount)
						{
							// Set sync flag to true
							bIsNameAndCountInSync = true;

							TArray< TSharedPtr<FJsonValue> > FoliageUniqueNameJsonArr =
								FoliageJsonItr->AsObject()->GetArrayField("id_to_unique_name");

							// Check if the number of unique ids are in sync with the declared ones
							if (FoliageUniqueNameJsonArr.Num() != CurrCount)
							{
								UE_LOG(SemLog, Error, TEXT(" !! Foliage unique names not in sync! Number of unique names differ."));
								ASLManager::CancelLogging();
								return false;
							}

							TArray<TPair<FBodyInstance*, FString>> FoliageUniqueNameArr;
							for (const auto& FoliageUniqueNameJsonItr : FoliageUniqueNameJsonArr)
							{								
								// Get the index and the unique name
								const uint32 CurrIndex = FoliageUniqueNameJsonItr->AsObject()->GetNumberField("index");
								const FString CurrUniqueName = FoliageUniqueNameJsonItr->AsObject()->GetStringField("unique_name");
								// Set and add the foliage key value pair
								TPair<FBodyInstance*, FString> FoliageKeyValPair;
								FoliageKeyValPair.Key = Bodies[CurrIndex];
								FoliageKeyValPair.Value = CurrUniqueName;
								FoliageUniqueNameArr.EmplaceAt(CurrIndex, FoliageKeyValPair);
								//UE_LOG(SemLog, Log, TEXT(" \t\t %i --> %s"), CurrIndex, *CurrUniqueName);
							}
							// Add foliage class name and the array of the unique names to the map
							FoliageComponentToUniqueNameArray.Emplace(FoliageClassNameToCompItr.Value, FoliageUniqueNameArr);
							// Foliage class name found in the components, break
							break;
						}
					}
					if (!bIsNameAndCountInSync)
					{
						UE_LOG(SemLog, Error, TEXT(" !! Foliage unique names not in sync! Class or count number differ."));
						ASLManager::CancelLogging();
						return false;
					}
				}
			}


			UE_LOG(SemLog, Log, TEXT(" \t Reading road segments unique names:"));

			RoadUniqueName = RootJsonObject->GetStringField("road_unique_name");

			// Get the actor to unique name array
			TArray< TSharedPtr<FJsonValue> > RoadJsonArr = RootJsonObject->GetArrayField("road_segments_unique_names");
			// Check that the two arrays are of the same size
			if (RoadCompNameToComponent.Num() != RoadJsonArr.Num())
			{
				UE_LOG(SemLog, Error,
					TEXT(" !! Road segments unique names not in sync! Current to previous number differ!"));
				ASLManager::CancelLogging();
				return false;
			}

			// Map that will store all the names and unique names from the json file
			TMap<FString, FString> RoadNameToUniqueNameMap;
			// Iterate the json array to read the names and the unique names
			for (const auto& RoadJsonItr : RoadJsonArr)
			{
				RoadNameToUniqueNameMap.Add(*RoadJsonItr->AsObject()->GetStringField("name"),
					*RoadJsonItr->AsObject()->GetStringField("unique_name"));
			}

			// Check if the current items to be logged are stored in the json array
			for (const auto& RoadCompNameToCompItr : RoadCompNameToComponent)
			{
				// Local copy of the road segment name
				const FString RoadSegmentName = RoadCompNameToCompItr.Key;
				if (RoadNameToUniqueNameMap.Contains(RoadSegmentName))
				{
					RoadComponentNameToUniqueName.Add(RoadSegmentName,
						RoadNameToUniqueNameMap[RoadSegmentName]);
					UE_LOG(SemLog, Log, TEXT(" \t\t %s --> %s"), *RoadSegmentName, *RoadNameToUniqueNameMap[RoadSegmentName]);
				}
				else
				{
					UE_LOG(SemLog, Error, TEXT(" !! Road segment unique names not in sync! %s not found!"), *RoadSegmentName);
					ASLManager::CancelLogging();
					return false;
				}
			}

		}
		else
		{
			UE_LOG(SemLog, Error, TEXT(" !! Unique names cannot be read! Json string: %s"), *JsonString);
			ASLManager::CancelLogging();
			return false;
		}

		// Succesfully read all the unique values
		return false;
	}
	else
	{
		UE_LOG(SemLog, Warning,
			TEXT(" ** No previous level unique names found at: %s, generating new ones!"), *Path);
		return true;
	}
}

// Generate items unique names
void ASLManager::GenerateNewUniqueNames()
{
	UE_LOG(SemLog, Log, TEXT(" ** Generating new unique names:"));
	// Generate unqiue names for the actors
	UE_LOG(SemLog, Log, TEXT(" \t Generating actor unique names:"));	
	for (const auto& ActToInfoItr : ActorToSemLogInfo)
	{
		FString UName = ActToInfoItr.Key->GetName();
		UName += (UName.Contains("_")) ? FSLUtils::GenerateRandomFString(4)
			: "_" + FSLUtils::GenerateRandomFString(4);
		ActorToUniqueName.Add(ActToInfoItr.Key, UName);
		UE_LOG(SemLog, Log, TEXT(" \t\t %s --> %s"), *ActToInfoItr.Key->GetName(), *UName);
	}

	// Generate unique names for the foliage components
	if (FoliageClassNameToComponent.Num() > 0)
	{
		UE_LOG(SemLog, Log, TEXT(" \t Generating foliage unique names:"));
		// Iterate through the foliage components
		for (const auto& FoliageClassNameToCompItr : FoliageClassNameToComponent)
		{
			// Get and iterate through the bodies
			TArray<FBodyInstance*> Bodies = FoliageClassNameToCompItr.Value->InstanceBodies;
			// Array of the unique names;
			TArray<TPair<FBodyInstance*, FString>> UniqueNamesArr;
			for (const auto& BodItr : Bodies)
			{	
				TPair<FBodyInstance*, FString> FoliageKeyValPair;
				FoliageKeyValPair.Key = BodItr;
				FoliageKeyValPair.Value = FoliageClassNameToCompItr.Key + "_" + FSLUtils::GenerateRandomFString(4);
				// Add key-val info to the array	
				UniqueNamesArr.Add(FoliageKeyValPair);
				//UE_LOG(SemLog, Log, TEXT(" \t\t unique name: %s"), *UniqueNamesArr.Last().Value);
			}
			// Add foliage class name and the array of the unique names to the map
			FoliageComponentToUniqueNameArray.Emplace(FoliageClassNameToCompItr.Value, UniqueNamesArr);
			UE_LOG(SemLog, Log, TEXT(" \t\t %s --> unique names count: %i"),
				*FoliageClassNameToCompItr.Key, FoliageClassNameToCompItr.Value->GetInstanceCount());
		}
	}

	// Generate unique names for the road components
	if (RoadCompNameToComponent.Num() > 0)
	{
		UE_LOG(SemLog, Log, TEXT(" \t Generating road unique names:"));

		RoadUniqueName = "MountainRoad_" + FSLUtils::GenerateRandomFString(4);
		UE_LOG(SemLog, Log, TEXT(" \t Road unique name: %s"), *RoadUniqueName);

		for (const auto& RoadCompItr : RoadCompNameToComponent)
		{
			FString RoadCompUniqueName = "RoadSegment_" + FSLUtils::GenerateRandomFString(4);
			RoadComponentNameToUniqueName.Add(RoadCompItr.Key, RoadCompUniqueName);
			UE_LOG(SemLog, Log, TEXT(" \t\t %s --> unique name: %s"),
				*RoadCompItr.Key, *RoadCompUniqueName);
		}
	}
}

// Write generated unique names to file
void ASLManager::StoreNewUniqueNames(const FString Path)
{
	UE_LOG(SemLog, Log, TEXT(" ** Writing unique names to [%s]:"), *Path);
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);


	// Actor unique names
	UE_LOG(SemLog, Log, TEXT(" \t Writing actor unique names:"));
	// Json array of actors
	TArray< TSharedPtr<FJsonValue> > JsonUniqueNamesArr;
	// Add actors to be logged to the Json array
	for (const auto& ActToUniqNameItr : ActorToUniqueName)
	{
		// Local copy of the actor name
		FString ActName = ActToUniqNameItr.Key->GetName();
		// Json location object
		TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
		// Add fields
		JsonObj->SetStringField("name", ActName);
		JsonObj->SetStringField("unique_name", ActToUniqNameItr.Value);
		// Add actor to Json array
		JsonUniqueNamesArr.Add(MakeShareable(new FJsonValueObject(JsonObj)));
		UE_LOG(SemLog, Log, TEXT(" \t\t %s --> %s"), *ActName, *ActToUniqNameItr.Value);
	}
	// Add actors to Json root
	JsonRootObj->SetArrayField("actor_unique_names", JsonUniqueNamesArr);


	// Check if foliage data is present
	if (FoliageClassNameToComponent.Num() > 0)
	{
		UE_LOG(SemLog, Log, TEXT(" \t Writing foliage unique names:"));
		// Json array of foliage type
		TArray< TSharedPtr<FJsonValue> > JsonFoliageTypeArr;
		// Iterate the foliage class types, and store the unique names
		for (const auto& FoliageClassNameToCompItr : FoliageClassNameToComponent)
		{
			// Get the current component unique names array
			TArray<TPair<FBodyInstance*, FString>> CurrCompUniqueNameArray =
				*FoliageComponentToUniqueNameArray.Find(FoliageClassNameToCompItr.Value);

			// Json foliage type object
			TSharedPtr<FJsonObject> JsonFoliageTypeObj = MakeShareable(new FJsonObject);
			// Add fields
			JsonFoliageTypeObj->SetStringField("class", FoliageClassNameToCompItr.Key);
			JsonFoliageTypeObj->SetNumberField("count", CurrCompUniqueNameArray.Num());

			// Json array of foliage unique names type
			TArray< TSharedPtr<FJsonValue> > JsonUniqueNameArr;
			uint32 Index = 0;
			for (const auto& UniqueNameItr : CurrCompUniqueNameArray)
			{
				// Json foliage type object
				TSharedPtr<FJsonObject> JsonFoliageUniqueNameObj = MakeShareable(new FJsonObject);
				// Add fields
				JsonFoliageUniqueNameObj->SetNumberField("index", Index);
				JsonFoliageUniqueNameObj->SetStringField("unique_name", UniqueNameItr.Value);
				// Add to array
				JsonUniqueNameArr.Add(MakeShareable(new FJsonValueObject(JsonFoliageUniqueNameObj)));
				Index++;
			}
			// Add unique names field
			JsonFoliageTypeObj->SetArrayField("id_to_unique_name", JsonUniqueNameArr);

			// Add foliage object to Json array
			JsonFoliageTypeArr.Add(MakeShareable(new FJsonValueObject(JsonFoliageTypeObj)));
			UE_LOG(SemLog, Log, TEXT(" \t\t %s --> unique names count: %i"),
				*FoliageClassNameToCompItr.Key, CurrCompUniqueNameArray.Num());
		}
		// Add foliage unique names to Json root
		JsonRootObj->SetArrayField("foliage_unique_names", JsonFoliageTypeArr);
	}

	// Generate unique names for the road components
	if (RoadCompNameToComponent.Num() > 0)
	{
		UE_LOG(SemLog, Log, TEXT(" \t Writing road unique name:"));
		UE_LOG(SemLog, Log, TEXT(" \t\t %s"), *RoadUniqueName);
		// Add road unique name to Json root
		JsonRootObj->SetStringField("road_unique_name", RoadUniqueName);

		UE_LOG(SemLog, Log, TEXT(" \t Writing road segments unique names:"));
		// Json array of road segments
		TArray< TSharedPtr<FJsonValue> > JsonRoadSegmentTypeArr;
		// Iterate the foliage class types, and store the unique names
		for (const auto& RoadCompItr : RoadComponentNameToUniqueName)
		{
			// Json location object
			TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
			// Add fields
			JsonObj->SetStringField("name", RoadCompItr.Key);
			JsonObj->SetStringField("unique_name", RoadCompItr.Value);
			// Add actor to Json array
			JsonRoadSegmentTypeArr.Add(MakeShareable(new FJsonValueObject(JsonObj)));
			UE_LOG(SemLog, Log, TEXT(" \t\t %s --> %s"), *RoadCompItr.Key, *RoadCompItr.Value);
		}

		// Add actors to Json root
		JsonRootObj->SetArrayField("road_segments_unique_names", JsonRoadSegmentTypeArr);
	}


	// Transform to string
	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);
	// Write string to file
	FFileHelper::SaveStringToFile(JsonOutputString, *Path);
}

// Cancel logging
void ASLManager::CancelLogging()
{
	// Set flags to false
	bStartLoggingAtLoadTime = false;
	bLogRawData = false;
	bLogSemanticMap = false;
	bLogSemanticEvents = false;

	// Disable tick
	SetActorTickEnabled(false);

	// Disable listening to events
	if (SemEventsExporter)
	{
		SemEventsExporter->SetListenToEvents(false);
	}

	UE_LOG(SemLog, Error, TEXT(" !! Logging cancelled!"));
}
