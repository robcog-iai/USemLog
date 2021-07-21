// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualInfoComponent.h"
#include "Individuals/SLIndividualInfoTextComponent.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/KismetMathLibrary.h" // FindLookAtRotation
#include "Engine/LocalPlayer.h" // Frustrum check
#include "TimerManager.h"

#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif //WITH_EDITOR


// Sets default values for this component's properties
USLIndividualInfoComponent::USLIndividualInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.1f;
	bTickInEditor = true;

	bIsInit = false;
	bIsLoaded = false;
	bIsConnected = false;
	IndividualComponent = nullptr;
	TextComponent = nullptr;
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config. IsTemplate() could be true
void USLIndividualInfoComponent::PostInitProperties()
{
	Super::PostInitProperties();
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Do any object-specific cleanup required immediately after loading an object.
void USLIndividualInfoComponent::PostLoad()
{
	Super::PostLoad();
	
	// Make sure the individual sibling is connected before
	FTimerHandle DelayTimerHandle;
	FTimerDelegate DelayTimerDelegate;
	DelayTimerDelegate.BindLambda([this] 
		{
			if (!IsConnected())
			{
				Connect();
				//SetOwnTextInfo();
				//SetIndividualComponentTextInfo();
			}
		});
	if (GetWorld())
	{		
		GetWorld()->GetTimerManager().SetTimer(DelayTimerHandle, DelayTimerDelegate, 0.5f, false);
	}
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualInfoComponent::OnRegister()
{
	Super::OnRegister();
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Initializes the component.
void USLIndividualInfoComponent::InitializeComponent()
{
	Super::InitializeComponent();
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualInfoComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	/*UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());*/

	// Check if actor already has a semantic data component
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(USLIndividualComponent::StaticClass(), Components);
	for (const auto AC : Components)
	{
		if (AC != this)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has an individual info component (%s), self-destruction commenced.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
			//DestroyComponent();
			ConditionalBeginDestroy();
			return;
		}
	}
#else
	for (const auto AC : GetOwner()->GetComponentsByClass(USLIndividualInfoComponent::StaticClass()))
	{
		if (AC != this)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has an individual info component (%s), self-destruction commenced.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
			//DestroyComponent();
			ConditionalBeginDestroy();
			return;
		}
	}
#endif
}

// Called when the game starts
void USLIndividualInfoComponent::BeginPlay()
{
	Super::BeginPlay();	
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Called every frame
void USLIndividualInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	OrientateTowardsViewer();
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
	//FVector ViewLocation;
	//IsInViewFrustrum(ViewLocation);
	//OrientateTowardsLocation(ViewLocation);
	//SetTextScale(ViewLocation);

	//if (IsInViewFrustrum(ViewLocation))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's Tick at %.4fs, In FRUSTRUM "),
	//		*FString(__FUNCTION__), __LINE__, *GetFullGroupName(false), FPlatformTime::Seconds());
	//	OrientateTowardsLocation(ViewLocation);
	//	SetTextScale(ViewLocation);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d %s's Tick at %.4fs, NOT In FRUSTRUM "),
	//		*FString(__FUNCTION__), __LINE__, *GetFullGroupName(true), FPlatformTime::Seconds());
	//	//OrientateTowardViewer();
	//}

	//if (GetOwner()->WasRecentlyRendered())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s WasRecentlyRendered"), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Log, TEXT("%s::%d NOT %s WasRecentlyRendered"), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	//}
}

// Called before destroying the object.
void USLIndividualInfoComponent::BeginDestroy()
{
	//SetIsInit(false); 
	//SetIsLoaded(false);
	OnDestroyed.Broadcast(this);
	if(HasValidTextComponent())
	{
		ClearTextComponent();
	}
	ClearChildrenTextComponents();
	Super::BeginDestroy();
}

// Connect to owner individual component
bool USLIndividualInfoComponent::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl());
	return IsInit();
}

// Refresh values from parent (returns false if component not init)
bool USLIndividualInfoComponent::Load(bool bReset)
{
	if (bReset)
	{
		LoadReset();
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			return false;
		}
	}

	SetIsLoaded(LoadImpl());
	return IsLoaded();
}

