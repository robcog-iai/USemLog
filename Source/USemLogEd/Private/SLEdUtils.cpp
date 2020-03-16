// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)


#include "SLEdUtils.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"

// SL
#include "Editor/SLSemanticMapWriter.h"
#include "Vision/SLVisionCamera.h"
#include "SLManager.h"

// Utils
#include "Utils/SLTagIO.h"
#include "Utils/SLUUid.h"

// Write the semantic map
void FSLEdUtils::WriteSemanticMap(UWorld* World, bool bOverwrite)
{
	FSLSemanticMapWriter SemMapWriter;
	FString TaskDir;

	for (TActorIterator<ASLManager> ActItr(World); ActItr; ++ActItr)
	{
		TaskDir = *ActItr->GetTaskId();
		break;
	}
	if(TaskDir.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the semantic manager to read the task id, set to default.."),
			*FString(__func__), __LINE__);
		TaskDir = "DefaultTaskId";
	}
	
	// Generate map and write to file
	SemMapWriter.WriteToFile(World, ESLOwlSemanticMapTemplate::IAIKitchen, TaskDir, TEXT("SemanticMap"), bOverwrite);
}

// Write unique IDs
void FSLEdUtils::WriteUniqueIds(UWorld* World, bool bOverwrite)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("Id");

	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		/* SMA */
		if (ActItr->IsA(AStaticMeshActor::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
		}

		/* SkMA */
		if (ActItr->IsA(ASkeletalMeshActor::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);

			// Get the semantic data component containing the semantics (class names mask colors) about the bones
			if (UActorComponent* AC = ActItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
			{
				// Load existing visual mask values from the skeletal data
				USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
				for (auto& Pair : SkDC->SemanticBonesData)
				{
					// Double check if bone has a semantic class
					if (!Pair.Value.IsClassSet())
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Semantic bones should have a class name set.."), *FString(__func__), __LINE__);
						continue;
					}

					// Check if the bone has id data
					if (bOverwrite)
					{
						Pair.Value.Id = FSLUuid::NewGuidInBase64Url();

						// Add the data to the map used at by the metadatalogger as well
						if (SkDC->AllBonesData.Contains(Pair.Key))
						{
							SkDC->AllBonesData[Pair.Key].Id = Pair.Value.Id;
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
								*FString(__func__), __LINE__, *Pair.Key.ToString());
						}
					}
					else if (Pair.Value.Id.IsEmpty())
					{
						Pair.Value.Id = FSLUuid::NewGuidInBase64Url();

						// Add the data to the map used at runtime as well
						if (SkDC->AllBonesData.Contains(Pair.Key))
						{
							SkDC->AllBonesData[Pair.Key].Id = Pair.Value.Id;
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
								*FString(__func__), __LINE__, *Pair.Key.ToString());
						}
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
					*FString(__func__), __LINE__, *ActItr->GetName());
			}
		}

		/* Joints */
		if (ActItr->IsA(APhysicsConstraintActor::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
		}

		/* Vision cameras */
		if (ActItr->IsA(ASLVisionCamera::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
		}
	}
}

// Write class names
void FSLEdUtils::WriteClassNames(UWorld* World, bool bOverwrite)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("Class");

	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		FString ClassName = GetClassName(*ActItr);
		if (!ClassName.IsEmpty())
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, ClassName, bOverwrite);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not get the class name for %s.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
	}
}

// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
FString FSLEdUtils::GetClassName(AActor* Actor, bool bDefaultToLabelName)
{
	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Actor))
	{
		if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			FString ClassName = SMC->GetStaticMesh()->GetFullName();
			int32 FindCharPos;
			ClassName.FindLastChar('.', FindCharPos);
			ClassName.RemoveAt(0, FindCharPos + 1);
			if (!ClassName.RemoveFromStart(TEXT("SM_")))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s StaticMesh has no SM_ prefix in its name.."),
					*FString(__func__), __LINE__, *Actor->GetName());
			}
			return ClassName;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SMC.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			return FString();
		}
	}
	else if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(Actor))
	{
		if (USkeletalMeshComponent* SkMC = SkMA->GetSkeletalMeshComponent())
		{
			FString ClassName = SkMC->SkeletalMesh->GetFullName();
			int32 FindCharPos;
			ClassName.FindLastChar('.', FindCharPos);
			ClassName.RemoveAt(0, FindCharPos + 1);
			ClassName.RemoveFromStart(TEXT("SK_"));
			return ClassName;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SkMC.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			return FString();
		}
	}
	else if (ASLVisionCamera* VCA = Cast<ASLVisionCamera>(Actor))
	{
		static const FString TagType = "SemLog";
		static const FString TagKey = "Class";
		FString ClassName = "View";

		// Check attachment actor
		if (AActor* AttAct = Actor->GetAttachParentActor())
		{
			if (Actor->GetAttachParentSocketName() != NAME_None)
			{
				return Actor->GetAttachParentSocketName().ToString() + ClassName;
			}
			else
			{
				FString AttParentClass = FSLTagIO::GetValue(AttAct, TagType, TagKey);
				if (!AttParentClass.IsEmpty())
				{
					return AttParentClass + ClassName;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Attached parent %s has no semantic class (yet?).."),
						*FString(__func__), __LINE__, *AttAct->GetName());
					return ClassName;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not attached to any actor.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			return ClassName;
		}
	}
	else if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(Actor))
	{
		FString ClassName = "Joint";

		if (UPhysicsConstraintComponent* PCC = PCA->GetConstraintComp())
		{
			if (PCC->ConstraintInstance.GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
			{
				return "Linear" + ClassName;
			}
			else if (PCC->ConstraintInstance.GetAngularSwing1Motion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetAngularSwing2Motion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetAngularTwistMotion() != ELinearConstraintMotion::LCM_Locked)
			{
				return "Revolute" + ClassName;
			}
			else
			{
				return "Fixed" + ClassName;
			}
		}
		return ClassName;
	}
	else if (bDefaultToLabelName)
	{
		return Actor->GetActorLabel();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not get the semantic class for %s .."),
			*FString(__func__), __LINE__, *Actor->GetName());
		return FString();
	}
}
