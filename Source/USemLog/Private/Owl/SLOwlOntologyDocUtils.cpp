// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Owl/SLOwlOntologyDocUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"

#include "EngineUtils.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// Owl
#include "Owl/SLOwlDoc.h"

#if SL_WITH_ROS_CONVERSIONS
#include "Conversions.h"
#endif // SL_WITH_ROS_CONVERSIONS

/* Common structures */
// Owl
const FSLOwlPrefixName FSLOwlOntologyDocUtils::OwlNamedIndividual("owl", "NamedIndividual");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::OwlClass("owl", "Class");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::OwlRestriction("owl", "Restriction");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::OwlHasValue("owl", "hasValue");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::OwlOnProperty("owl", "onProperty");
// Rdf
const FSLOwlPrefixName FSLOwlOntologyDocUtils::RdfAbout("rdf", "about");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::RdfType("rdf", "type");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::RdfDatatype("rdf", "datatype");
const FSLOwlPrefixName FSLOwlOntologyDocUtils::RdfResource("rdf", "resource");
// Rdfs
const FSLOwlPrefixName FSLOwlOntologyDocUtils::RdfsSubClassOf("rdfs", "subClassOf");
// AV
const FSLOwlAttributeValue FSLOwlOntologyDocUtils::AVFloat("xsd", "float");

// Steps for creating and printing the semantic map with default arguments
bool FSLOwlOntologyDocUtils::CreateAndPrintDoc(UWorld* World, bool bOverwrite, ESLOwlOntologyTemplateTypes Type)
{
    // The semantic map document
    FSLOwlDoc SemMapOwlDoc = GetDocumentTemplate(Type);

    // Add map individual, set the task id
    FString TaskId = "None";
    //for (TActorIterator<ASLManager> ActItr(World); ActItr; ++ActItr)
    //{

    //    TaskId = *ActItr->GetTaskId();
    //    break;
    //}
    FSLOwlOntologyDocUtils::AddMapIndividual(SemMapOwlDoc, TaskId);

    // Iterate individuals from the world
    for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
    {
        if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(*ActItr))
        {
            FSLOwlOntologyDocUtils::AddIndividual(SemMapOwlDoc, BI);
        }
    }

    // Print the document
    return FSLOwlOntologyDocUtils::PrintDoc(SemMapOwlDoc, "/SL/" + TaskId, TEXT("SemanticMap"), bOverwrite);
}

// Print the document to file
bool FSLOwlOntologyDocUtils::PrintDoc(const FSLOwlDoc& InDoc, const FString& Path, const FString& Filename, bool bOverwrite)
{
    FString FullPath;

    if (!Path.Contains(FPaths::ProjectDir()))
    {
        FullPath.Append(FPaths::ProjectDir() + "/" + Path);
    }

    if (Filename.Contains(".owl"))
    {
        FullPath.Append("/" + Filename);
    }
    else
    {
        FullPath.Append("/" + Filename + ".owl");
    }

    FPaths::RemoveDuplicateSlashes(FullPath);

    if (!bOverwrite && FPaths::FileExists(FullPath))
    {
        return false;
    }

    return FFileHelper::SaveStringToFile(InDoc.ToString(), *FullPath);
}

// Create a semantic map document template
FSLOwlDoc FSLOwlOntologyDocUtils::GetDocumentTemplate(ESLOwlOntologyTemplateTypes Type)
{
    FSLOwlDoc TemplateDoc;
    AddEntityDefinitions(TemplateDoc, Type);
    AddNamespaceDeclarations(TemplateDoc, Type);
    AddOntologies(TemplateDoc, Type);

    // Add coments
    if (Type != ESLOwlOntologyTemplateTypes::NONE)
    {
        /**
        	<!-- Property Definitions -->
	        <owl:ObjectProperty rdf:about="&knowrob;describedInMap"/>
	        <owl:ObjectProperty rdf:about="&knowrob;pathToCadModel"/>
        **/
        TemplateDoc.AddPropertyDefinition(FOwlCommentNode("Property Definitions:"));
        TemplateDoc.AddPropertyDefinition(KRNs, "describedInMap");
        TemplateDoc.AddPropertyDefinition(KRNs, "pathToCadModel");

        /**
            <!-- Datatype Definitions -->
            <owl:DatatypeProperty rdf:about="&knowrob;quaternion"/>
            <owl:DatatypeProperty rdf:about="&knowrob;translation"/>
        **/
        TemplateDoc.AddDatatypeDefinition(FOwlCommentNode("Datatype Definitions:"));
        TemplateDoc.AddDatatypeDefinition(KRNs, "quaternion");
        TemplateDoc.AddDatatypeDefinition(KRNs, "translation");

        /**
            <!-- Class Definitions -->
            <owl:Class rdf:about="&knowrob;Pose"/> 
        **/
        TemplateDoc.AddClassDefinition(FOwlCommentNode("Class Definitions:"));
        TemplateDoc.AddClassDefinition(KRNs, "Pose");

        TemplateDoc.AddIndividual(FOwlCommentNode("Individuals:"));
    }

    return TemplateDoc;
}

