// Copyright Epic Games, Inc. All Rights Reserved.

#include "DBTPluginTest.h"
#include "DBTBehaviorTreeDataManager.h"
#include "DynamicTaskNode.h"
#include "DynamicRootNodeCustomization.h"
#include "AbilityCounterComponent.h"
#include "AssetTypeActions_Base.h"
#include "PropertyEditorModule.h"
#include "BehaviorTree/BTTaskNode.h"

#define LOCTEXT_NAMESPACE "FDBTPluginTestModule"

void FDBTPluginTestModule::StartupModule()
{
	GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Plugin loaded!"));

#if WITH_EDITOR
	RegisterTaskNodeCustomizations();
#endif
}

void FDBTPluginTestModule::ShutdownModule()
{
	GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Plugin unloaded!"));

#if WITH_EDITOR
	UnregisterTaskNodeCustomizations();

	FTaskNodeCustomization::ClearFlagsMap();
#endif

	UDBTBehaviorTreeDataManager::Release();
}

#if WITH_EDITOR
void FDBTPluginTestModule::RegisterTaskNodeCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// List of standard Task classes for registration
		TArray<FName> TaskClassNames = {
			FName("BTTaskNode"),                 // Base class for all BT Tasks
			FName("BTTask_BlackboardBase"),      // Base class with Blackboard
			FName("BTTask_BlueprintBase"),       // Blueprint-based task
			FName("BTTask_Wait"),                // Task Wait
			FName("BTTask_MoveTo"),              // Task MoveTo
			FName("BTTask_RotateToFaceBBEntry"), // Task RotateToFaceBBEntry
			FName("BTTask_PlayAnimation"),       // Task PlayAnimation
			FName("BTTask_RunEQSQuery")          // Task RunEQSQuery
		};

		int length_standart_class = 0;
		for (const FName& ClassName : TaskClassNames)
		{
			PropertyModule.RegisterCustomClassLayout(
				ClassName,
				FOnGetDetailCustomizationInstance::CreateStatic(&FTaskNodeCustomization::MakeInstance)
			);

			length_standart_class++;
		}

		if (length_standart_class == TaskClassNames.Num())
		{
			GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Standart task nodes customizations registered successfully"));
		}

		RegisterAllTaskClasses(PropertyModule);

		PropertyModule.RegisterCustomClassLayout(
			FName("BTCompositeNode"),
			FOnGetDetailCustomizationInstance::CreateStatic(&FBehaviorTreeRootNodeCustomization::MakeInstance)
		);

		GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Root nodes customizations registered successfully"));

		PropertyModule.NotifyCustomizationModuleChanged();

		GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: All task nodes customizations registered successfully"));
	}
}

void FDBTPluginTestModule::RegisterAllTaskClasses(FPropertyEditorModule& PropertyModule)
{
	UClass* BTTaskNodeClass = UBTTaskNode::StaticClass();

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* CurrentClass = *ClassIt;

		if (CurrentClass->IsChildOf(BTTaskNodeClass))
		{
			if (CurrentClass != BTTaskNodeClass &&
				!CurrentClass->GetName().StartsWith("BTTask_"))
			{
				PropertyModule.RegisterCustomClassLayout(
					CurrentClass->GetFName(),
					FOnGetDetailCustomizationInstance::CreateStatic(&FTaskNodeCustomization::MakeInstance)
				);

				GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Registered customization for custom task %s"), *CurrentClass->GetName());
			}
		}
	}
}

void FDBTPluginTestModule::UnregisterTaskNodeCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(FName("DynamicBTTaskNode"));
		PropertyModule.UnregisterCustomClassLayout(FName("DynamicBTCompositeNode"));

		PropertyModule.NotifyCustomizationModuleChanged();

		GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Dynamic node customizations unregistered"));
	}
}
#endif

void FDBTPluginTestModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<class IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDBTPluginTestModule, DBTPluginTest)