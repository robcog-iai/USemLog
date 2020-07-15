// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualInfoComponent.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualTextComponent.h"
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
	SiblingIndividualComponent = nullptr;

	TextComponent = CreateDefaultSubobject<USLIndividualTextComponent>("SLIndividualText");
	TextComponent->SetupAttachment(this);
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config. IsTemplate() could be true
void USLIndividualInfoComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Do any object-specific cleanup required immediately after loading an object.
void USLIndividualInfoComponent::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
	
	// Make sure the individual sibling is connected before
	FTimerHandle DelayTimerHandle;
	FTimerDelegate DelayTimerDelegate;
	DelayTimerDelegate.BindLambda([this] 
		{
			if (!IsConnected())
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
				Connect();
				SetOwnStateValuesText();
				SetSiblingIndividualStateValuesText();
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
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Initializes the component.
void USLIndividualInfoComponent::InitializeComponent()
{
	Super::InitializeComponent();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualInfoComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%s::%.4fs"), *FString(__FUNCTION__), __LINE__, *GetFullName(), FPlatformTime::Seconds());

	// Check if actor already has a semantic data component
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
	SetIsInit(false);
	SetIsLoaded(false);
	OnDestroyed.Broadcast(this);

	//if (TextComponent && TextComponent->IsValidLowLevel() && !TextComponent->IsPendingKill())
	TextComponent->ConditionalBeginDestroy();
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

// Show / hide text data
void USLIndividualInfoComponent::ToggleTextVisibility()
{
	TextComponent->SetVisibility(!TextComponent->IsVisible(), true);
}

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
				SetWorldRotation(LevelVC->GetViewRotation() + FRotator(180.f, 0.f, 180.f));
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

// Clear all references of the individual
void USLIndividualInfoComponent::InitReset()
{
	LoadReset();
	SetIsInit(false);
	TextComponent->ClearAllTextLines();
	SiblingIndividualComponent = nullptr;
	ClearDelegates();
}

// Clear all data of the individual
void USLIndividualInfoComponent::LoadReset()
{
	TArray<FString> IgnoreKeys;
	IgnoreKeys.Add(FString(SelfTextLineKey));
	IgnoreKeys.Add(FString(SiblingIndividualTextLineKey));
	TextComponent->ClearTextLineValues(IgnoreKeys);
	SetIsLoaded(false);
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
		SetOwnStateValuesText();
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
		SetOwnStateValuesText();
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
		SetOwnStateValuesText();
	}
}

// Private init implementation
bool USLIndividualInfoComponent::InitImpl()
{
	if (HasValidSiblingIndividualComponent() || SetSiblingIndividualComponent())
	{
		TextComponent->AddTextLine(SelfTextLineKey);
		TextComponent->AddTextLine(SiblingIndividualTextLineKey);

		// Set the current text of the sibling
		SetOwnStateValuesText();
		SetSiblingIndividualStateValuesText();

		// Listen to the siblings changes
		Connect();
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not acces the individual component sibling, this should not happen.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Private load implementation
bool USLIndividualInfoComponent::LoadImpl()
{
	if (HasValidSiblingIndividualComponent())
	{
		// Force publishing sibling individual values
		SiblingIndividualComponent->TriggerValuesBroadcast();
		return true;
	}
	return false;
}

// Update info as soon as the individual changes their data
bool USLIndividualInfoComponent::BindDelegates()
{
	bool bRetVal = true;
	if (!HasValidSiblingIndividualComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component sibling is not valid, cannot bind delegates.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	/* Init */
	if (!SiblingIndividualComponent->OnInitChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnSiblingIndividualComponentInitChanged))
	{
		SiblingIndividualComponent->OnInitChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnSiblingIndividualComponentInitChanged);			
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's init changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Load */
	if (!SiblingIndividualComponent->OnLoadedChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnSiblingIndividualComponentLoadedChanged))
	{
		SiblingIndividualComponent->OnLoadedChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnSiblingIndividualComponentLoadedChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's loaded changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Value Changed */
	if (!SiblingIndividualComponent->OnValueChanged.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnSiblingIndividualComponentValueChanged))
	{
		SiblingIndividualComponent->OnValueChanged.AddDynamic(
			this, &USLIndividualInfoComponent::OnSiblingIndividualComponentValueChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's on value changed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Destroyed */
	if (!SiblingIndividualComponent->OnDestroyed.IsAlreadyBound(
		this, &USLIndividualInfoComponent::OnSiblingIndividualComponentDestroyed))
	{
		SiblingIndividualComponent->OnDestroyed.AddDynamic(
			this, &USLIndividualInfoComponent::OnSiblingIndividualComponentDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d info component %s's destroyed delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	return true;
}

// Check if sibling individual component is set
bool USLIndividualInfoComponent::HasValidSiblingIndividualComponent() const
{
	return SiblingIndividualComponent && SiblingIndividualComponent->IsValidLowLevel() && !SiblingIndividualComponent->IsPendingKill();
}

// Set the sibling individual component
bool USLIndividualInfoComponent::SetSiblingIndividualComponent()
{
	if (HasValidSiblingIndividualComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual component owner already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	// Check if the owner has an individual component
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		SiblingIndividualComponent = CastChecked<USLIndividualComponent>(AC);
		return true;
	}
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
void USLIndividualInfoComponent::SetOwnStateValuesText()
{	
	const FString TextValue = FString::Printf(TEXT("%s : I:%s; L:%s; C:%s;"),
		ANSI_TO_TCHAR(SelfTextLineKey), IsInit() ? "T" : "F", IsLoaded() ? "T" : "F", IsConnected() ? "T" : "F");
	TextComponent->SetTextLineValue(SelfTextLineKey, TextValue);
	if (IsConnected())
	{
		if (IsLoaded())
		{
			TextComponent->SetTextLineColor(SelfTextLineKey, FColor::Green);
		}
		else if (IsInit())
		{
			TextComponent->SetTextLineColor(SelfTextLineKey, FColor::Yellow);
		}
		else
		{
			TextComponent->SetTextLineColor(SelfTextLineKey, FColor::Red);
		}
	}
	else
	{
		TextComponent->SetTextLineColor(SelfTextLineKey, FColor::Silver);
	}
}

// Set its individuals state values
void USLIndividualInfoComponent::SetSiblingIndividualStateValuesText()
{		
	if (HasValidSiblingIndividualComponent())
	{
		const FString TextValue = FString::Printf(TEXT("%s : I:%s; L:%s; C:%s;"),
			*FString(SiblingIndividualTextLineKey), IsInit() ? "T" : "F", IsLoaded() ? "T" : "F", IsConnected() ? "T" : "F");
		TextComponent->SetTextLineValue(SiblingIndividualTextLineKey, TextValue);
		if (SiblingIndividualComponent->IsConnected())
		{
			if (SiblingIndividualComponent->IsLoaded())
			{
				TextComponent->SetTextLineColor(SiblingIndividualTextLineKey, FColor::Green);
			}
			else if (SiblingIndividualComponent->IsInit())
			{
				TextComponent->SetTextLineColor(SiblingIndividualTextLineKey, FColor::Yellow);
			}
			else
			{
				TextComponent->SetTextLineColor(SiblingIndividualTextLineKey, FColor::Red);
			}
		}
		else
		{
			TextComponent->SetTextLineColor(SiblingIndividualTextLineKey, FColor::Silver);
		}
	}
	else
	{
		TextComponent->SetTextLineColor(SiblingIndividualTextLineKey, FColor::White);
		TextComponent->SetTextLineValue(SiblingIndividualTextLineKey, FString::Printf(TEXT("%s : null"), SiblingIndividualTextLineKey));
	}
}


/* Delegate functions */
// Called when owners init value has changed
void USLIndividualInfoComponent::OnSiblingIndividualComponentInitChanged(USLIndividualComponent* IC, bool bNewVal)
{
	SetSiblingIndividualStateValuesText();
}

// Called when owners load value has changed
void USLIndividualInfoComponent::OnSiblingIndividualComponentLoadedChanged(USLIndividualComponent* IC, bool bNewVal)
{
	SetSiblingIndividualStateValuesText();
}

// Called when the siblings connected value has changed
void USLIndividualInfoComponent::OnSiblingIndividualComponentConnectedChanged(USLIndividualComponent* IC, bool bNewVal)
{
	SetSiblingIndividualStateValuesText();
}

// Called when the siblings values have changed
void USLIndividualInfoComponent::OnSiblingIndividualComponentValueChanged(USLIndividualComponent* IC, const FString& NewKey, const FString& NewValue)
{
	TextComponent->SetTextLineValueAndColor(NewKey, NewValue, FColor::Silver);
}

// Called when owner is being destroyed
void USLIndividualInfoComponent::OnSiblingIndividualComponentDestroyed(USLIndividualComponent* IC)
{
	// Trigger self destruct
	ConditionalBeginDestroy();
}

// Called when sibling delegates are cleared
void USLIndividualInfoComponent::OnSiblingIndividualComponentDelegatesClearedDestroyed(USLIndividualComponent* IC)
{
	SetIsConnected(false);
}

