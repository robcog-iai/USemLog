// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLCVScanner.generated.h"

// Forward declarations
class ASLIndividualManager;
class ASLMongoQueryManager;
class USLVisibleIndividual;
class AStaticMeshActor;
class UGameViewportClient;
class UMaterialInstanceDynamic;
class ADirectionalLight;
class USLCVQScene;

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
enum class ESLCVRenderMode : uint8
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
	// Setup user input bindings
	void SetupInputBindings();

	// Request a high res screenshot
	void RequestScreenshotAsync();

	// Called when the screenshot is captured
	void ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap);
	
	// Set next view mode (return false if the last view mode was reached)
	bool SetNextRenderMode();

	// Set next camera pose (return false if the last was reached)
	bool SetNextCameraPose();

	// Set next view (individual or scene) (return false if the last was reached)
	bool SetNextScene();

	//  Quit the editor when finished
	void QuitEditor();

private:
	// Apply the selected view mode
	void ApplyRenderMode(ESLCVRenderMode Mode);

	// Apply the camera pose
	void ApplyCameraPose(FTransform UnitSpherePose);

	// Apply the individual into position
	void ApplyIndividual(USLVisibleIndividual* Individual);

	// Set the individual / scene 
	void ApplyScene();

	// Hide mask clone, show original individual
	void ShowOriginalIndividual();

	// Hide original individual, show mask clone
	void ShowMaskIndividual();

	// Remove detachments and hide all actors in the world
	void HideAllActors();

	// Detach all actors
	void DetachAllActors();

	// Disable physiscs and detach all actors
	void DisablePhysicsOnAllActors();

	// Set ppv proerties (disable / ambient occlusion)
	void SetPostProcessVolumeProperties();

	// Set screenshot image resolution
	void SetScreenshotResolution(FIntPoint InResolution);

	// Set the rendering parameters
	void SetRenderParams();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Get the mongo query manager (used to set up scenes from episodic memories)
	bool SetMongoQueryManager();

	// Set the individuals to be scanned
	bool SetScanIndividuals();

	// Set the scenes to be scanned
	bool SetScanScenes();

	// Spawn a light actor which will also be used to move the camera around
	bool SetCameraPoseAndLightActor();

	// Create clones of the individuals with mask material
	bool SetMaskClones();

	// Generate mask clones from the ids
	void GenerateMaskClones(const TArray<USLVisibleIndividual*>& VisibleIndividuals);

	// Set the background static mesh actor and material
	bool SetBackgroundStaticMeshActor();

	// Generate sphere camera scan poses
	bool SetScanPoses(uint32 MaxNumPoints/*, float Radius = 1.f*/);

	// Set the image name
	void SetImageName();

	// Calculate camera pose sphere radius (proportionate to the sphere bounds of the visual mesh)
	void SetCameraPoseSphereRadius();

	// Print progress to terminal
	void PrintProgress() const;

	// Save image to file
	void SaveToFile(const TArray<uint8>& CompressedBitmap) const;

