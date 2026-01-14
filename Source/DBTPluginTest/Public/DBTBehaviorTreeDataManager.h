// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DBTBehaviorTreeDataManager.generated.h"

UCLASS(BlueprintType)
class DBTPLUGINTEST_API UDBTBehaviorTreeDataManager : public UObject
{
    GENERATED_BODY()

public:
    static UDBTBehaviorTreeDataManager& Get();
    
    static void Release();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior Tree")
    void SetLimitChangeForNode(UObject* Node, int32 LimitChange);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dynamic Behavior Tree")
    int32 GetLimitChangeForNode(UObject* Node) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dynamic Behavior Tree")
    bool HasLimitChangeForNode(UObject* Node) const;
    
    UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior Tree")
    void ClearAllData();

protected:
    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, int32> NodeDataMap;
    
    static UDBTBehaviorTreeDataManager* Instance;
};