// Listen to the individual object delegates
bool USLIndividualInfoComponent::Connect()
{
	if (IsConnected())
	{
		return true;
	}
	SetIsConnected(BindDelegates());
	return IsConnected();
}

// Enable/disable tick
void USLIndividualInfoComponent::ToggleTick()
{
	SetComponentTickEnabled(!IsComponentTickEnabled());
}

// Set text info visiblity
void USLIndividualInfoComponent::SetTextVisibility(bool bNewVisiblitity)
{
	if (HasValidTextComponent())
	{
		TextComponent->SetVisibility(bNewVisiblitity, true);
	}
	for (const auto& ChildKV : ChildrenTextComponents)
	{
		ChildKV.Value->SetVisibility(bNewVisiblitity, true);
	}
}

//// Show / hide text data
//void USLIndividualInfoComponent::ToggleTextVisibility()
//{
//	if (HasValidTextComponent())
//	{
//		TextComponent->SetVisibility(!TextComponent->IsVisible(), true);
//	}
//	for (const auto& ChildKV : ChildrenTextComponents)
//	{
//		ChildKV.Value->SetVisibility(!ChildKV.Value->IsVisible(), true);
//	}
//}

// Rotate component towards the screen
bool USLIndividualInfoComponent::OrientateTowardsViewer()
{
	// True if we are in the editor (this is still true when using Play In Editor). You may want to use GWorld->HasBegunPlay in that case)	
	if (GIsEditor)
	{
		// TODO check if standalone e.g. 
		//if (GIsPlayInEditorWorld) 

#if WITH_EDITOR
		for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
		{
			if (LevelVC && LevelVC->IsPerspective())
			{
				const FRotator ToViewerRotator = LevelVC->GetViewRotation() + FRotator(180.f, 0.f, 180.f);
				if (HasValidTextComponent())
				{
					TextComponent->SetWorldRotation(ToViewerRotator);
				}
				for (const auto& ChildKV : ChildrenTextComponents)
				{
					ChildKV.Value->SetWorldRotation(ToViewerRotator);
				}
				break;
			}
		}
#endif //WITH_EDITOR
	}
	else if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		//PC->PlayerCameraManager; // This will not call or yield anything
	}

	return false;
}

// Rotate component towards the given location
void USLIndividualInfoComponent::OrientateTowardsLocation(const FVector& Location)
{
	const FVector TowardsDirection = (Location - GetComponentLocation()).GetSafeNormal();
	const FQuat TowardsRotation = TowardsDirection.ToOrientationQuat();
	SetWorldRotation((TowardsRotation * FVector::ForwardVector).ToOrientationQuat());
}

// Clear any bound delegates (called when init is reset)
void USLIndividualInfoComponent::ClearDelegates()
{
	OnDestroyed.Clear();
	OnDelegatesCleared.Broadcast(this);
	OnDelegatesCleared.Clear();
}

// Set the init flag, return true if the state change
void USLIndividualInfoComponent::SetIsInit(bool bNewValue, bool bBroadcast)
{
	if (bIsInit != bNewValue)
	{
		if (!bNewValue)
		{
			SetIsLoaded(false);
		}

		bIsInit = bNewValue;
		if (bBroadcast)
		{
			// OnInitChanged.Broadcast(this, bNewValue);
		}
		//SetOwnTextInfo();
	}
}

// Set the loaded flag
void USLIndividualInfoComponent::SetIsLoaded(bool bNewValue, bool bBroadcast)
{
	if (bIsLoaded != bNewValue)
	{
		bIsLoaded = bNewValue;
		if (bBroadcast)
		{
			// OnLoadedChanged.Broadcast(this, bNewValue);
		}
		//SetOwnTextInfo();
	}
}

// Set the connected flag, broadcast on new value
void USLIndividualInfoComponent::SetIsConnected(bool bNewValue, bool bBroadcast)
{
	if (bIsConnected != bNewValue)
	{
		bIsConnected = bNewValue;
		if (bBroadcast)
		{
			// OnConnectedChanged.Broadcast(this, bNewValue);
		}
		//SetOwnTextInfo();
	}
}

