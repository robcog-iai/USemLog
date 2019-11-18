// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEditorToolkit.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"

// UUtils
#include "Ids.h"
#include "Tags.h"
#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR


// Ctor
FSLEditorToolkit::FSLEditorToolkit()
{
}

// Dtor
FSLEditorToolkit::~FSLEditorToolkit()
{
}

// Create the semantic map
void FSLEditorToolkit::WriteSemanticMap(UWorld* World, bool bOverwrite, const FString& TaskId, const FString& Filename, ESLOwlSemanticMapTemplate Template)
{
	// TODO overwrite option
	FSLSemanticMapWriter SemMapWriter;
	SemMapWriter.WriteToFile(World, Template, TaskId, Filename);
}

// Write class properties to the tags
void FSLEditorToolkit::WriteClassProperties(UWorld* World, bool bOverwrite)
{
	const FString TagType = TEXT("SemLog");
	const FString TagKey = TEXT("Class");

	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			// Ignore if actor is already tagged
			if (!FTags::HasKey(*ActItr, TagType, TagKey))
			{
				// Ignore if component is already tagged
				if (!FTags::HasKey(SMC, TagType, TagKey))
				{
					// Get the class name from the asset name
					FString ClassName = SMC->GetStaticMesh()->GetFullName();

					// Remove path info and prefix
					int32 FindCharPos;
					ClassName.FindLastChar('.', FindCharPos);
					ClassName.RemoveAt(0, FindCharPos + 1);
					ClassName.RemoveFromStart(TEXT("SM_"));

					// Check if the class should be added to the actor or the component
					if (FTags::HasType(*ActItr, TagType))
					{
						// Tag the actor because it is semantically tagged but is missing the  key-value pair
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, ClassName);
					}
					else if (FTags::HasType(SMC, TagType))
					{
						// Tag the component because it is semantically tagged but is missing the  key-value pair
						FTags::AddKeyValuePair(SMC, TagType, TagKey, ClassName);
					}
					else
					{
						// None have the tag type and key, generate new one to the actor
						FTags::AddTagType(*ActItr, TagType);
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, ClassName);
					}
				}
				else if (bOverwrite)
				{
					// Get the class name from the asset name
					FString ClassName = SMC->GetStaticMesh()->GetFullName();
					// Remove path info and prefix
					int32 FindCharPos;
					ClassName.FindLastChar('.', FindCharPos);
					ClassName.RemoveAt(0, FindCharPos + 1);
					ClassName.RemoveFromStart(TEXT("SM_"));
					FTags::AddKeyValuePair(SMC, TagType, TagKey, ClassName, true);
				}

			}
			else if (bOverwrite)
			{
				// Get the class name from the asset name
				FString ClassName = SMC->GetStaticMesh()->GetFullName();
				// Remove path info and prefix
				int32 FindCharPos;
				ClassName.FindLastChar('.', FindCharPos);
				ClassName.RemoveAt(0, FindCharPos + 1);
				ClassName.RemoveFromStart(TEXT("SM_"));
				FTags::AddKeyValuePair(*ActItr, TagType, TagKey, ClassName, true);
			}
		}
	}

	/* Skeletal actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (USkeletalMeshComponent* SkMC = ActItr->GetSkeletalMeshComponent())
		{
			// Ignore if actor is already tagged
			if (!FTags::HasKey(*ActItr, TagType, TagKey))
			{
				// Ignore if component is already tagged
				if (!FTags::HasKey(SkMC, TagType, TagKey))
				{
					// Get the class name from the asset name
					FString ClassName = SkMC->GetFullName();

					// Remove path info and prefix
					int32 FindCharPos;
					ClassName.FindLastChar('.', FindCharPos);
					ClassName.RemoveAt(0, FindCharPos + 1);
					ClassName.RemoveFromStart(TEXT("SK_"));

					// Check if the class should be added to the actor or the component
					if (FTags::HasType(*ActItr, TagType))
					{
						// Tag the actor because it is semantically tagged but is missing the  key-value pair
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, ClassName);
					}
					else if (FTags::HasType(SkMC, TagType))
					{
						// Tag the component because it is semantically tagged but is missing the  key-value pair
						FTags::AddKeyValuePair(SkMC, TagType, TagKey, ClassName);
					}
					else
					{
						// None have the tag type and key, generate new one to the actor
						FTags::AddTagType(*ActItr, TagType);
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, ClassName);
					}
				}
				else if (bOverwrite)
				{
					// Get the class name from the asset name
					FString ClassName = SkMC->GetFullName();
					// Remove path info and prefix
					int32 FindCharPos;
					ClassName.FindLastChar('.', FindCharPos);
					ClassName.RemoveAt(0, FindCharPos + 1);
					ClassName.RemoveFromStart(TEXT("SK_"));
					FTags::AddKeyValuePair(SkMC, TagType, TagKey, ClassName, true);
				}

			}
			else if (bOverwrite)
			{
				// Get the class name from the asset name
				FString ClassName = SkMC->GetFullName();
				// Remove path info and prefix
				int32 FindCharPos;
				ClassName.FindLastChar('.', FindCharPos);
				ClassName.RemoveAt(0, FindCharPos + 1);
				ClassName.RemoveFromStart(TEXT("SK_"));
				FTags::AddKeyValuePair(*ActItr, TagType, TagKey, ClassName, true);
			}
		}
	}
}

// Write unique id properties
void FSLEditorToolkit::WriteUniqueIdProperties(UWorld* World, bool bOverwrite)
{
#if WITH_EDITOR
	const FString TagType = TEXT("SemLog");
	const FString TagKey = TEXT("Id");
	
	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			// Apply the changes in the editor world
			AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr);
			if(!EdAct)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find %s in editor world.."),
					*FString(__func__), __LINE__, *ActItr->GetName());
				continue;
			}
			
			// Ignore if actor is already tagged
			if (!FTags::HasKey(EdAct, TagType, TagKey))
			{
				// Ignore if component is already tagged
				if (!FTags::HasKey(SMC, TagType, TagKey))
				{
					// Generate a unique id
					const FString Id = FIds::NewGuidInBase64();

					// Check if the class should be added to the actor or the component
					if (FTags::HasType(EdAct, TagType))
					{
						// Tag the actor because it is semantically tagged but is missing the key-value pair
						FTags::AddKeyValuePair(EdAct, TagType, TagKey, Id);
					}
					else if (FTags::HasType(SMC, TagType))
					{
						// Tag the component because it is semantically tagged but is missing the key-value pair
						FTags::AddKeyValuePair(SMC, TagType, TagKey, Id);
					}
					else
					{
						// None have the tag type, generate new one to the actor
						FTags::AddTagType(EdAct, TagType);
						FTags::AddKeyValuePair(EdAct, TagType, TagKey, Id);
					}
				}
				else if (bOverwrite)
				{
					// Generate a unique id
					const FString Id = FIds::NewGuidInBase64();
					FTags::AddKeyValuePair(SMC, TagType, TagKey, Id, true);
				}
			}
			else if (bOverwrite)
			{
				// Generate a unique id
				const FString Id = FIds::NewGuidInBase64();
				FTags::AddKeyValuePair(EdAct, TagType, TagKey, Id, true);
			}
		}
	}

	/* Skeletal meshes */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (USkeletalMeshComponent* SkMC = ActItr->GetSkeletalMeshComponent())
		{
			// Ignore if actor is already tagged
			if (!FTags::HasKey(*ActItr, TagType, TagKey))
			{
				// Ignore if component is already tagged
				if (!FTags::HasKey(SkMC, TagType, TagKey))
				{
					// Generate a unique id
					const FString Id = FIds::NewGuidInBase64();

					// Check if the class should be added to the actor or the component
					if (FTags::HasType(*ActItr, TagType))
					{
						// Tag the actor because it is semantically tagged but is missing the key-value pair
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, Id);
					}
					else if (FTags::HasType(SkMC, TagType))
					{
						// Tag the component because it is semantically tagged but is missing the key-value pair
						FTags::AddKeyValuePair(SkMC, TagType, TagKey, Id);
					}
					else
					{
						// None have the tag type, generate new one to the actor
						FTags::AddTagType(*ActItr, TagType);
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, Id);
					}
				}
				else if (bOverwrite)
				{
					// Generate a unique id
					const FString Id = FIds::NewGuidInBase64();
					FTags::AddKeyValuePair(SkMC, TagType, TagKey, Id, true);
				}
			}
			else if (bOverwrite)
			{
				// Generate a unique id
				const FString Id = FIds::NewGuidInBase64();
				FTags::AddKeyValuePair(*ActItr, TagType, TagKey, Id, true);
			}
		}
	}

	/* Constraint actors */
	for (TActorIterator<APhysicsConstraintActor> ActItr(World); ActItr; ++ActItr)
	{
		// Continue only if a valid mesh component is available
		if (UPhysicsConstraintComponent* PCC = ActItr->GetConstraintComp())
		{
			// Ignore if actor is already tagged
			if (!FTags::HasKey(*ActItr, TagType, TagKey))
			{
				// Ignore if component is already tagged
				if (!FTags::HasKey(PCC, TagType, TagKey))
				{
					// Generate a unique id
					const FString Id = FIds::NewGuidInBase64();

					// Check if the class should be added to the actor or the component
					if (FTags::HasType(*ActItr, TagType))
					{
						// Tag the actor because it is semantically tagged but is missing the key-value pair
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, Id);
					}
					else if (FTags::HasType(PCC, TagType))
					{
						// Tag the component because it is semantically tagged but is missing the key-value pair
						FTags::AddKeyValuePair(PCC, TagType, TagKey, Id);
					}
					else
					{
						// None have the tag type, generate new one to the actor
						FTags::AddTagType(*ActItr, TagType);
						FTags::AddKeyValuePair(*ActItr, TagType, TagKey, Id);
					}
				}
				else if (bOverwrite)
				{
					// Generate a unique id
					const FString Id = FIds::NewGuidInBase64();
					FTags::AddKeyValuePair(PCC, TagType, TagKey, Id, true);
				}
			}
			else if (bOverwrite)
			{
				// Generate a unique id
				const FString Id = FIds::NewGuidInBase64();
				FTags::AddKeyValuePair(*ActItr, TagType, TagKey, Id, true);
			}
		}
	}
#endif // WITH_EDITOR	
}
