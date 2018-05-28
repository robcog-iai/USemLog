// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSemanticMapWriter.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// UOwl
#include "OwlSemanticMapIAIKitchen.h"
#include "OwlSemanticMapIAISupermarket.h"

// UUtils
#include "Tags.h"
#include "Ids.h"
#include "Conversions.h"

// Write semantic map to file
bool FSLSemanticMapWriter::WriteToFile(UWorld* World,
	EMapTemplateType TemplateType,
	const FString& InDirectory,
	const FString& InFilename)
{
	// Create the semantic map template
	TSharedPtr<FOwlSemanticMap> SemMap = CreateSemanticMapTemplate(TemplateType);

	// Add entries to map
	AddAllEntries(SemMap, World);

	// Write map to file
	FString FullFilePath = FPaths::ProjectDir() +
		InDirectory + TEXT("/") + InFilename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(SemMap->ToString(), *FullFilePath);
}

// Create semantic map template
TSharedPtr<FOwlSemanticMap> FSLSemanticMapWriter::CreateSemanticMapTemplate(EMapTemplateType TemplateType)
{
	if (TemplateType == EMapTemplateType::Default)
	{
		return MakeShareable(new FOwlSemanticMapIAI());
	}
	else if (TemplateType == EMapTemplateType::IAIKitchen)
	{
		return MakeShareable(new FOwlSemanticMapIAIKitchen());
	}
	else if (TemplateType == EMapTemplateType::IAISupermarket)
	{
		return MakeShareable(new FOwlSemanticMapIAISupermarket());
	}
	return MakeShareable(new FOwlSemanticMap());
}

// Add entries to the semantic map
void FSLSemanticMapWriter::AddAllEntries(TSharedPtr<FOwlSemanticMap> InSemMap, UWorld* World)
{
	// Iterate objects with SemLog tag key
	for (const auto& ObjToTagsItr : FTags::GetObjectsToKeyValuePairs(World, "SemLog"))
	{
		// Cache some tag id results
		const FString Id = ObjToTagsItr.Value.Contains("Id") ?
			ObjToTagsItr.Value["Id"] : TEXT("");
		const FString Class = ObjToTagsItr.Value.Contains("Class") ?
			ObjToTagsItr.Value["Class"] : TEXT("");

		// Take into account only objects with an id
		if (!Id.IsEmpty())
		{
			// Check if 
			if (!Class.IsEmpty())
			{				
				AddObjectEntry(InSemMap, ObjToTagsItr.Key, Id, Class);
			}
			else if (APhysicsConstraintActor* ConstrAct = Cast<APhysicsConstraintActor>(ObjToTagsItr.Key))
			{
				AddConstraintEntry(InSemMap, ConstrAct->GetConstraintComp(), Id);
			}
			else if (UPhysicsConstraintComponent* ConstrComp = Cast<UPhysicsConstraintComponent>(ObjToTagsItr.Key))
			{
				AddConstraintEntry(InSemMap, ConstrComp, Id);
			}
		}

		// Add class entries (Id not mandatory)
		if (!Class.IsEmpty())
		{
			const FString SubClass = ObjToTagsItr.Value.Contains("SubClass") ?
				ObjToTagsItr.Value["SubClass"] : TEXT("");
			AddClassDefinition(InSemMap, ObjToTagsItr.Key, Class, SubClass);
		}
	}
}

