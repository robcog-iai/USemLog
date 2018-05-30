// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "OwlStatics.h"

/* Owl individuals creation */
// Create an object entry
FOwlNode FOwlStatics::CreateObjectIndividual(const FString& Id, const FString& Class)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ObjectIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue("log", Id)));
	ObjectIndividual.Comment = TEXT("Object " + Class + " " + Id);
	ObjectIndividual.ChildNodes.Add(FOwlStatics::CreateClassProperty(Class));
	return ObjectIndividual;
}

// Create a pose entry
FOwlNode FOwlStatics::CreatePoseIndividual(const FString& InId, const FVector& InLoc, const FQuat& InQuat)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName RdfType("rdf", "type");
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	// Attribute values constants
	const FOwlAttributeValue AttrValPose("knowrob", "Pose");
	const FOwlAttributeValue AttrValString("xsd", "string");

	// Pose individual
	FOwlNode PoseIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue("log", InId)));
	FOwlNode PoseProperty(RdfType, FOwlAttribute(RdfResource, AttrValPose));
	PoseIndividual.ChildNodes.Add(PoseProperty);
	PoseIndividual.ChildNodes.Add(FOwlStatics::CreateQuaternionProperty(InQuat));
	PoseIndividual.ChildNodes.Add(FOwlStatics::CreateLocationProperty(InLoc));
	return PoseIndividual;
}

// Create a constraint individual
FOwlNode FOwlStatics::CreateConstraintIndividual(const FString& InId,
	const FString& ParentId,
	const FString& ChildId)
{
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ObjectIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue("log", InId)));
	ObjectIndividual.Comment = TEXT("Constraint " + InId);
	ObjectIndividual.ChildNodes.Add(FOwlStatics::CreateClassProperty("Constraint"));
	ObjectIndividual.ChildNodes.Add(FOwlStatics::CreateParentProperty(ParentId));
	ObjectIndividual.ChildNodes.Add(FOwlStatics::CreateChildProperty(ChildId));

	return ObjectIndividual;
}

// Create constraint linear limits individual
FOwlNode FOwlStatics::CreateLinearConstraintProperties(const FString& InId,
	uint8 XMotion,
	uint8 YMotion,
	uint8 ZMotion,
	float Limit,
	bool bSoftConsraint,
	float Stiffness,
	float Damping)
{
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ConstrPropIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue("log", InId)));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateClassProperty("LinearConstraint"));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "xMotion"), XMotion));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "yMotion"), YMotion));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "zMotion"), ZMotion));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "limit"), Limit));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateBoolValueProperty(
		FOwlPrefixName("knowrob", "softConstraint"), bSoftConsraint));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "stiffness"), Stiffness));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "damping"), Damping));

	return ConstrPropIndividual;
}

// Create constraint angular limits individual
FOwlNode FOwlStatics::CreateAngularConstraintProperties(const FString& InId,
	uint8 Swing1Motion,
	uint8 Swing2Motion,
	uint8 TwistMotion,
	float Swing1Limit,
	float Swing2Limit,
	float TwistLimit,
	bool bSoftSwingConstraint,
	float SwingStiffness,
	float SwingDamping,
	bool bSoftTwistConstraint,
	float TwistStiffness,
	float TwistDamping)
{
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ConstrPropIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue("log", InId)));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateClassProperty("AngularConstraint"));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob","swing1Motion"), Swing1Motion));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "swing2Motion"), Swing2Motion));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "twistMotion"), TwistMotion));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swing1Limit"), Swing1Limit));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swing2Limit"), Swing2Limit));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "twistLimit"), TwistLimit));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateBoolValueProperty(
		FOwlPrefixName("knowrob", "softSwingConstraint"), bSoftSwingConstraint));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swingStiffness"), SwingStiffness));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swingDamping"), SwingDamping));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateBoolValueProperty(
		FOwlPrefixName("knowrob", "softTwistConstraint"), bSoftTwistConstraint));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "twistStiffness"), TwistStiffness));
	ConstrPropIndividual.ChildNodes.Add(FOwlStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "twistDamping"), TwistDamping));

	return ConstrPropIndividual;
}

// Create a class individual
FOwlNode FOwlStatics::CreateClassDefinition(const FString& InClass)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlClass("owl", "Class");

	return FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", InClass)));
}

/* Owl properties creation */
// Create property
FOwlNode FOwlStatics::CreateGenericProperty(const FOwlPrefixName& InPrefixName,
	const FOwlAttributeValue& InAttributeValue)
{
	return FOwlNode();
}

// Create class property
FOwlNode FOwlStatics::CreateClassProperty(const FString& InClass)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfType("rdf", "type");

	return FOwlNode(RdfType, FOwlAttribute(RdfResource, FOwlAttributeValue("knowrob", InClass)));
}

// Create subClassOf property
FOwlNode FOwlStatics::CreateSubClassOfProperty(const FString& InSubClass)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfsSubClassOf("rdfs", "subClassOf");

	return FOwlNode(RdfsSubClassOf, FOwlAttribute(RdfResource, FOwlAttributeValue("knowrob", InSubClass)));
}

// Create skeletal bone property
FOwlNode FOwlStatics::CreateSkeletalBoneProperty(const FString& InBone)
{
	const FOwlPrefixName KbSkelBone("knowrob", "skeletalBone");
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValString("xsd", "string");

	return FOwlNode(KbSkelBone, FOwlAttribute(RdfDatatype, AttrValString), InBone);
}