protected:
	// Skip auto init and start
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// Request the screenshots manually
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bManualTrigger : 1;

	// Folder to store the images in
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bManualTrigger"))
	FName UserInputActionName = "SLGenericTrigger";

	// Output progress to terminal
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bPrintProgress : 1;

	// Choose what to scan (individuals or scenes)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLCVScanMode ScanMode = ESLCVScanMode::Individuals;

	// Scan only the selected individuals
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bScanOnlySelectedIndividuals : 1;

	// List of individuals to scan
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bScanOnlySelectedIndividuals && ScanMode==ESLCVScanMode::Individuals"))
	TArray<FString> SelectedIndividualsId;

	// Scenes to scan
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "ScanMode==ESLCVScanMode::Scenes"))
	TArray<USLCVQScene*> Scenes;

	// Maximal individual size
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "!bScanOnlySelectedIndividuals && ScanMode==ESLCVScanMode::Individuals"))
	float MaxBoundsSphereRadius = 500.f;

	// Folder to store the images in
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Location")
	FString TaskId = "DefaultTaskId";

	// Save images to file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Location")
	uint8 bSaveToFile : 1;

	// Overwrite files
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Location")
	uint8 bOverwrite : 1;

	// Use unique ids or the actor name as folder names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Location", meta = (editcondition = "bSaveToFile && ScanMode==ESLCVScanMode::Individuals"))
	uint8 bUseIdsForFolderNames : 1;

	// Maximal number of scan points on the sphere
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	uint32 MaxNumScanPoints = 32;

	// Image resolution
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	FIntPoint Resolution = FIntPoint(640, 640);

	// Rendering modes to include
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	TArray<ESLCVRenderMode> RenderModes;

	// Color of the background (rendered or replaced in postprocess)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	FColor CustomBackgroundColor = FColor::Black;

	// If true, instead of rendereing the backgorund, replace the black pixels with the value
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	uint8 bReplaceBackgroundPixels : 1;

	// Manhattan distance between the pixel color to replace
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image", meta = (editcondition = "bReplaceBackgroundPixels"))
	int32 CustomBackgroundColorTolerance = 7;

	// Color of the mask image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	uint8 bUseIndividualMaskValue : 1;

	// Color of the mask image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image", meta = (editcondition = "!bUseIndividualMaskValue"))
	FColor MaskColor = FColor::White;

	// Disable post process volumes in the world
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	uint8 bDisablePostProcessVolumes : 1;

	// Disable ambient occlusion (world and process volumes)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	uint8 bDisableAO : 1;

	// Directional camera light intensity
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	float CameraLightIntensity = 1.6f;

	// The scene bounding box radius camera multiplier
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Image")
	float CameraRadiusDistanceMultiplier = 1.5f;

	/* Edit */
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

	/* Debug */
	// Debug params
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Debug")
	float DebugArrowThickness = 0.f;

	// Debug params
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Debug")
	float DebugArrowHeadSize = 1.f;

	// Debug params (0 - sphere origin, 1 - arrow origin)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Debug")
	float ArrowHeadLocPerc = 0.95f;

	// Debug params
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Debug")
	FColor StartColorLerp = FColor::Green;

	// Debug params
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Debug")
	FColor EndColorLerp = FColor::Magenta;

	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done 
	uint8 bIsFinished : 1;

	// Used to access the actors given their ids 
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Used to to access the locations of the scenes in the episodic memories
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLMongoQueryManager* MongoQueryManager;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY()
	ADirectionalLight* CameraPoseAndLightActor;

	// Mesh used for the background
	UPROPERTY()
	AStaticMeshActor* BackgroundSMA;

private:
	// Camera poses on the unit sphere (this will be multiplied with each scenes bounds spehre radius)
	TArray<FTransform> CameraScanUnitPoses;

	// Individuals to calibrate
	TArray<USLVisibleIndividual*> Individuals;

	// Clones with dynamic mask materials on
	TMap<USLVisibleIndividual*, AStaticMeshActor*> IndividualsMaskClones;

	// Current active view mode
	ESLCVRenderMode PrevRenderMode = ESLCVRenderMode::NONE;

	// Current radius of the camera sphere poses
	float CurrCameraPoseSphereRadius = 1.f;

	// Current individual index in the array
	int32 IndividualOrSceneIdx = INDEX_NONE;

	// Current camera pose index in the array
	int32 CameraPoseIdx = INDEX_NONE;

	// Current view mode index in the array
	int32 RenderModeIdx = INDEX_NONE;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// The name of the current image
	FString CurrImageName;

	// The individual id (used for the folder name)
	FString SceneNameString;

	// View mode as string (used as folder and image name)
	FString RenderModeString;

	// The camera pose post fix to be applied to the image name
	FString CameraPoseIdxString;

	// The individual or scene index as string
	FString IndividualOrSceneIdxString;

	/* Constants */
	static constexpr auto DynMaskMatAssetPath = TEXT("/USemLog/CV/M_SLDefaultMask.M_SLDefaultMask");
	static constexpr auto BackgroundAssetPath = TEXT("/USemLog/CV/Background/SM_CVBackgroundSphere.SM_CVBackgroundSphere");
	static constexpr auto BackgroundDynMatAssetPath = TEXT("/USemLog/CV/Background/M_CVBackground.M_CVBackground");
};
