// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SemLogEdModule.h"
#include "SemLogEdStyle.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"


#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FSemLogEdStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

FString FSemLogEdStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("USemLog"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FSemLogEdStyle::StyleSet = NULL;
TSharedPtr< class ISlateStyle > FSemLogEdStyle::Get() { return StyleSet; }

FName FSemLogEdStyle::GetStyleSetName()
{
	static FName SemLogStyleName(TEXT("SemLogEdStyle"));
	return SemLogStyleName;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FSemLogEdStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	// Style
	{
		StyleSet->Set("LevelEditor.SemLogEd", new IMAGE_PLUGIN_BRUSH("Icons/icon_Mode_SemLog_40px", Icon40x40));
		StyleSet->Set("LevelEditor.SemLogEd.Small", new IMAGE_PLUGIN_BRUSH("Icons/icon_Mode_SemLog_40px", Icon20x20));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef IMAGE_PLUGIN_BRUSH
#undef IMAGE_BRUSH

void FSemLogEdStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