// Add map boilerplate entity definitions
void FSLOwlOntologyDocUtils::AddEntityDefinitions(FSLOwlDoc& Doc, ESLOwlOntologyTemplateTypes Type)
{
    /**
    <!DOCTYPE rdf:RDF[
        <!ENTITY owl "http://www.w3.org/2002/07/owl#">
        <!ENTITY xsd "http://www.w3.org/2001/XMLSchema#">
        <!ENTITY knowrob "http://knowrob.org/kb/knowrob.owl#">
        <!ENTITY rdfs "http://www.w3.org/2000/01/rdf-schema#">
        <!ENTITY rdf "http://www.w3.org/1999/02/22-rdf-syntax-ns#">
        <!ENTITY ameva "http://knowrob.org/kb/ameva.owl#">
    ]>
    **/

    if (Type != ESLOwlOntologyTemplateTypes::NONE)
    {
        Doc.AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
        Doc.AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
        Doc.AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
        Doc.AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
        Doc.AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    }

    if (Type == ESLOwlOntologyTemplateTypes::Ameva)
    {
        Doc.AddEntityDefintion(AmevaNs, "http://knowrob.org/kb/" + FString(AmevaNs) + ".owl#");
    }
}

void FSLOwlOntologyDocUtils::AddNamespaceDeclarations(FSLOwlDoc& Doc, ESLOwlOntologyTemplateTypes Type)
{
    /**
        <rdf:RDF xmlns="http://knowrob.org/kb/ameva.owl#"
             xml:base="http://knowrob.org/kb/ameva.owl#"
             xmlns:owl="http://www.w3.org/2002/07/owl#"
             xmlns:xsd="http://www.w3.org/2001/XMLSchema#"
             xmlns:knowrob="http://knowrob.org/kb/knowrob.owl#"
             xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
             xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
             xmlns:ameva="http://knowrob.org/kb/ameva.owl#">
    **/

    if (Type != ESLOwlOntologyTemplateTypes::NONE)
    {
        Doc.AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + FString(AmevaNs) + ".owl#");
        Doc.AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + FString(AmevaNs) + ".owl#");
        Doc.AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
        Doc.AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
        Doc.AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
        Doc.AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
        Doc.AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    }

    if (Type == ESLOwlOntologyTemplateTypes::Ameva)
    {
        Doc.AddNamespaceDeclaration("xmlns", FString(AmevaNs), "http://knowrob.org/kb/" + FString(AmevaNs) + ".owl#");
       
    }
}

// Add map ontology imports 
void FSLOwlOntologyDocUtils::AddOntologies(FSLOwlDoc& Doc, ESLOwlOntologyTemplateTypes Type)
{
    /**
        <!-- Ontologies -->
        <owl:Ontology rdf:about="http://knowrob.org/kb/.owl">
            <owl:imports rdf:resource="package://knowrob/owl/knowrob.owl"/>
            <owl:imports rdf:resource="package://knowrob/owl/ameva.owl"/>
        </owl:Ontology>
    **/

    if (Type == ESLOwlOntologyTemplateTypes::Ameva)
    {
        Doc.OntologyName = FString(AmevaNs);
        Doc.CreateOntologyNode();
        Doc.AddOntologyImport("package://knowrob/owl/knowrob.owl");
        Doc.AddOntologyImport("package://knowrob/owl/" + FString(AmevaNs) + ".owl");
    }
}

// Add map individual to the document
void FSLOwlOntologyDocUtils::AddMapIndividual(FSLOwlDoc& Doc, const FString& MapId)
{
    /**
        <!-- InClass Definitions -->
        <owl:InClass rdf:about="&knowrob;SemanticEnvironmentMap"/>

        <!-- Semantic Map none -->
        <owl:NamedIndividual rdf:about="&ameva;map_id">
            <rdf:type rdf:resource="&knowrob;SemanticEnvironmentMap"/>
        </owl:NamedIndividual>
    **/
    FSLOwlAttributeValue SemEnvMapAV(KRNs, "SemanticEnvironmentMap");

    FSLOwlNode SemMapNode(OwlNamedIndividual);

    SemMapNode.AddAttribute(FSLOwlAttribute(RdfAbout,
        FSLOwlAttributeValue(AmevaNs, MapId)));


    SemMapNode.AddChildNode(FSLOwlNode(RdfType,
        FSLOwlAttribute(RdfResource, SemEnvMapAV)));

    SemMapNode.SetComment(TEXT("Semantic Map ") + MapId);

    Doc.AddClassDefinition(SemEnvMapAV);
    Doc.AddIndividual(SemMapNode);
}

// Add individual to document
bool FSLOwlOntologyDocUtils::AddIndividual(FSLOwlDoc& Doc, USLBaseIndividual* Individual)
{
    if (Individual->IsLoaded())
    {
        AddUniqueClassDefinition(Doc, Individual);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s::%d individual %s is not loaded.."),
			*FString(__FUNCTION__), __LINE__, *Individual->GetFullName());
        return false;
    }
}

