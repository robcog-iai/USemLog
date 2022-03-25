// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLAssetManager.h"
#include "ModuleManager.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#endif // WITH_EDITOR

// Ctor
USLAssetManager::USLAssetManager()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLAssetManager::~USLAssetManager()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();		
	}
}

// Init scanner
void USLAssetManager::Init(const FString& TaskId, const FString& ServerIp,
	uint16 ServerPort, ESLAssetAction InAction, bool bOverwrite)
{
	if (!bIsInit)
	{
		if (InAction == ESLAssetAction::NONE)
		{
			return;
		}

		Action = InAction;

		if (Action == ESLAssetAction::Move)
		{
			const FString Path = "/Game/SemLogAssets/"+ TaskId;
			if (!MoveCurrentLevelAssets(Path))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: Unable to move the assets"), *FString(__func__), __LINE__);
				return;
			}
		}
		else if (Action == ESLAssetAction::MoveAndUpload)
		{
			const FString Path = "/Game/SemLogAssets/" + TaskId;
			if (!MoveCurrentLevelAssets(Path))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: Unable to move the assets"), *FString(__func__), __LINE__);
				return;
			}

			// Change to the remaining action
			Action = ESLAssetAction::Upload;
		}

		if (Action == ESLAssetAction::Download || Action == ESLAssetAction::Upload)
		{
			if (!DBHandler.Connect(TaskId, ServerIp, ServerPort, Action, bOverwrite))
			{
				return;
			}
		}

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLAssetManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (Action == ESLAssetAction::Download || Action == ESLAssetAction::Upload)
		{
			DBHandler.Execute();
		}
		bIsStarted = true;
	}
}

// Finish scanning
void USLAssetManager::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if (Action == ESLAssetAction::Download || Action == ESLAssetAction::Upload)
		{
			DBHandler.Disconnect();
		}
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Move assets to path
bool USLAssetManager::MoveCurrentLevelAssets(const FString & Path)
{
#if WITH_EDITOR
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetsList;
	AssetRegistryModule.Get().GetAssetsByPackageName(GetCurrentLevel(), AssetsList);
	MoveReferencedObjects(GetCurrentLevel(), TEXT("/Game"), Path);
	MoveAssets(AssetsList, Path, TEXT("/Game"));
#endif // WITH_EDITOR
	return true;
}

FName USLAssetManager::GetCurrentLevel()
{
	// Load Main Level Asset
	ULevel* MainLevel = GetWorld()->GetCurrentLevel();

	// Process asset path. Example:"/Game/UEDPIE_0_MapMain.MapMain:PersistentLevel" to "/Game/MapMain"
	FString PackageName;
	TArray<FString> RawPath, Temp1, Temp2;
	MainLevel->GetPathName().ParseIntoArray(RawPath, *FString("/"), true);
	RawPath.Top().ParseIntoArray(Temp1, *FString("."), true);
	Temp1.Top().ParseIntoArray(Temp2, *FString(":"), true);

	for (int i = 0; i < RawPath.Num() - 1; i++)
	{
		PackageName += TEXT("/") + RawPath[i];
	}
	PackageName += TEXT("/") + Temp2[0];

	return FName(*PackageName);
}

