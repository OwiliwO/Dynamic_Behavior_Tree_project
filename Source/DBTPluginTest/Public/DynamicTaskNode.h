// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"
#include "Misc/Guid.h"

class FTaskNodeCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	static void ClearFlagsMap() {
		DynamicBehaviorFlagsMap.Empty();
		DynamicBehaviorCategoriesMap.Empty();
	}

	static const TMap<FObjectKey, TSharedPtr<FString>>& GetDynamicBehaviorCategoriesMap() { return DynamicBehaviorCategoriesMap; }
private:
	static TMap<FObjectKey, bool> DynamicBehaviorFlagsMap;
	static TMap<FObjectKey, TSharedPtr<FString>> DynamicBehaviorCategoriesMap;

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;

	static TArray<TSharedPtr<FString>> CategoryOptions;
};
#endif