// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSemanticMapWriter.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "Tags.h"
#include "Ids.h"
#include "Conversions.h"

#include "Owl/SemanticMapIAI.h"
#include "Owl/SemanticMapIAIKitchen.h"
#include "Owl/SemanticMapIAISupermarket.h"

using namespace SLOwl;

// Constructor
FSLSemanticMapWriter::FSLSemanticMapWriter()
{
	SetLogDirectory(TEXT("SemLog"));
}

// Constructor with init
FSLSemanticMapWriter::FSLSemanticMapWriter(UWorld* World, EMapTemplateType TemplateType, const FString& InDirectory)
{
	SetLogDirectory(InDirectory);
	Generate(World, TemplateType);
}

// Destructor
FSLSemanticMapWriter::~FSLSemanticMapWriter()
{
}

// Generate semantic map from world
void FSLSemanticMapWriter::Generate(UWorld* World, EMapTemplateType TemplateType)
{
	if (TemplateType == EMapTemplateType::Default)
	{
		SemMap = MakeShareable(new FSemanticMapIAI());
	}
	else if (TemplateType == EMapTemplateType::IAIKitchen)
	{
		SemMap = MakeShareable(new FSemanticMapIAIKitchen());
	}
	else if (TemplateType == EMapTemplateType::IAISupermarket)
	{
		SemMap = MakeShareable(new FSemanticMapIAISupermarket());
	}

	// Add entries
	AddEntries(World);
}

// Add semantic map entries
void FSLSemanticMapWriter::AddEntries(UWorld* World)
{
	// Iterate objects with SemLog tag key
	for (const auto& ObjToTagsItr : FTags::GetObjectsToKeyValuePairs(World, "SemLog"))
	{
		// Take into account only objects with an id and class value set
		if (ObjToTagsItr.Value.Contains("Id"))
		{
			const FString Id = ObjToTagsItr.Value["Id"];
			if (ObjToTagsItr.Value.Contains("Class"))
			{
				const FString Class = ObjToTagsItr.Value["Class"];
				AddObjectEntry(ObjToTagsItr.Key, Id, Class);
			}
			else if (UPhysicsConstraintComponent* ConstrComp = Cast<UPhysicsConstraintComponent>(ObjToTagsItr.Key))
			{

			}
			else if (APhysicsConstraintActor* ConstrAct = Cast<APhysicsConstraintActor>(ObjToTagsItr.Key))
			{

			}
		}

	}
}

// Add actor entry
void FSLSemanticMapWriter::AddObjectEntry(UObject* Object,
	const FString& InId,
	const FString& InClass)
{
	// Create the object individual
	FNode ObjIndividual = CreateObjectIndividual(InId, InClass);
		
	// Add pose individual to map
	if (AActor* ActEntry = Cast<AActor>(Object))
	{
		// Generate unique id for the pose individual
		const FString PoseId = FIds::NewGuidInBase64Url();
		ObjIndividual.ChildNodes.Add(CreatePoseProperty(PoseId));
		SemMap->Entries.Add(ObjIndividual);
		SemMap->Entries.Add(CreatePoseIndividual(PoseId, ActEntry->GetActorLocation(), ActEntry->GetActorQuat()));

		//if (AActor* AttachedParent = ActEntry->GetAttachParentActor())
		//{
		//	if (ObjToTagsMap.Contains(AttachedParent) &&
		//		ObjToTagsMap[AttachedParent].Contains("Id") &&
		//		ObjToTagsMap[AttachedParent].Contains("Class"))
		//	{

		//	}
		//}
	}
	else if (USceneComponent* CompEntry = Cast<USceneComponent>(Object))
	{
		// Generate unique id for the pose individual
		const FString PoseId = FIds::NewGuidInBase64Url();
		ObjIndividual.ChildNodes.Add(CreatePoseProperty(PoseId));
		SemMap->Entries.Add(ObjIndividual);
		SemMap->Entries.Add(CreatePoseIndividual(PoseId, CompEntry->GetComponentLocation(), CompEntry->GetComponentQuat()));
	}
	else
	{
		// Obj has no pose info
		SemMap->Entries.Add(ObjIndividual);
	}
}

// Create an object entry
SLOwl::FNode FSLSemanticMapWriter::CreateObjectIndividual(const FString& Id, const FString& Class)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName OwlNI("owl", "NamedIndividual");

	FNode ObjectIndividual(OwlNI, FAttribute(RdfAbout, FAttributeValue("log", Id)));
	ObjectIndividual.Comment = TEXT("Object " + Class + " " + Id);
	ObjectIndividual.ChildNodes.Add(CreateClassProperty(Class));
	return ObjectIndividual;
}

// Create class node
SLOwl::FNode FSLSemanticMapWriter::CreateClassProperty(const FString& InClass)
{
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName RdfType("rdf", "type");

	return FNode(RdfType, FAttribute(RdfResource, FAttributeValue("knowrob", InClass)));
}

// Create pose property
SLOwl::FNode FSLSemanticMapWriter::CreatePoseProperty(const FString& InId)
{
	const FPrefixName KbPose("knowrob", "pose");
	const FPrefixName RdfResource("rdf", "resource");

	return FNode(KbPose, FAttribute(RdfResource, FAttributeValue("log", InId)));
}

// Create a pose entry
FNode FSLSemanticMapWriter::CreatePoseIndividual(const FString& InId, const FVector& InLoc, const FQuat& InQuat)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfType("rdf", "type");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName OwlNI("owl", "NamedIndividual");

	// Attribute values constants
	const FAttributeValue AttrValPose("knowrob", "Pose");
	const FAttributeValue AttrValString("xsd", "string");
	
	// Pose individual
	FNode PoseIndividual(OwlNI, FAttribute(RdfAbout, FAttributeValue("log", InId)));
	FNode PoseClass(RdfType, FAttribute(RdfResource, AttrValPose));
	PoseIndividual.ChildNodes.Add(PoseClass);
	PoseIndividual.ChildNodes.Add(CreateQuaternionProperty(InQuat));
	PoseIndividual.ChildNodes.Add(CreateLocationProperty(InLoc));
	return PoseIndividual;
}

// Create a location node
FNode FSLSemanticMapWriter::CreateLocationProperty(const FVector& InLoc)
{
	const FPrefixName RdfDatatype("rdf", "datatype");
	const FPrefixName KbTransl("knowrob", "translation");
	const FAttributeValue AttrValString("xsd", "string");

	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FString LocStr = FString::Printf(TEXT("%f %f %f"),
		ROSLoc.X, ROSLoc.Y, ROSLoc.Z);
	return FNode(KbTransl, FAttribute(RdfDatatype, AttrValString), LocStr);
}

// Create a quaternion node
FNode FSLSemanticMapWriter::CreateQuaternionProperty(const FQuat& InQuat)
{
	const FPrefixName RdfDatatype("rdf", "datatype");
	const FPrefixName KbQuat("knowrob", "quaternion");
	const FAttributeValue AttrValString("xsd", "string");

	const FQuat ROSQuat = FConversions::UToROS(InQuat);
	const FString QuatStr = FString::Printf(TEXT("%f %f %f %f"),
		ROSQuat.W, ROSQuat.X, ROSQuat.Y, ROSQuat.Z);
	return FNode(KbQuat, FAttribute(RdfDatatype, AttrValString), QuatStr);
}

// Write semantic map to file
bool FSLSemanticMapWriter::WriteToFile(const FString& Filename)
{
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/") + Filename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(SemMap->ToString(), *FullFilePath);
}
