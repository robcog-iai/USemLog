// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"

/**
 *
 */
class USEMLOGVIS_API RawDataAsyncWorker2 : public FNonAbandonableTask
{
private:
	int Width;
	int Height;
	FDateTime TimeStamp;
	FString FolderName;
	FString ImageName;
	FString Episode;
	FString CameraName;
	TSharedPtr<IImageWrapper> ImageWrapper;
	TArray<FColor> Image;
public:
	RawDataAsyncWorker2(TArray<FColor>& Image_init, TSharedPtr<IImageWrapper>& ImageWrapperRef, FDateTime Stamp, FString Folder, FString Name, int Width_init, int Height_init, FString ImageEpisode, FString CameraId);
	~RawDataAsyncWorker2();
	FORCEINLINE TStatId GetStatId() const;
	void DoWork();
	void SetLogToImage();
	void SaveImage(TArray<FColor>&image, TSharedPtr<IImageWrapper> &ImageWrapper, FDateTime Stamp, FString Folder, FString ImageName, int Width, int Height, FString Episode, FString CameraId);
};