// Add class definition if it does not exist
void FSLOwlOntologyDocUtils::AddUniqueClassDefinition(FSLOwlDoc& Doc, USLBaseIndividual* Individual)
{
    // Get the class name
    const FString ClassVal = Individual->GetClassValue();

    // Make sure the class was not defined before
    for (const auto& ClassNode : Doc.ClassDefinitions)
    {
        for (const auto& ClassAttr : ClassNode.Attributes)
        {
            if (ClassAttr.Key.Prefix.Equals("rdf") &&
                ClassAttr.Key.LocalName.Equals("about") &&
                ClassAttr.Value.LocalValue.Equals(ClassVal))
            {
                return;
            }
        }
    }

    // Create the class node
    FSLOwlNode ClassNode(OwlClass, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(AmevaNs, ClassVal)));
    ClassNode.SetComment(TEXT("Class ") + ClassVal);

    // Add class definition properties (if any)
    AddClassDefinitionProperties(ClassNode, Individual);

    // Add class node to the document
    Doc.AddClassDefinition(ClassNode);
}

// Add class definition properties
void FSLOwlOntologyDocUtils::AddClassDefinitionProperties(FSLOwlNode& ClassNode, USLBaseIndividual* Individual)
{
    AActor* ParentActor = Individual->GetParentActor();
    if(ParentActor == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("%s::%d individual %s's owner actor is not set, this should not happen.."),
            *FString(__FUNCTION__), __LINE__, *Individual->GetFullName());
        return;
    }

    /* BB properties */
    FVector BBSize;
#if SL_WITH_ROS_CONVERSIONS
    BBSize = FConversions::CmToM(ParentActor->GetComponentsBoundingBox().GetSize());
#else
    BBSize = ParentActor->GetComponentsBoundingBox().GetSize();
#endif // SL_WITH_ROS_CONVERSIONS
    if (!BBSize.IsNearlyZero())
    {
        AddBBClassDefinitionProperties(ClassNode, BBSize);
    }

    /* Constraint properties */

    /* Static mesh properties */
}

// Add bounding box properties to the class node defintion
void FSLOwlOntologyDocUtils::AddBBClassDefinitionProperties(FSLOwlNode& ClassNode, FVector BBSize)
{   
    /**
        <rdfs:subClassOf>
			<owl:Restriction>
				<owl:onProperty rdf:resource="&knowrob;depthOfObject"/>
				<owl:hasValue rdf:datatype="&xsd;float">0.89</owl:hasValue>
			</owl:Restriction>
		</rdfs:subClassOf>
    **/

    /* Depth X */
    FSLOwlNode SubClassOfNodeD(RdfsSubClassOf);
    FSLOwlNode RestrictionNodeD(OwlRestriction);
    RestrictionNodeD.AddChildNode(FSLOwlNode(OwlOnProperty,
        FSLOwlAttribute(RdfResource, 
            FSLOwlAttributeValue(KRNs, "depthOfObject"))));
    RestrictionNodeD.AddChildNode(FSLOwlNode(OwlHasValue,
        FSLOwlAttribute(RdfDatatype, AVFloat), FString::SanitizeFloat(BBSize.X)));
    SubClassOfNodeD.AddChildNode(RestrictionNodeD);
    ClassNode.AddChildNode(SubClassOfNodeD);

    /* Width Y */
    FSLOwlNode SubClassOfNodeW(RdfsSubClassOf);
    FSLOwlNode RestrictionNodeW(OwlRestriction);
    RestrictionNodeW.AddChildNode(FSLOwlNode(OwlOnProperty,
        FSLOwlAttribute(RdfResource,
            FSLOwlAttributeValue(KRNs, "widthOfObject"))));
    RestrictionNodeW.AddChildNode(FSLOwlNode(OwlHasValue,
        FSLOwlAttribute(RdfDatatype, AVFloat), FString::SanitizeFloat(BBSize.Y)));
    SubClassOfNodeW.AddChildNode(RestrictionNodeW);
    ClassNode.AddChildNode(SubClassOfNodeW);

    /* Height Z */
    FSLOwlNode SubClassOfNodeH(RdfsSubClassOf);
    FSLOwlNode RestrictionNodeH(OwlRestriction);
    RestrictionNodeH.AddChildNode(FSLOwlNode(OwlOnProperty,
        FSLOwlAttribute(RdfResource,
            FSLOwlAttributeValue(KRNs, "heightOfObject"))));
    RestrictionNodeH.AddChildNode(FSLOwlNode(OwlHasValue,
        FSLOwlAttribute(RdfDatatype, AVFloat), FString::SanitizeFloat(BBSize.Z)));
    SubClassOfNodeH.AddChildNode(RestrictionNodeH);
    ClassNode.AddChildNode(SubClassOfNodeH);
}