// Private init implementation
bool USLIndividualInfoComponent::InitImpl()
{
	if (HasValidIndividualComponent() || SetIndividualComponent())
	{
		if (HasValidTextComponent() || SetTextComponent())
		{
			//SetOwnTextInfo();
			//SetIndividualComponentTextInfo();

			// Set children if any
			SetChildrenTextComponents();

			// Listen to the individual component changes
			Connect();
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not acces the individual component sibling, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Private load implementation
bool USLIndividualInfoComponent::LoadImpl()
{
	if (!HasValidIndividualComponent())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s's individual component is not set.. this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	if (!HasValidTextComponent())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s's text component is not set.. this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	
	//SetOwnTextInfo();
	//SetIndividualComponentTextInfo();

	// Force publishing individual component values
	return IndividualComponent->TriggerIndividualValuesBroadcast();
}

// Clear all references of the individual
void USLIndividualInfoComponent::InitReset()
{
	SetIsConnected(false);
	LoadReset();
	SetIsInit(false);
	ClearChildrenTextComponents();
	IndividualComponent = nullptr;
	ClearDelegates();
}

// Clear all data of the individual
void USLIndividualInfoComponent::LoadReset()
{
	//TSet<FString> IgnoreKeys;
	//IgnoreKeys.Add(FString(SelfTextRowKey));
	//IgnoreKeys.Add(FString(ICTextRowKey));
	//TextComponent->RemoveAllTextRowsBut(IgnoreKeys);
	if (HasValidTextComponent())
	{
		TextComponent->ClearAllRows();
	}
	ClearChildrenTextComponentsData();
	SetIsLoaded(false);
}

// Update info as soon as the individual changes their data
bool USLIndividualInfoComponent::BindDelegates()
{
	bool bRetVal = true;
	if (!HasValidIndividualComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not valid, cannot bind delegates.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	/* Init */
	if (!IndividualComponent->OnInitChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentInitChanged))
	{
		IndividualComponent->OnInitChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentInitChanged);			
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's init changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Load */
	if (!IndividualComponent->OnLoadedChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentLoadedChanged))
	{
		IndividualComponent->OnLoadedChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentLoadedChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's loaded changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Value Changed */
	if (!IndividualComponent->OnValueChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentValueChanged))
	{
		IndividualComponent->OnValueChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentValueChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's on value changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Child Value Changed */
	if (!IndividualComponent->OnChildValueChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentChildValueChanged))
	{
		IndividualComponent->OnChildValueChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentChildValueChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's on child value changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Child Num has changed*/
	if (!IndividualComponent->OnChildrenNumChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentChildrenNumChanged))
	{
		IndividualComponent->OnChildrenNumChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentChildrenNumChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's on children num changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Destroyed */
	if (!IndividualComponent->OnDestroyed.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentDestroyed))
	{
		IndividualComponent->OnDestroyed.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's destroyed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Delegates cleared */
	if (!IndividualComponent->OnDelegatesCleared.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnIndividualComponentDelegatesCleared))
	{
		IndividualComponent->OnDelegatesCleared.AddDynamic(
			this, &USLIndividualInfoComponent::OnIndividualComponentDelegatesCleared);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's delegates cleared delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return true;
}

// Check if individual component is set
bool USLIndividualInfoComponent::HasValidIndividualComponent() const
{
	return IndividualComponent && IndividualComponent->IsValidLowLevel() && !IndividualComponent->IsPendingKill();
}

// Set the individual component
bool USLIndividualInfoComponent::SetIndividualComponent()
{
	if (HasValidIndividualComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component is already set, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	// Check if the owner has an individual component
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		IndividualComponent = CastChecked<USLIndividualComponent>(AC);
		return true;
	}
	return false;
}

// Set individual text component
bool USLIndividualInfoComponent::SetTextComponent()
{
	if (HasValidTextComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's already has an individual text component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (!HasValidIndividualComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's has no valid individual component, cannot create a text component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	TextComponent = NewObject<USLIndividualInfoTextComponent>(this);
	TextComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	return true;
}

// Destroy the individual text component
void USLIndividualInfoComponent::ClearTextComponent()
{
	if (HasValidTextComponent())
	{
		TextComponent->ConditionalBeginDestroy();
	}
	TextComponent = nullptr;
}

// Check if the individual text component is valid
bool USLIndividualInfoComponent::HasValidTextComponent()
{
	return TextComponent && TextComponent->IsValidLowLevel() && !TextComponent->IsPendingKill();
}

// Set text components for the children
bool USLIndividualInfoComponent::SetChildrenTextComponents()
{
	if (!HasValidIndividualComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's has no valid individual component, cannot check for children.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());

		if(HasChildrenTextComponents())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's should not have children without the individual set, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			ClearChildrenTextComponents();
		}
		return false;
	}

	if (HasChildrenTextComponents())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's children text components are already set, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	for (const auto KeyVal : IndividualComponent->GetAttachableIndividualChildren())
	{
		USLIndividualInfoTextComponent* IITC = NewObject<USLIndividualInfoTextComponent>(this);		
		IITC->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform, KeyVal.Value);
		ChildrenTextComponents.Add(KeyVal.Key, IITC);
	}
	return true;
}

// Destroy the children text components
void USLIndividualInfoComponent::ClearChildrenTextComponents()
{
	for (const auto& KeyVal : ChildrenTextComponents)
	{
		KeyVal.Value->ConditionalBeginDestroy();
	}
	ChildrenTextComponents.Empty();
}

// Remove all rows from the children text
void USLIndividualInfoComponent::ClearChildrenTextComponentsData()
{
	for (const auto& KeyVal : ChildrenTextComponents)
	{
		KeyVal.Value->ClearAllRows();
	}
}

// Check if the children are in sync with the individual components
bool USLIndividualInfoComponent::HasChildrenInSync() const
{
	if (HasValidIndividualComponent())
	{
		return IndividualComponent->GetAttachableIndividualChildren().Num() == ChildrenTextComponents.Num();
	}
	else
	{
		if (HasChildrenTextComponents())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no valid individual component and has children, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());			
		}
	}
	// no individual component, cannot say if in sync
	return false;
}

