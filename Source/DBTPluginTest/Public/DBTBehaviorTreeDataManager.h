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
    void SetTaskNodeDynamicData(UObject* TaskNode, bool bIsDynamic, const FString& Category);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dynamic Behavior Tree")
    bool GetTaskNodeIsDynamic(UObject* TaskNode) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dynamic Behavior Tree")
    FString GetTaskNodeCategory(UObject* TaskNode) const;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior Tree")
	void SetAIControllerDynamicBehaviorFlag(UObject* AIController, bool bFlag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dynamic Behavior Tree")
	bool GetAIControllerDynamicBehaviorFlag(UObject* AIController) const;

	UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior Tree")
	void SetAIControllerTimeLimit(UObject* AIController, int32 TimeLimit);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dynamic Behavior Tree")
	int32 GetAIControllerTimeLimit(UObject* AIController) const;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior Tree")
    void ClearAllData();

protected:
    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, int32> NodeDataMap;
    
    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, bool> TaskNodeDynamicFlagsMap;
    
    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, FString> TaskNodeCategoriesMap;

    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, bool> AIControllerDynamicBehaviorFlags;
    
    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, int32> AIControllerTimeLimits;

    static UDBTBehaviorTreeDataManager* Instance;
};