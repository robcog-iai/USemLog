#pragma once
// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
* Structure holding the semantic data of an entity
*/
struct FSLEntity
{
	// UObject of entity
	UObject* Obj;

	// Semantic id of entity
	FString Id;

	// Semantic class of entity
	FString Class;

	// Default constructor
	FSLEntity() {};

	// Init constructor
	FSLEntity(UObject* InObj, const FString& InId, const FString& InClass) :
		Obj(InObj), Id(InId), Class(InClass) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsValid() const { return Obj != nullptr && !Id.IsEmpty() && !Class.IsEmpty(); }

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("UniqueID:%ld Id:%s Class:%s"), Obj->GetUniqueID(), *Id, *Class);
	}
};

/**
* Structure holding the semantic data of two entities
*/
struct FSLEntityPair
{
	// First entity
	FSLEntity Entity1;

	// Second entity
	FSLEntity Entity2;

	// Default constructor
	FSLEntityPair() {};

	// Init constructor
	FSLEntityPair(const FSLEntity& InEntity1, const FSLEntity& InEntity2) : Entity1(InEntity1), Entity2(InEntity2) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsValid() const { return Entity1.IsValid() && Entity2.IsValid(); }

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("%s %s"), *Entity1.ToString(), *Entity2.ToString());
	}
};


/**
* Templated data structure of entities with semantic and previous transform information
*/
// TODO remove the Obj pointer, and use the one from entity, add a IsValid test to make sure the Obj can have a location
template <typename T>
struct TSLEntityPreviousPose
{
	// Pointer to the actor/component/skeletal actor, the object should have a transform
	TWeakObjectPtr<T> Obj;

	// The semantically annotated entity
	FSLEntity Entity;

	// Its previous location
	FVector PrevLoc;

	// Its previous rotation
	FQuat PrevQuat;

	// Default constructor
	TSLEntityPreviousPose() {};

	// Init constructor
	TSLEntityPreviousPose(TWeakObjectPtr<T> InObj,
		const FSLEntity& InEntity,
		FVector InPrevLoc = FVector(BIG_NUMBER),
		FQuat InPrevQuat = FQuat::Identity) :
		Obj(InObj),
		Entity(InEntity),
		PrevLoc(InPrevLoc),
		PrevQuat(InPrevQuat)
	{};

	// Check if the entity is valid and has a transform
	bool IsValid() const { return Entity.IsValid() && (Cast<USceneComponent>(Entity.Obj) || Cast<AActor>(Entity.Obj)); }
};

