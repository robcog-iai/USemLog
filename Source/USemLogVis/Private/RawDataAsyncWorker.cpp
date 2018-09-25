// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "RawDataAsyncWorker2.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"


RawDataAsyncWorker2::RawDataAsyncWorker2(TArray<FColor>& Image_init, TSharedPtr<IImageWrapper>& ImageWrapperRef, FDateTime Stamp, FString Folder, FString Name, int Width_init, int Height_init, FString ImageEpisode, FString CameraId)
{
	Width = Width_init;
	Height = Height_init;
	TimeStamp = Stamp;
	FolderName = Folder;
	ImageName = Name;
	ImageWrapper = ImageWrapperRef;
	Image = Image_init;
	Episode = ImageEpisode;
	CameraName = CameraId;

}

RawDataAsyncWorker2::~RawDataAsyncWorker2()
{
	UE_LOG(LogTemp, Warning, TEXT("Task Deleted"));
}

TStatId RawDataAsyncWorker2::GetStatId() const
{
	return TStatId();
}

void RawDataAsyncWorker2::DoWork()
{
	UE_LOG(LogTemp, Warning, TEXT("Task Begin"));
	if (Width > 0 && Height > 0)
	{
		SaveImage(Image, ImageWrapper, TimeStamp, FolderName, ImageName, Width, Height, Episode, CameraName);
	}

}

void RawDataAsyncWorker2::SetLogToImage()
{
}

void RawDataAsyncWorker2::SaveImage(TArray<FColor>& image, TSharedPtr<IImageWrapper>& ImageWrapper, FDateTime Stamp, FString Folder, FString ImageName, int Width, int Height, FString Episode, FString CameraId)
{
	// get the time stamp
	FString TimeStamp = FString::FromInt(Stamp.GetYear()) + "_" + FString::FromInt(Stamp.GetMonth()) + "_" + FString::FromInt(Stamp.GetDay())
		+ "_" + FString::FromInt(Stamp.GetHour()) + "_" + FString::FromInt(Stamp.GetMinute()) + "_" + FString::FromInt(Stamp.GetSecond()) + "_" +
		FString::FromInt(Stamp.GetMillisecond());
	UE_LOG(LogTemp, Warning, TEXT("Height %i,Width %i"), Height, Width);
	// initial Image Wrapper
	TArray<uint8> ImgData;
	ImageWrapper->SetRaw(image.GetData(), image.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);
	ImgData = ImageWrapper->GetCompressed();



	//save image in local disk as image
	FString FileName = ImageName + TimeStamp + ".jpg";
	FString FileDir = FPaths::ProjectDir() + TEXT("/Vis Logger/") + Episode + "/" + CameraId + "/" + TEXT("viewport/") + Folder + "/";
	FPaths::RemoveDuplicateSlashes(FileDir);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FileDir)) {
		PlatformFile.CreateDirectoryTree(*FileDir);

	}
	FString AbsolutePath = FileDir + FileName;
	FFileHelper::SaveArrayToFile(ImgData, *AbsolutePath);
}