// Create subclass - depth property
FOwlNode FOwlStatics::CreateDepthProperty(float Value) 
{
	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FOwlPrefixName OwlRestriction("owl", "Restriction");
	const FOwlPrefixName OwlHasVal("owl", "hasValue");

	FOwlNode SubClass(RdfsSubClass);
	FOwlNode Restriction(OwlRestriction);
	Restriction.ChildNodes.Add(CreateOnProperty("depthOfObject"));
	Restriction.ChildNodes.Add(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.ChildNodes.Add(Restriction);
	return SubClass;
}

// Create subclass - height property
FOwlNode FOwlStatics::CreateHeightProperty(float Value) 
{
	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FOwlPrefixName OwlRestriction("owl", "Restriction");
	const FOwlPrefixName OwlHasVal("owl", "hasValue");

	FOwlNode SubClass(RdfsSubClass);
	FOwlNode Restriction(OwlRestriction);
	Restriction.ChildNodes.Add(CreateOnProperty("heightOfObject"));
	Restriction.ChildNodes.Add(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.ChildNodes.Add(Restriction);
	return SubClass;
}

// Create subclass - width property
FOwlNode FOwlStatics::CreateWidthProperty(float Value) 
{
	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FOwlPrefixName OwlRestriction("owl", "Restriction");
	const FOwlPrefixName OwlHasVal("owl", "hasValue");

	FOwlNode SubClass(RdfsSubClass);
	FOwlNode Restriction(OwlRestriction);
	Restriction.ChildNodes.Add(CreateOnProperty("widthOfObject"));
	Restriction.ChildNodes.Add(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.ChildNodes.Add(Restriction);
	return SubClass;
}

// Create owl:onProperty meta property
FOwlNode FOwlStatics::CreateOnProperty(const FString& InProperty)
{
	const FOwlPrefixName OwlOnProp("owl", "onProperty");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(OwlOnProp, FOwlAttribute(RdfResource,FOwlAttributeValue("knowrob", InProperty)));
}

// Create a property with a bool value
FOwlNode FOwlStatics::CreateBoolValueProperty(const FOwlPrefixName& InPrefixName, bool bValue)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValBool("xsd", "boolean");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValBool), bValue ? "true" : "false");
}

// Create a property with an integer value
FOwlNode FOwlStatics::CreateIntValueProperty(const FOwlPrefixName& InPrefixName, int32 Value)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValInt("xsd", "integer");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValInt), FString::FromInt(Value));
}

// Create a property with a float value
FOwlNode FOwlStatics::CreateFloatValueProperty(const FOwlPrefixName& InPrefixName, float Value)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValFloat("xsd", "float");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValFloat), FString::SanitizeFloat(Value));
}

// Create a property with a string value
FOwlNode FOwlStatics::CreateStringValueProperty(const FOwlPrefixName& InPrefixName, const FString& InValue)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValString("xsd", "string");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValString), InValue);
}

// Create pose property
FOwlNode FOwlStatics::CreatePoseProperty(const FString& InId)
{
	const FOwlPrefixName KbPose("knowrob", "pose");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbPose, FOwlAttribute(RdfResource, FOwlAttributeValue("log", InId)));
}

// Create linear constraint property
FOwlNode FOwlStatics::CreateLinearConstraintProperty(const FString& InId)
{
	const FOwlPrefixName KbLinearConstr("knowrob", "linearConstraint");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbLinearConstr, FOwlAttribute(RdfResource, FOwlAttributeValue("log", InId)));
}

// Create angular constraint property
FOwlNode FOwlStatics::CreateAngularConstraintProperty(const FString& InId)
{
	const FOwlPrefixName KbAngularConstr("knowrob", "angularConstraint");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbAngularConstr, FOwlAttribute(RdfResource, FOwlAttributeValue("log", InId)));
}

// Create child property
FOwlNode FOwlStatics::CreateChildProperty(const FString& InId)
{
	const FOwlPrefixName KbChild("knowrob", "child");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbChild, FOwlAttribute(RdfResource, FOwlAttributeValue("log", InId)));
}

// Create parent property
FOwlNode FOwlStatics::CreateParentProperty(const FString& InId)
{
	const FOwlPrefixName KbParent("knowrob", "parent");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbParent, FOwlAttribute(RdfResource, FOwlAttributeValue("log", InId)));
}

// Create a location node
FOwlNode FOwlStatics::CreateLocationProperty(const FVector& InLoc)
{
	const FOwlPrefixName KbTransl("knowrob", "translation");
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValString("xsd", "string");

	const FString LocStr = FString::Printf(TEXT("%f %f %f"),
		InLoc.X, InLoc.Y, InLoc.Z);
	return FOwlNode(KbTransl, FOwlAttribute(RdfDatatype, AttrValString), LocStr);
}

// Create a quaternion node
FOwlNode FOwlStatics::CreateQuaternionProperty(const FQuat& InQuat)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlPrefixName KbQuat("knowrob", "quaternion");
	const FOwlAttributeValue AttrValString("xsd", "string");
	
	const FString QuatStr = FString::Printf(TEXT("%f %f %f %f"),
		InQuat.W, InQuat.X, InQuat.Y, InQuat.Z);
	return FOwlNode(KbQuat, FOwlAttribute(RdfDatatype, AttrValString), QuatStr);
}
