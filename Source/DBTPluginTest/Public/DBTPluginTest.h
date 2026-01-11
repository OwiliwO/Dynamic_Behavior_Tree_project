// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDBTPluginTestModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:

	void RegisterAssetTypeAction(class IAssetTools& AssetTools, TSharedRef<class IAssetTypeActions> Action);
	void RegisterTaskNodeCustomizations();
	void RegisterAllTaskClasses(FPropertyEditorModule& PropertyModule);
	void UnregisterTaskNodeCustomizations();
	
	TArray<TSharedPtr<class IAssetTypeActions>> CreatedAssetTypeActions;
};
