// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisMaskHelper.h"
#include "Tags.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

// Default constructor
FSLVisMaskHelper::FSLVisMaskHelper()
{
}

// Init material constructor
FSLVisMaskHelper::FSLVisMaskHelper(UMaterialInterface* InDefaultMaskMaterial) : DefaultMaskMaterial(InDefaultMaskMaterial)
{
}

// Init
void FSLVisMaskHelper::Init(UWorld* World)
{
	if (!bIsInit && World && DefaultMaskMaterial)
	{
		// Iterate static meshes
		for (TActorIterator<AStaticMeshActor> SMAItr(World); SMAItr; ++SMAItr)
		{
			if (SMAItr->Tags.Num() > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Tag=%s"), TEXT(__FUNCTION__), __LINE__, *SMAItr->Tags[0].ToString());
			}

			if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
			{
				// Check if the entity is annotated with a semantic color mask
				FString ColorHex = FTags::GetValue(*SMAItr,"SemLog", "VisMask");
				if (ColorHex.IsEmpty())
				{
					ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
					if (ColorHex.IsEmpty())
					{
						// Generate 
					}

				}
			}
		}

		// TODO include semantic data from skeletal meshes as well
		// Iterate skeletal meshes

		UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
		bIsInit = true;
	}
}