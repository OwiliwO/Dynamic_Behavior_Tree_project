// Copyright Epic Games, Inc. All Rights Reserved.

#include "DBTPluginTest.h"
#include "DynamicTaskNode.h"
#include "AssetTypeActions_Base.h"
#include "PropertyEditorModule.h"
#include "BehaviorTree/BTTaskNode.h"

#define LOCTEXT_NAMESPACE "FDBTPluginTestModule"

void FDBTPluginTestModule::StartupModule()
{
	GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Plugin loaded!"));

	RegisterTaskNodeCustomizations();
}

void FDBTPluginTestModule::ShutdownModule()
{
	GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Plugin unloaded!"));

	UnregisterTaskNodeCustomizations();

	FTaskNodeCustomization::ClearFlagsMap();
}

void FDBTPluginTestModule::RegisterTaskNodeCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			
		// Список стандартных классов Task для регистрации
		TArray<FName> TaskClassNames = {
			FName("BTTaskNode"),                 // Базовый класс всех BT Task
			FName("BTTask_BlackboardBase"),      // Базовый класс с Blackboard
			FName("BTTask_BlueprintBase"),       // Blueprint-based задачи
			FName("BTTask_Wait"),                // Узел Wait
			FName("BTTask_MoveTo"),              // Узел MoveTo
			FName("BTTask_RotateToFaceBBEntry"), // Узел RotateToFaceBBEntry
			FName("BTTask_PlayAnimation"),       // Узел PlayAnimation
			FName("BTTask_RunEQSQuery")          // Узел RunEQSQuery
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
			GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Standart task node customizations registered successfully"));
		}
	
		RegisterAllTaskClasses(PropertyModule);
	
		PropertyModule.NotifyCustomizationModuleChanged();
	
		GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: All task node customizations registered successfully"));
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

		TArray<FName> TaskClassNames = {
			FName("BTTaskNode"),
			FName("BTTask_BlackboardBase"),
			FName("BTTask_BlueprintBase"),
			FName("BTTask_Wait"),
			FName("BTTask_MoveTo"),
			FName("BTTask_RotateToFaceBBEntry"),
			FName("BTTask_PlayAnimation"),
			FName("BTTask_RunEQSQuery")
		};

		for (const FName& ClassName : TaskClassNames)
		{
			PropertyModule.UnregisterCustomClassLayout(ClassName);
		}

		UClass* BTTaskNodeClass = UBTTaskNode::StaticClass();

		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* CurrentClass = *ClassIt;

			if (CurrentClass->IsChildOf(BTTaskNodeClass))
			{
				if (CurrentClass != BTTaskNodeClass &&
					!CurrentClass->GetName().StartsWith("BTTask_"))
				{
					PropertyModule.UnregisterCustomClassLayout(CurrentClass->GetFName());
				}
			}
		}

		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

void FDBTPluginTestModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<class IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDBTPluginTestModule, DBTPluginTest)