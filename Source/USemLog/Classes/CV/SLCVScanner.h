// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLCVScanner.generated.h"

// Forward declarations
class ASLIndividualManager;
class USLVisibleIndividual;
class AStaticMeshActor;
class UGameViewportClient;
class UMaterialInstanceDynamic;
class ADirectionalLight;

/**
* Scan modes
*/
UENUM()
enum class ESLCVScanMode : uint8
{
	Individuals				UMETA(DisplayName = "Individuals"),
	Scenes					UMETA(DisplayName = "Scenes"),
};

/**
* View modes
*/
UENUM()
enum class ESLCVViewMode : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Lit						UMETA(DisplayName = "Lit"),
	Unlit					UMETA(DisplayName = "Unlit"),
	Mask					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
};

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL CV Scanner")
class ASLCVScanner : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLCVScanner();

	// Dtor
	~ASLCVScanner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Set up any required references and connect to server
	void Init();

	// Start processing any incomming messages
	void Start();

	// Stop processing the messages, and disconnect from server
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Request a high res screenshot
	void RequestScreenshotAsync();

	// Called when the screenshot is captured
	void ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap);
	
	// Set next view mode (return false if the last view mode was reached)
	bool SetNextViewMode();

	// Set next camera pose (return false if the last was reached)
	bool SetNextCameraPose();

	// Set next view (individual or scene) (return false if the last was reached)
	bool SetNextView();

	//  Quit the editor when finished
	void QuitEditor();

private:
	// Apply the selected view mode
	void ApplyViewMode(ESLCVViewMode Mode);

	// Apply the camera pose
	void ApplyCameraPose(FTransform NormalizedTransform);

	// Apply the individual into position
	void ApplyIndividual(USLVisibleIndividual* Individual);

	// Apply the scene into position
	void ApplyScene();

	// Hide mask clone, show original individual
	void ShowOriginalIndividual();

	// Hide original individual, show mask clone
	void ShowMaskIndividual();

	// Remove detachments and hide all actors in the world
	void SetWorldState();

	// Set screenshot image resolution
	void SetScreenshotResolution(FIntPoint InResolution);

	// Set the rendering parameters
	void SetRenderParams();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Set the individuals to be scanned
	bool SetScanIndividuals();

	// Spawn a light actor which will also be used to move the camera around
	bool SetCameraPoseAndLightActor();

	// Create clones of the individuals with mask material
	bool SetMaskClones();

	// Set the background static mesh actor and material
	bool SetBackgroundStaticMeshActor();

	// Generate sphere camera scan poses
	bool SetScanPoses(uint32 MaxNumPoints/*, float Radius = 1.f*/);

	// Set the image name
	void SetImageName();

	// Calculate camera pose sphere radius (proportionate to the sphere bounds of the visual mesh)
	void CalcCameraPoseSphereRadius();

	// Print progress to terminal
	void PrintProgress() const;

	// Save image to file
	void SaveToFile(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap) const;

protected:
	// Skip auto init and start
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// Save images to file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bSaveToFile : 1;

	// Save images to file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bPrintProgress : 1;

	// Use parent actor names for folder names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bUseActorNamesForFolders : 1;

	// Choose what to scan (individuals or scenes)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLCVScanMode ScanMode = ESLCVScanMode::Individuals;

	// Scan only the selected individuals
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bScanOnlySelectedIndividuals : 1;

	// List of individuals to scan
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bScanOnlySelectedIndividuals && ScanMode==ESLCVScanMode::Individuals"))
	TArray<FString> SelectedIndividualsId;

	// Maximal individual size
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "!bScanOnlySelectedIndividuals && ScanMode==ESLCVScanMode::Individuals"))
	float MaxBoundsSphereRadius = 500.f;

	// Color of the background
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FColor BackgroundColor = FColor::Black;

	// Color of the mask image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bUseIndividualMaskValue : 1;

	// Color of the mask image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "!bUseIndividualMaskValue"))
	FColor MaskColor = FColor::White;

	// Disable post process volumes in the world
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bDisablePostProcessVolumes : 1;

	// Disable ambient occlusion (world and process volumes)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bDisableAO : 1;

	// Add ids from selection button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	FString IdCSVString;

	// Add ids from selection button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bAddSelectedIdsButton = false;

	// Remove selected ids from selection button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bRemoveSelectedIdsButton = false;

	// Overwrite selection
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bOverwriteSelectedIds = false;

	// Scenes to scan
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta=(editcondition="ScanMode==ESLCVScanMode::Scenes"))
	TArray<FString> Scenes;

	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done 
	uint8 bIsFinished : 1;
	
	// Rendering modes to include
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<ESLCVViewMode> ViewModes;

	// Maximal number of scan points on the sphere
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint32 MaxNumScanPoints = 32;

	// Image resolution
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FIntPoint Resolution = FIntPoint(640, 480);



	// Directional camera light intensity
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float CameraLightIntensity = 1.6f;

	// Folder to store the images in
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString TaskId = "DefaultTaskId";

	// Mongo server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 MongoServerPort = 27017;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY()
	ADirectionalLight* CameraPoseAndLightActor;

	// Mesh used for the background
	UPROPERTY()
	AStaticMeshActor* BackgroundSMA;
	
	// Material used to render the masks
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaskMaterial;

private:
	// Camera poses on the sphere
	TArray<FTransform> CameraScanPoses;

	// Individuals to calibrate
	TArray<USLVisibleIndividual*> Individuals;

	// Clones with dynamic mask materials on
	TMap<USLVisibleIndividual*, AStaticMeshActor*> IndividualsMaskClones;

	// Current active view mode
	ESLCVViewMode CurrViewmode = ESLCVViewMode::NONE;

	// Current radius of the camera sphere poses
	float CurrCameraPoseSphereRadius;

	// Current individual index in the array
	int32 ViewIdx = INDEX_NONE;

	// Current camera pose index in the array
	int32 CameraPoseIdx = INDEX_NONE;

	// Current view mode index in the array
	int32 ViewModeIdx = INDEX_NONE;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// The name of the current image
	FString CurrImageName;

	// The individual id (used for the folder name)
	FString ViewNameString;

	// View mode as string (used as folder and image name)
	FString ViewModeString;

	// The camera pose post fix to be applied to the image name
	FString CameraPoseIdxString;

	// The camera pose post fix to be applied to the image name
	FString ViewIdxString;

	/* Constants */
	static constexpr auto DynMaskMatAssetPath = TEXT("/USemLog/CV/M_SLDefaultMask.M_SLDefaultMask");
	static constexpr auto BackgroundAssetPath = TEXT("/USemLog/CV/Background/SM_CVBackgroundSphere.SM_CVBackgroundSphere");
	static constexpr auto BackgroundDynMatAssetPath = TEXT("/USemLog/CV/Background/M_CVBackground.M_CVBackground");
};