// Check if the component is in the view frustrum
bool USLIndividualInfoComponent::IsInViewFrustrum(FVector& OutViewLocation) const
{
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			GetWorld()->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);
		OutViewLocation = ViewLocation;
		if (SceneView != nullptr)
		{
			return SceneView->ViewFrustum.IntersectSphere(
				GetOwner()->GetActorLocation(), GetOwner()->GetSimpleCollisionRadius());
		}
	}
	return false;
}

// Scale the text relative to the distance towards it
void USLIndividualInfoComponent::SetTextScale(const FVector& Location)
{
	float Distance = FVector::Distance(Location, GetComponentLocation());
	float DistanceSq = FVector::DistSquared(Location, GetComponentLocation());
	UE_LOG(LogTemp, Log, TEXT("%s::%d Distance=%f; DistanceSq=%f;"),
		*FString(__FUNCTION__), __LINE__, Distance, DistanceSq);
	
	/*float Scale = FMath::Clamp(Distance, )
	TextComponent->SetWorldScale3D()*/
	//MiddleLocation.Z += Scale * 5.0f;
	//UserScaleIndicatorText->SetWorldTransform(FTransform((VRMode->GetHeadTransform().GetLocation() - MiddleLocation).ToOrientationRotator(),
	//	MiddleLocation,
	//	VRMode->GetRoomSpaceHeadTransform().GetScale3D() * Scale
	//));
}

// Set its own states as text values
void USLIndividualInfoComponent::SetOwnTextInfo()
{	
	if (!HasValidTextComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's has no valid individual text component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return;
	}

	const FString TextValue = FString::Printf(TEXT("%s : I:%s; L:%s; C:%s; ChNum:%ld;"),
		OwnTextInfoKey, 
		IsInit() ? "T" : "F", 
		IsLoaded() ? "T" : "F", 
		IsConnected() ? "T" : "F",
		ChildrenTextComponents.Num());
	TextComponent->SetRowValue(OwnTextInfoKey, TextValue);
	if (IsConnected())
	{
		if (IsLoaded())
		{
			TextComponent->SetRowColor(OwnTextInfoKey, FColor::Green);
		}
		else if (IsInit())
		{
			TextComponent->SetRowColor(OwnTextInfoKey, FColor::Yellow);
		}
		else
		{
			TextComponent->SetRowColor(OwnTextInfoKey, FColor::Red);
		}
	}
	else
	{
		TextComponent->SetRowColor(OwnTextInfoKey, FColor::Silver);
	}
}

