#include "SemLogEditorPrivatePCH.h"
#include "BaseEditorTool.h"
#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "BaseEditorToolCustomization.h"
#include "DemoCommands.h"
#include "DemoStyle.h"

#define LOCTEXT_NAMESPACE "DemoTools"

class FSemLogEditorModule : public IModuleInterface
{
public:
	// IMoudleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End of IModuleInterface interface
		
	static void TriggerTool(UClass* ToolClass);
	static void CreateToolListMenu(class FMenuBuilder& MenuBuilder);
	static void CreateWindowListMenu(class FMenuBuilder& MenuBuilder);
	static void OnToolWindowClosed(const TSharedRef<SWindow>& Window, UBaseEditorTool* Instance);

	static void HandleTestCommandExcute();

	static bool HandleTestCommandCanExcute();

	TSharedPtr<FUICommandList> CommandList;
};

void FSemLogEditorModule::StartupModule()
{
	// Register the details customizations
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyModule.RegisterCustomClassLayout(TEXT("BaseEditorTool"), FOnGetDetailCustomizationInstance::CreateStatic(&FBaseEditorToolCustomization::MakeInstance));
	}

	// Register slate style overrides
	FDemoStyle::Initialize();

	// Register commands
	FDemoCommands::Register();
	CommandList = MakeShareable(new FUICommandList);

	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

		CommandList->Append(LevelEditorModule.GetGlobalLevelEditorActions());

		CommandList->MapAction(
			FDemoCommands::Get().TestCommand,
			FExecuteAction::CreateStatic(&FSemLogEditorModule::HandleTestCommandExcute),
			FCanExecuteAction::CreateStatic(&FSemLogEditorModule::HandleTestCommandCanExcute)
			);

		struct Local
		{
			static void AddToolbarCommands(FToolBarBuilder& ToolbarBuilder)
			{
				ToolbarBuilder.AddToolBarButton(FDemoCommands::Get().TestCommand);
			}

			static void AddMenuCommands(FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddSubMenu(LOCTEXT("DemoTools", "Demo Tools"),
					LOCTEXT("DemoToolsTooltip", "List of tools"),
					FNewMenuDelegate::CreateStatic(&FSemLogEditorModule::CreateToolListMenu)
				);
			}

			static void AddWindowCommands(FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddSubMenu(LOCTEXT("SemanticLogger", "Semantic Logger"),
					LOCTEXT("ListSemanticLogger", "List of semantic logging tools"),
					FNewMenuDelegate::CreateStatic(&FSemLogEditorModule::CreateWindowListMenu)
					);
			}

			static void AddWindowTab(FMenuBuilder& MenuBuilder)
			{
				FString FriendlyName = TEXT("SemLog");
				FText MenuDescription = FText::Format(LOCTEXT("SemLogDescription", "{0}"), FText::FromString(FriendlyName));
				FText MenuTooltip = FText::Format(LOCTEXT("SemLogMenuTooltip", "Execute the {0} tool"), FText::FromString(FriendlyName));

				FUIAction Action(FExecuteAction::CreateStatic(&FSemLogEditorModule::TriggerTool, UBaseEditorTool::StaticClass()));

				MenuBuilder.AddMenuEntry(
					MenuDescription,
					MenuTooltip,
					FSlateIcon(),
					Action,
					TEXT("LevelEditor"));
			}
		};

		TSharedRef<FExtender> MenuExtender(new FExtender());
		MenuExtender->AddMenuExtension(
			TEXT("EditMain"),
			EExtensionHook::After,
			CommandList.ToSharedRef(),
			FMenuExtensionDelegate::CreateStatic(&Local::AddMenuCommands));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

		TSharedRef<FExtender> WindowExtender(new FExtender());
		WindowExtender->AddMenuExtension(
			TEXT("LevelEditor"),
			EExtensionHook::After,
			CommandList.ToSharedRef(),
			FMenuExtensionDelegate::CreateStatic(&Local::AddWindowCommands));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(WindowExtender);

		TSharedRef<FExtender> ToolbarExtender(new FExtender());
		ToolbarExtender->AddToolBarExtension(
			TEXT("Game"),
			EExtensionHook::After,
			CommandList.ToSharedRef(),
			FToolBarExtensionDelegate::CreateStatic(&Local::AddToolbarCommands));
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FSemLogEditorModule::ShutdownModule()
{
	FDemoCommands::Unregister();
	FDemoStyle::Shutdown();
}

void FSemLogEditorModule::TriggerTool(UClass* ToolClass)
{
	UBaseEditorTool* ToolInstance = NewObject<UBaseEditorTool>(GetTransientPackage(), ToolClass);
	ToolInstance->AddToRoot();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	TArray<UObject*> ObjectsToView;
	ObjectsToView.Add(ToolInstance);
	TSharedRef<SWindow> Window = PropertyModule.CreateFloatingDetailsView(ObjectsToView, /*bIsLockeable=*/ false);

	Window->SetOnWindowClosed(FOnWindowClosed::CreateStatic(&FSemLogEditorModule::OnToolWindowClosed, ToolInstance));
}

void FSemLogEditorModule::CreateToolListMenu(class FMenuBuilder& MenuBuilder)
{
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (!Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Abstract))
		{
			if (Class->IsChildOf(UBaseEditorTool::StaticClass()))
			{
				FString FriendlyName = Class->GetName();
				FText MenuDescription = FText::Format(LOCTEXT("ToolMenuDescription", "{0}"), FText::FromString(FriendlyName));
				FText MenuTooltip = FText::Format(LOCTEXT("ToolMenuTooltip", "Execute the {0} tool"), FText::FromString(FriendlyName));

				FUIAction Action(FExecuteAction::CreateStatic(&FSemLogEditorModule::TriggerTool, Class));

				MenuBuilder.AddMenuEntry(
					MenuDescription,
					MenuTooltip,
					FSlateIcon(),
					Action);
			}
		}
	}
}

void FSemLogEditorModule::CreateWindowListMenu(class FMenuBuilder& MenuBuilder)
{
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (!Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Abstract))
		{
			if (Class->IsChildOf(UBaseEditorTool::StaticClass()))
			{
				FString FriendlyName = Class->GetName();
				FText MenuDescription = FText::Format(LOCTEXT("ToolMenuDescription", "{0}"), FText::FromString(FriendlyName));
				FText MenuTooltip = FText::Format(LOCTEXT("ToolMenuTooltip", "Execute the {0} tool"), FText::FromString(FriendlyName));

				FUIAction Action(FExecuteAction::CreateStatic(&FSemLogEditorModule::TriggerTool, Class));

				MenuBuilder.AddMenuEntry(
					MenuDescription,
					MenuTooltip,
					FSlateIcon(),
					Action);
			}
		}
	}
}

void FSemLogEditorModule::OnToolWindowClosed(const TSharedRef<SWindow>& Window, UBaseEditorTool* Instance)
{
	Instance->RemoveFromRoot();
}

void FSemLogEditorModule::HandleTestCommandExcute()
{
	FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, TEXT("Test Command Excuted!!!"), TEXT("TestCommand"));
}

bool FSemLogEditorModule::HandleTestCommandCanExcute()
{
	return true;
}

IMPLEMENT_MODULE(FSemLogEditorModule, SemLogEditor);

#undef LOCTEXT_NAMESPACE