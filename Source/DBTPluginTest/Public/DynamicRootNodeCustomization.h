// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"

class FBehaviorTreeRootNodeCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

    static const TMap<FObjectKey, int32>& GetRootLimitChangeMap() { return RootLimitChangeMap; }
private:
    TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
    static TMap<FObjectKey, int32> RootLimitChangeMap;
};