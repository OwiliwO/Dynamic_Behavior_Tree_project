// Copyright Epic Games, Inc. All Rights Reserved.

#include "DBTBehaviorTreeDataManager.h"
#include "Engine/Engine.h"

UDBTBehaviorTreeDataManager* UDBTBehaviorTreeDataManager::Instance = nullptr;

UDBTBehaviorTreeDataManager& UDBTBehaviorTreeDataManager::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDBTBehaviorTreeDataManager>();
        Instance->AddToRoot();
        GLog->Logf(ELogVerbosity::Display, TEXT("DBTBehaviorTreeDataManager created"));
    }
    return *Instance;
}

void UDBTBehaviorTreeDataManager::Release()
{
    if (Instance)
    {
        Instance->ClearAllData();
        Instance->RemoveFromRoot();
        Instance = nullptr;
        GLog->Logf(ELogVerbosity::Display, TEXT("DBTBehaviorTreeDataManager released"));
    }
}

void UDBTBehaviorTreeDataManager::SetLimitChangeForNode(UObject* Node, int32 LimitChange)
{
    if (Node)
    {
        NodeDataMap.Add(Node, LimitChange);
        GLog->Logf(ELogVerbosity::Display, TEXT("Set LimitChange for node %s: %d"), *Node->GetName(), LimitChange);
    }
}

int32 UDBTBehaviorTreeDataManager::GetLimitChangeForNode(UObject* Node) const
{
    if (!Node)
    {
        return 0;
    }

    const int32* ValuePtr = NodeDataMap.Find(Node);

    if (ValuePtr && Node->IsValidLowLevel())
    {
        return *ValuePtr;
    }

    return 0;
}

bool UDBTBehaviorTreeDataManager::HasLimitChangeForNode(UObject* Node) const
{
    if (!Node)
    {
        return false;
    }

    const int32* ValuePtr = NodeDataMap.Find(Node);
    return ValuePtr != nullptr && Node->IsValidLowLevel();
}

void UDBTBehaviorTreeDataManager::SetTaskNodeDynamicData(UObject* TaskNode, bool bIsDynamic, const FString& Category)
{
    if (TaskNode)
    {
        TaskNodeDynamicFlagsMap.Add(TaskNode, bIsDynamic);
        TaskNodeCategoriesMap.Add(TaskNode, Category);

        GLog->Logf(ELogVerbosity::Display, TEXT("Set Dynamic Data for TaskNode %s: IsDynamic=%s, Category=%s"), *TaskNode->GetName(), bIsDynamic ? TEXT("True") : TEXT("False"), *Category);
    }
}

bool UDBTBehaviorTreeDataManager::GetTaskNodeIsDynamic(UObject* TaskNode) const
{
    if (!TaskNode) return false;

    const bool* ValuePtr = TaskNodeDynamicFlagsMap.Find(TaskNode);
    return ValuePtr && TaskNode->IsValidLowLevel() ? *ValuePtr : false;
}

FString UDBTBehaviorTreeDataManager::GetTaskNodeCategory(UObject* TaskNode) const
{
    if (!TaskNode) return FString();

    const FString* ValuePtr = TaskNodeCategoriesMap.Find(TaskNode);
    return ValuePtr && TaskNode->IsValidLowLevel() ? *ValuePtr : FString();
}

void UDBTBehaviorTreeDataManager::SetAIControllerDynamicBehaviorFlag(UObject* AIController, bool bFlag)
{
    if (AIController && AIController->IsA<AAIController>())
    {
        AIControllerDynamicBehaviorFlags.Add(AIController, bFlag);
        GLog->Logf(ELogVerbosity::Display, TEXT("Set DynamicBehaviorFlag for AI Controller %s: %s"), *AIController->GetName(), bFlag ? TEXT("True") : TEXT("False"));
    }
}

bool UDBTBehaviorTreeDataManager::GetAIControllerDynamicBehaviorFlag(UObject* AIController) const
{
    if (!AIController || !AIController->IsA<AAIController>())
    {
        return false;
    }

    const bool* ValuePtr = AIControllerDynamicBehaviorFlags.Find(AIController);
    return ValuePtr && AIController->IsValidLowLevel() ? *ValuePtr : false;
}

void UDBTBehaviorTreeDataManager::SetAIControllerTimeLimit(UObject* AIController, int32 TimeLimit)
{
    if (AIController && AIController->IsA<AAIController>())
    {
        AIControllerTimeLimits.Add(AIController, TimeLimit);
        GLog->Logf(ELogVerbosity::Display, TEXT("Set TimeLimit for AI Controller %s: %d"),
            *AIController->GetName(), TimeLimit);
    }
}

int32 UDBTBehaviorTreeDataManager::GetAIControllerTimeLimit(UObject* AIController) const
{
    if (!AIController || !AIController->IsA<AAIController>())
    {
        return 0;
    }

    const int32* ValuePtr = AIControllerTimeLimits.Find(AIController);
    return ValuePtr && AIController->IsValidLowLevel() ? *ValuePtr : 0;
}

void UDBTBehaviorTreeDataManager::SetGlobalAdjustmentDelay(float DelaySeconds)
{
    GlobalAdjustmentDelay = DelaySeconds;
    GLog->Logf(ELogVerbosity::Display, TEXT("DBTBehaviorTreeDataManager: Global adjustment delay set to: %.1f seconds"), DelaySeconds);
}

float UDBTBehaviorTreeDataManager::GetGlobalAdjustmentDelay() const
{
    return GlobalAdjustmentDelay;
}

bool UDBTBehaviorTreeDataManager::IsAnyAIControllerDynamicBehaviorEnabled() const
{
    for (const auto& Pair : AIControllerDynamicBehaviorFlags)
    {
        if (Pair.Key.IsValid() && Pair.Value)
        {
            return true;
        }
    }
    return false;
}

void UDBTBehaviorTreeDataManager::ClearAllData()
{
    NodeDataMap.Empty();
    GLog->Logf(ELogVerbosity::Display, TEXT("DBTBehaviorTreeDataManager: All data cleared"));
}