// Add object entry to the semantic map
void FSLSemanticMapWriter::AddObjectEntry(TSharedPtr<FOwlSemanticMap> InSemMap,
	UObject* Object,
	const FString& InId,
	const FString& InClass)
{
	// Create the object individual
	FOwlNode ObjIndividual = FOwlStatics::CreateObjectIndividual(InId, InClass);

	// Add parent property
	const FString ParentId = GetParentId(Object);
	if (!ParentId.IsEmpty())
	{
		ObjIndividual.ChildNodes.Add(FOwlStatics::CreateParentProperty(ParentId));
	}

	// Add children properties
	for (const auto& ChildId : GetAllChildIds(Object))
	{
		ObjIndividual.ChildNodes.Add(FOwlStatics::CreateChildProperty(ChildId));
	}

	// Add pose individual to map
	if (AActor* ActEntry = Cast<AActor>(Object))
	{
		// Generate unique id for the pose individual
		const FString PoseId = FIds::NewGuidInBase64Url();
		ObjIndividual.ChildNodes.Add(FOwlStatics::CreatePoseProperty(PoseId));
		InSemMap->Entries.Add(ObjIndividual);
		const FVector ROSLoc = FConversions::UToROS(ActEntry->GetActorLocation());
		const FQuat ROSQuat = FConversions::UToROS(ActEntry->GetActorQuat());
		InSemMap->Entries.Add(FOwlStatics::CreatePoseIndividual(PoseId, ROSLoc, ROSQuat));
	}
	else if (USceneComponent* CompEntry = Cast<USceneComponent>(Object))
	{
		// Generate unique id for the pose individual
		const FString PoseId = FIds::NewGuidInBase64Url();
		ObjIndividual.ChildNodes.Add(FOwlStatics::CreatePoseProperty(PoseId));
		InSemMap->Entries.Add(ObjIndividual);
		const FVector ROSLoc = FConversions::UToROS(CompEntry->GetComponentLocation());
		const FQuat ROSQuat = FConversions::UToROS(CompEntry->GetComponentQuat());
		InSemMap->Entries.Add(FOwlStatics::CreatePoseIndividual(PoseId, ROSLoc, ROSQuat));
	}
	else
	{
		// Obj has no pose info
		InSemMap->Entries.Add(ObjIndividual);
	}
}


// Add class entry
void FSLSemanticMapWriter::AddClassDefinition(TSharedPtr<FOwlSemanticMap> InSemMap,
	UObject* Object,
	const FString& InClass,
	const FString& InSubClass)
{
	// Return if class was already defined
	for (const auto& ClassDef : InSemMap->ClassDefinitions)
	{
		for (const auto& ClassAttr : ClassDef.Attributes)
		{
			if (ClassAttr.Key.Prefix.Equals("rdf") &&
				ClassAttr.Key.LocalName.Equals("about") &&
				ClassAttr.Value.LocalValue.Equals(InClass))
			{
				return;
			}
		}
	}

	// Create class definition entry
	FOwlNode ClassDefinition = FOwlStatics::CreateClassDefinition(InClass);
	ClassDefinition.Comment = TEXT("Class ") + InClass;

	// Check if subclass is known
	if (!InSubClass.IsEmpty())
	{
		ClassDefinition.ChildNodes.Add(FOwlStatics::CreateSubClassProperty(InSubClass));
	}

	// Add bounds if available
	if (AStaticMeshActor* ObjAsSMAct = Cast<AStaticMeshActor>(Object))
	{
		if (UStaticMeshComponent* SMComp = ObjAsSMAct->GetStaticMeshComponent())
		{
			const FVector BBSize = FConversions::CmToM(
				SMComp->Bounds.GetBox().GetSize());
			if (!BBSize.IsZero())
			{
				ClassDefinition.ChildNodes.Add(FOwlStatics::CreateDepthProperty(BBSize.X));
				ClassDefinition.ChildNodes.Add(FOwlStatics::CreateWidthProperty(BBSize.Y));
				ClassDefinition.ChildNodes.Add(FOwlStatics::CreateHeightProperty(BBSize.Z));
			}
		}
	}
	else if(UPrimitiveComponent* ObjAsPrimComp = Cast<UPrimitiveComponent>(Object))
	{
		const FVector BBSize = FConversions::CmToM(
			ObjAsPrimComp->Bounds.GetBox().GetSize());
		if (!BBSize.IsZero())
		{
			ClassDefinition.ChildNodes.Add(FOwlStatics::CreateDepthProperty(BBSize.X));
			ClassDefinition.ChildNodes.Add(FOwlStatics::CreateWidthProperty(BBSize.Y));
			ClassDefinition.ChildNodes.Add(FOwlStatics::CreateHeightProperty(BBSize.Z));
		}
	}
	InSemMap->ClassDefinitions.Add(ClassDefinition);
}

