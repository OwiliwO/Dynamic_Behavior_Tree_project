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
        //Instance->MarkAsGarbage();
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

void UDBTBehaviorTreeDataManager::ClearAllData()
{
    NodeDataMap.Empty();
    GLog->Logf(ELogVerbosity::Display, TEXT("DBTBehaviorTreeDataManager: All data cleared"));
}