void USLAssetManager::MoveReferencedObjects(FName PackageName, const FString& SourceBasePath, const FString& DestBasePath)
{
	
	

#if WITH_EDITOR
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FName> HardDependencies;
	//AssetRegistryModule.Get().GetDependencies(PackageName, HardDependencies, EAssetRegistryDependencyType::Hard);
	
	TArray<FName> SoftDependencies;
	//AssetRegistryModule.Get().GetDependencies(PackageName, SoftDependencies, EAssetRegistryDependencyType::Soft);

	TArray<FName> AllDependencies;
	AssetRegistryModule.Get().GetDependencies(PackageName, AllDependencies, UE::AssetRegistry::EDependencyCategory::All);

	TArray<FAssetData> AssetsList;

	if (AllDependencies.Num() > 0)
	{
		for (const FName Dependency : AllDependencies)
		{
			if (Dependency.ToString().StartsWith(TEXT("/Game/")) && !Dependency.ToString().EndsWith(TEXT("BuiltData")))
			{
				if (AssetRegistryModule.Get().GetAssetsByPackageName(Dependency, AssetsList))
					MoveReferencedObjects(Dependency, SourceBasePath, DestBasePath);
			}
		}
	}

	if (HardDependencies.Num() > 0)
	{
		for (const FName HardDependency : HardDependencies)
		{
			if (HardDependency.ToString().StartsWith(TEXT("/Game/")) && !HardDependency.ToString().EndsWith(TEXT("BuiltData")))
			{
				if (AssetRegistryModule.Get().GetAssetsByPackageName(HardDependency, AssetsList))
					MoveReferencedObjects(HardDependency, SourceBasePath, DestBasePath);
			}
		}
	}

	if (SoftDependencies.Num() > 0)
	{
		for (const FName SoftDependency : SoftDependencies)
		{
			if (SoftDependency.ToString().StartsWith(TEXT("/Game/")))
			{
				TArray<FAssetData> SoftAsset;
				if (AssetRegistryModule.Get().GetAssetsByPackageName(SoftDependency, SoftAsset))
				{
					if (SoftDependency.ToString().Equals(PackageName.ToString())) {
						continue;
					}

					if (SoftAsset.Last().AssetClass.ToString().Equals(TEXT("World")))
					{
						AssetsList.Add(SoftAsset.Last());
						MoveReferencedObjects(SoftDependency, SourceBasePath, DestBasePath);
					}
				}
			}
		}
	}

	if (AssetsList.Num() > 0)
	{
		MoveAssets(AssetsList, DestBasePath, SourceBasePath);
	}
#endif // WITH_EDITOR
}

void USLAssetManager::MoveAssets(TArray<FAssetData> AssetList, const FString& DestPath, const FString& SourcePath)
{
#if WITH_EDITOR
	TArray<UObject*> AssetObjects;
	for (int32 AssetIdx = 0; AssetIdx < AssetList.Num(); ++AssetIdx)
	{
		const FAssetData& AssetData = AssetList[AssetIdx];

		UObject* Obj = AssetData.GetAsset();
		if (Obj)
		{
			AssetObjects.Add(Obj);
		}
	}

	check(DestPath.Len() > 0);

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetsAndNames;
	for (auto AssetIt = AssetObjects.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		UObject* Asset = *AssetIt;

		if (!ensure(Asset))
		{
			continue;
		}

		FString PackagePath;
		FString ObjectName = Asset->GetName();

		// Only a DestPath was supplied, use it
		if (SourcePath.Len())
		{
			const FString CurrentPackageName = Asset->GetOutermost()->GetName();

			// This is a relative operation
			if (!ensure(CurrentPackageName.StartsWith(SourcePath)))
			{
				continue;
			}

			// Collect the relative path then use it to determine the new location
			// For example, if SourcePath = /Game/MyPath and CurrentPackageName = /Game/MyPath/MySubPath/MyAsset
			//     /Game/MyPath/MySubPath/MyAsset -> /MySubPath

			const int32 ShortPackageNameLen = FPackageName::GetLongPackageAssetName(CurrentPackageName).Len();
			const int32 RelativePathLen = CurrentPackageName.Len() - ShortPackageNameLen - SourcePath.Len() - 1; // -1 to exclude the trailing "/"
			const FString RelativeDestPath = CurrentPackageName.Mid(SourcePath.Len(), RelativePathLen);

			PackagePath = DestPath + RelativeDestPath;
		}
		else
		{
			// Only a DestPath was supplied, use it
			PackagePath = DestPath;
		}

		new(AssetsAndNames) FAssetRenameData(Asset, PackagePath, ObjectName);
	}

	if (AssetsAndNames.Num() > 0)
	{
		AssetToolsModule.Get().RenameAssetsWithDialog(AssetsAndNames);
	}
#endif // WITH_EDITOR
}