// Add constraint entry
void FSLSemanticMapWriter::AddConstraintEntry(TSharedPtr<FOwlSemanticMap> InSemMap,
	UPhysicsConstraintComponent* ConstraintComp,
	const FString& InId)
{

}

// Get children ids (only direct children, no grandchildren etc.)
TArray<FString> FSLSemanticMapWriter::GetAllChildIds(UObject* Object)
{
	// Array of children ids
	TArray<FString> Ids;

	// Check object type
	if (AActor* ObjAsActor = Cast<AActor>(Object))
	{
		// Iterate child actors (only direct children, no grandchildren etc.)
		TArray<AActor*> ChildActors;
		ObjAsActor->GetAllChildActors(ChildActors, false);
		for (const auto& ChildAct : ChildActors)
		{
			const FString ChildId = FTags::GetKeyValue(ChildAct, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FTags::HasKey(ChildAct, "SemLog", "Class"))
			{
				Ids.Add(ChildId);
			}
		}

		// Iterate child components (only direct children, no grandchildren etc.)
		TInlineComponentArray<UActorComponent*> ChildComponents;
		ObjAsActor->GetComponents(ChildComponents, false);
		for (const auto& ChildComp : ChildComponents)
		{
			const FString ChildId = FTags::GetKeyValue(ChildComp, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FTags::HasKey(ChildComp, "SemLog", "Class"))
			{
				Ids.Add(ChildId);
			}
		}
	}
	// Only scene components can have other components as children
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		// Iterate child components (only direct children, no grandchildren etc.)
		TArray<USceneComponent*> ChildComponents;
		ObjAsSceneComp->GetChildrenComponents(false, ChildComponents);
		for (const auto& ChildComp : ChildComponents)
		{
			const FString ChildId = FTags::GetKeyValue(ChildComp, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FTags::HasKey(ChildComp, "SemLog", "Class"))
			{
				Ids.Add(ChildId);
			}
		}
	}
	return Ids;
}

// Get parent id (empty string if none)
FString FSLSemanticMapWriter::GetParentId(UObject* Object)
{
	// Check object type
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		AActor* ParentAct = ObjAsAct->GetParentActor();
		const FString ParentActId = FTags::GetKeyValue(ParentAct, "SemLog", "Id");
		if (!ParentActId.IsEmpty() && FTags::HasKey(ParentAct, "SemLog", "Class"))
		{
			return ParentActId;
		}

		// If this actor was created by a child actor component
		UChildActorComponent* ParentComp = ObjAsAct->GetParentComponent();
		const FString ParentCompId = FTags::GetKeyValue(ParentComp, "SemLog", "Id");
		if (!ParentCompId.IsEmpty() && FTags::HasKey(ParentComp, "SemLog", "Class"))
		{
			return ParentCompId;
		}
	}
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		USceneComponent* AttachComp = ObjAsSceneComp->GetAttachParent();
		const FString AttachCompId = FTags::GetKeyValue(AttachComp, "SemLog", "Id");
		if (!AttachCompId.IsEmpty() && FTags::HasKey(AttachComp, "SemLog", "Class"))
		{
			return AttachCompId;
		}

		AActor* ParentAct = ObjAsSceneComp->GetAttachmentRootActor();
		const FString ParentActId = FTags::GetKeyValue(ParentAct, "SemLog", "Id");
		if (!ParentActId.IsEmpty() && FTags::HasKey(ParentAct, "SemLog", "Class"))
		{
			return ParentActId;
		}
	}
	else if (UActorComponent* ObjAsActComp = Cast<USceneComponent>(Object))
	{
		AActor* Owner = ObjAsActComp->GetOwner();
		const FString OwnerId = FTags::GetKeyValue(Owner, "SemLog", "Id");
		if (!OwnerId.IsEmpty() && FTags::HasKey(Owner, "SemLog", "Class"))
		{
			return OwnerId;
		}
	}
	// No parent
	return FString();
}