// Set its individuals state values
void USLIndividualInfoComponent::SetIndividualComponentTextInfo()
{		
	if (!HasValidTextComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's has no valid individual text component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return;
	}

	if (HasValidIndividualComponent())
	{
		const FString TextValue = FString::Printf(TEXT("%s : I:%s; L:%s; C:%s; ChNum:%ld;"),
			*FString(ICTextInfoKey), 
			IndividualComponent->IsInit() ? "T" : "F", 
			IndividualComponent->IsLoaded() ? "T" : "F", 
			IndividualComponent->IsConnected() ? "T" : "F",
			IndividualComponent->GetIndividualChildren().Num());
		TextComponent->SetRowValue(ICTextInfoKey, TextValue);
		if (IndividualComponent->IsConnected())
		{
			if (IndividualComponent->IsLoaded())
			{
				TextComponent->SetRowColor(ICTextInfoKey, FColor::Green);
			}
			else if (IndividualComponent->IsInit())
			{
				TextComponent->SetRowColor(ICTextInfoKey, FColor::Yellow);
			}
			else
			{
				TextComponent->SetRowColor(ICTextInfoKey, FColor::Red);
			}
		}
		else
		{
			TextComponent->SetRowColor(ICTextInfoKey, FColor::Silver);
		}
	}
	else
	{
		TextComponent->SetRowValueAndColor(ICTextInfoKey,
			FString::Printf(TEXT("%s : null"), ICTextInfoKey), FColor::White);
	}
}


/* Delegate functions */
// Called when owners init value has changed
void USLIndividualInfoComponent::OnIndividualComponentInitChanged(USLIndividualComponent* IC, bool bNewVal)
{
	//SetIndividualComponentTextInfo();
}

// Called when owners load value has changed
void USLIndividualInfoComponent::OnIndividualComponentLoadedChanged(USLIndividualComponent* IC, bool bNewVal)
{
	//SetOwnTextInfo();
}

// Called when the individual component connected value has changed
void USLIndividualInfoComponent::OnIndividualComponentConnectedChanged(USLIndividualComponent* IC, bool bNewVal)
{
	//SetIndividualComponentTextInfo();
}

// Called when the individual component values have changed
void USLIndividualInfoComponent::OnIndividualComponentValueChanged(USLIndividualComponent* IC, const FString& Key, const FString& NewValue)
{
	if (!HasValidTextComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's has no valid individual text component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return;
	}
	if (NewValue.IsEmpty())
	{
		TextComponent->SetRowValueAndColor(Key, "null", FColor::Silver);
	}
	else
	{
		TextComponent->SetRowValueAndColor(Key, NewValue, FColor::Silver);
	}
}

// Called when the indiviual component child values have changed
void USLIndividualInfoComponent::OnIndividualComponentChildValueChanged(USLIndividualComponent* IC, USLBaseIndividual* ConstraintIndividual1, const FString& Key, const FString& NewValue)
{
	if (USLIndividualInfoTextComponent** IITC = ChildrenTextComponents.Find(ConstraintIndividual1))
	{		
		if (NewValue.IsEmpty())
		{
			(*IITC)->SetRowValueAndColor(Key, "null", FColor::Silver);
		}
		else
		{
			(*IITC)->SetRowValueAndColor(Key, NewValue, FColor::Silver);
		}
	}
}

// Called when the children number has changed
void USLIndividualInfoComponent::OnIndividualComponentChildrenNumChanged(USLIndividualComponent* IC, int32 NumChildren, int32 NumAttachableChildren)
{
	if (NumAttachableChildren != ChildrenTextComponents.Num())
	{
		ClearChildrenTextComponents();
		SetChildrenTextComponents();
	}
}

// Called when owner is being destroyed
void USLIndividualInfoComponent::OnIndividualComponentDestroyed(USLIndividualComponent* IC)
{
	// Trigger self destruct
	ConditionalBeginDestroy();
}

// Called when individual component delegates are cleared
void USLIndividualInfoComponent::OnIndividualComponentDelegatesCleared(USLIndividualComponent* IC)
{
	SetIsConnected(false);
}

