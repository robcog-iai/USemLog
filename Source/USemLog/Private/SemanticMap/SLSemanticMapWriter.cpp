// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SemanticMap/SLSemanticMapWriter.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
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
		// Generate default template
		FMapTemplateDefault(Declaration, EntityDefinitions, Namespaces,
			OntologyImports, PropertyDefinitions, DatatypeDefinitions,
			ClassDefinitions, Entries);

		SemMap = MakeShareable(new FSemanticMapIAI());
	}
	else if (TemplateType == EMapTemplateType::IAIKitchen)
	{
		// Generate default template
		FMapTemplateIAIKitchen(Declaration, EntityDefinitions, Namespaces,
			OntologyImports, PropertyDefinitions, DatatypeDefinitions,
			ClassDefinitions, Entries);

		SemMap = MakeShareable(new FSemanticMapIAIKitchen());
	}
	else if (TemplateType == EMapTemplateType::IAISupermarket)
	{
		// Generate default template
		FMapTemplateIAIKitchen(Declaration, EntityDefinitions, Namespaces,
			OntologyImports, PropertyDefinitions, DatatypeDefinitions,
			ClassDefinitions, Entries);

		SemMap = MakeShareable(new FSemanticMapIAIKitchen());
	}


	// Add entries
	AddEntries(World);
}

// Add semantic map entries
void FSLSemanticMapWriter::AddEntries(UWorld* World)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfType("rdf", "type");
	const FPrefixName KbPose("knowrob", "pose");
	const FPrefixName KbQuat("knowrob", "quaternion");
	const FPrefixName KbTransl("knowrob", "translation");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName RdfDatatype("rdf", "datatype");
	const FPrefixName OwlNI("owl", "NamedIndividual");

	// Iterate objects with SemLog tag key
	auto ObjToTagsMap = FTags::GetObjectsToKeyValuePairs(World, "SemLog");
	for (const auto& ObjToTagsItr : ObjToTagsMap)
	{
		// Take into account only objects with an id and class value set
		if (ObjToTagsItr.Value.Contains("Id") && ObjToTagsItr.Value.Contains("Class"))
		{
			const FString Id = ObjToTagsItr.Value["Id"];
			const FString Class = ObjToTagsItr.Value["Class"];
			AddEntry(ObjToTagsItr.Key, Id, Class, ObjToTagsMap);
		}
	}
}

// Add actor entry
void FSLSemanticMapWriter::AddEntry(UObject* Object,
	const FString& Id,
	const FString& Class,
	const TMap<UObject*, TMap<FString, FString>> ObjectsToTagsMap)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfType("rdf", "type");
	const FPrefixName KbPose("knowrob", "pose");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName OwlNI("owl", "NamedIndividual");

	// Entity individual node
	FNode Individual(OwlNI, FAttribute(RdfAbout, FAttributeValue("log", Id)));
	Individual.Comment = TEXT("Object " + Class + " " + Id);
	FNode EntryClass(RdfType, FAttribute(RdfResource, FAttributeValue("knowrob", Class)));
	Individual.ChildNodes.Add(EntryClass);
	const FString PoseId = FIds::NewGuidInBase64Url();
	FNode EntryPose(KbPose, FAttribute(RdfResource, FAttributeValue("log", PoseId)));
	Individual.ChildNodes.Add(EntryPose);

	SemMap->Entries.Add(Individual);
	
	if (AActor* ActEntry = Cast<AActor>(Object))
	{
		// Create and add pose entry
		SemMap->Entries.Add(CreatePoseEntry(ActEntry->GetActorLocation(), ActEntry->GetActorQuat(), PoseId));

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
		// Create and add pose entry
		SemMap->Entries.Add(CreatePoseEntry(CompEntry->GetComponentLocation(), CompEntry->GetComponentQuat(), PoseId));

	}
}

// Create a pose entry
FNode FSLSemanticMapWriter::CreatePoseEntry(const FVector& InLoc, const FQuat& InQuat, const FString& InId)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfType("rdf", "type");
	const FPrefixName KbQuat("knowrob", "quaternion");
	const FPrefixName KbTransl("knowrob", "translation");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName RdfDatatype("rdf", "datatype");
	const FPrefixName OwlNI("owl", "NamedIndividual");

	// Attribute values constants
	const FAttributeValue AttrValPose("knowrob", "Pose");
	const FAttributeValue AttrValString("xsd", "string");

	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// Pose individual
	FNode PoseIndividual(OwlNI, FAttribute(RdfAbout, FAttributeValue("log", InId)));
	FNode PoseClass(RdfType, FAttribute(RdfResource, AttrValPose));
	PoseIndividual.ChildNodes.Add(PoseClass);
	const FString QuatStr = FString::Printf(TEXT("%f %f %f %f"),
		ROSQuat.W, ROSQuat.X, ROSQuat.Y, ROSQuat.Z);
	FNode PoseQuat(KbQuat, FAttribute(RdfDatatype, AttrValString), QuatStr);
	PoseIndividual.ChildNodes.Add(PoseQuat);
	const FString LocStr = FString::Printf(TEXT("%f %f %f"),
		ROSLoc.X, ROSLoc.Y, ROSLoc.Z);
	FNode PoseTrans(KbTransl, FAttribute(RdfDatatype, AttrValString), LocStr);
	PoseIndividual.ChildNodes.Add(PoseTrans);

	return PoseIndividual;
}

// Write semantic map to file
bool FSLSemanticMapWriter::WriteToFile(const FString& Filename)
{
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/") + Filename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(ToString(), *FullFilePath);
}

// To string
FString FSLSemanticMapWriter::ToString() const
{
	return SemMap->ToString();
	//return ToDoc().ToString();
}

// Create map from the data
FDoc FSLSemanticMapWriter::ToDoc() const
{
	return SemMap->ToDoc();
	//FNode Root(FPrefixName("rdf", "RDF"), Namespaces);
	//Root.ChildNodes.Add(OntologyImports);
	//Root.ChildNodes.Append(PropertyDefinitions);
	//Root.ChildNodes.Append(DatatypeDefinitions);
	//Root.ChildNodes.Append(ClassDefinitions);
	//Root.ChildNodes.Append(Entries);
	//return FDoc(Declaration, EntityDefinitions, Root);
}