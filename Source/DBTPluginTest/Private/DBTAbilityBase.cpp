// Copyright Epic Games, Inc. All Rights Reserved.

#include "DBTAbilityBase.h"
#include "AbilityCounterComponent.h"
#include "AbilityCategoryUtils.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DBTBehaviorTreeDataManager.h"

#if WITH_EDITOR
#include "DynamicRootNodeCustomization.h"
#include "DynamicTaskNode.h"
#endif

void UDBTAbilityBase::SwapTaskNodePriorities(TArray<FTaskNodeInfo>& FirstArray, TArray<FTaskNodeInfo>& SecondArray)
{
    if (FirstArray.Num() != SecondArray.Num())
    {
        GLog->Logf(ELogVerbosity::Warning, TEXT("[PRIORITY SWAP] Arrays have different sizes: First=%d, Second=%d. Trimming..."), FirstArray.Num(), SecondArray.Num());

        while (FirstArray.Num() > SecondArray.Num())
        {
            FirstArray.RemoveAt(FirstArray.Num() - 1);
        }
        while (SecondArray.Num() > FirstArray.Num())
        {
            SecondArray.RemoveAt(SecondArray.Num() - 1);
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] After trimming: First=%d, Second=%d"), FirstArray.Num(), SecondArray.Num());
    }

    if (FirstArray.Num() == 0 || SecondArray.Num() == 0)
    {
        GLog->Logf(ELogVerbosity::Warning, TEXT("[PRIORITY SWAP] Cannot swap priorities: one or both arrays are empty"));
        return;
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== [PRIORITY SWAP] START LOG ==="));

    TMap<UBTCompositeNode*, TArray<int32>> CompositeToFirstIndices;
    TMap<UBTCompositeNode*, TArray<int32>> CompositeToSecondIndices;

    for (const FTaskNodeInfo& Info : FirstArray)
    {
        if (Info.ParentComposite)
        {
            CompositeToFirstIndices.FindOrAdd(Info.ParentComposite).Add(Info.ChildIndex);
        }
    }

    for (const FTaskNodeInfo& Info : SecondArray)
    {
        if (Info.ParentComposite)
        {
            CompositeToSecondIndices.FindOrAdd(Info.ParentComposite).Add(Info.ChildIndex);
        }
    }

    for (auto& Pair : CompositeToFirstIndices)
    {
        Pair.Value.Sort();
    }
    for (auto& Pair : CompositeToSecondIndices)
    {
        Pair.Value.Sort();
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== [PRIORITY SWAP] BEFORE SWAP ==="));

    for (auto& FirstPair : CompositeToFirstIndices)
    {
        UBTCompositeNode* Composite = FirstPair.Key;
        TArray<int32>& FirstIndices = FirstPair.Value;

        TArray<int32>* SecondIndicesPtr = CompositeToSecondIndices.Find(Composite);
        if (!SecondIndicesPtr || SecondIndicesPtr->Num() != FirstIndices.Num())
        {
            continue;
        }

        TArray<int32>& SecondIndices = *SecondIndicesPtr;

        GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] Composite %s current children order:"), *Composite->GetName());
        for (int32 i = 0; i < Composite->Children.Num(); i++)
        {
            const FBTCompositeChild& Child = Composite->Children[i];
            if (Child.ChildTask)
            {
                GLog->Logf(ELogVerbosity::Display, TEXT("[%d] Task: %s"), i, *Child.ChildTask->GetName());
            }
            else if (Child.ChildComposite)
            {
                GLog->Logf(ELogVerbosity::Display, TEXT("[%d] Composite: %s"), i, *Child.ChildComposite->GetName());
            }
            else
            {
                GLog->Logf(ELogVerbosity::Display, TEXT("[%d] Empty"), i);
            }
        }

        FString FirstIndicesStr, SecondIndicesStr;
        for (int32 Idx : FirstIndices)
        {
            FirstIndicesStr += FString::Printf(TEXT("%d, "), Idx);
        }
        for (int32 Idx : SecondIndices)
        {
            SecondIndicesStr += FString::Printf(TEXT("%d, "), Idx);
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] Will swap indices: [%s] <-> [%s]"), *FirstIndicesStr, *SecondIndicesStr);

        for (int32 i = 0; i < FirstIndices.Num(); i++)
        {
            int32 FirstIdx = FirstIndices[i];
            int32 SecondIdx = SecondIndices[i];

            if (FirstIdx >= 0 && FirstIdx < Composite->Children.Num() &&
                SecondIdx >= 0 && SecondIdx < Composite->Children.Num())
            {
                UBTTaskNode* FirstTask = Composite->Children[FirstIdx].ChildTask;
                UBTTaskNode* SecondTask = Composite->Children[SecondIdx].ChildTask;

                if (FirstTask && SecondTask)
                {
                    GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] Pair %d: %s (idx %d) <-> %s (idx %d)"), i, *FirstTask->GetName(), FirstIdx, *SecondTask->GetName(), SecondIdx);
                }
            }
        }
    }

    for (auto& FirstPair : CompositeToFirstIndices)
    {
        UBTCompositeNode* Composite = FirstPair.Key;
        TArray<int32>& FirstIndices = FirstPair.Value;

        TArray<int32>* SecondIndicesPtr = CompositeToSecondIndices.Find(Composite);
        if (!SecondIndicesPtr || SecondIndicesPtr->Num() != FirstIndices.Num())
        {
            continue;
        }

        TArray<int32>& SecondIndices = *SecondIndicesPtr;

        TArray<FBTCompositeChild> TempChildren = Composite->Children;

        for (int32 i = 0; i < FirstIndices.Num(); i++)
        {
            int32 FirstIdx = FirstIndices[i];
            int32 SecondIdx = SecondIndices[i];

            if (FirstIdx >= 0 && FirstIdx < TempChildren.Num() &&
                SecondIdx >= 0 && SecondIdx < TempChildren.Num())
            {
                UBTTaskNode* FirstTask = TempChildren[FirstIdx].ChildTask;
                UBTTaskNode* SecondTask = TempChildren[SecondIdx].ChildTask;

                UBTTaskNode* TempTask = TempChildren[FirstIdx].ChildTask;
                TempChildren[FirstIdx].ChildTask = TempChildren[SecondIdx].ChildTask;
                TempChildren[SecondIdx].ChildTask = TempTask;
            }
        }

        Composite->Children = TempChildren;
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== [PRIORITY SWAP] AFTER SWAP ==="));

    for (auto& FirstPair : CompositeToFirstIndices)
    {
        UBTCompositeNode* Composite = FirstPair.Key;
        TArray<int32>& FirstIndices = FirstPair.Value;

        TArray<int32>* SecondIndicesPtr = CompositeToSecondIndices.Find(Composite);
        if (!SecondIndicesPtr || SecondIndicesPtr->Num() != FirstIndices.Num())
        {
            continue;
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] Composite %s new children order:"), *Composite->GetName());
        for (int32 i = 0; i < Composite->Children.Num(); i++)
        {
            const FBTCompositeChild& Child = Composite->Children[i];
            if (Child.ChildTask)
            {
                GLog->Logf(ELogVerbosity::Display, TEXT("[%d] Task: %s"), i, *Child.ChildTask->GetName());
            }
            else if (Child.ChildComposite)
            {
                GLog->Logf(ELogVerbosity::Display, TEXT("[%d] Composite: %s"), i, *Child.ChildComposite->GetName());
            }
            else
            {
                GLog->Logf(ELogVerbosity::Display, TEXT("[%d] Empty"), i);
            }
        }

        TArray<int32>& SecondIndices = *SecondIndicesPtr;
        int32 SuccessfulSwaps = 0;
        for (int32 i = 0; i < FirstIndices.Num(); i++)
        {
            int32 FirstIdx = FirstIndices[i];
            int32 SecondIdx = SecondIndices[i];

            if (FirstIdx >= 0 && FirstIdx < Composite->Children.Num() &&
                SecondIdx >= 0 && SecondIdx < Composite->Children.Num())
            {
                SuccessfulSwaps++;
            }
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] Composite %s: %d successful swaps out of %d attempted"), *Composite->GetName(), SuccessfulSwaps, FirstIndices.Num());
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== [PRIORITY SWAP] END LOG ==="));
}

UDBTAbilityBase::UDBTAbilityBase()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

FString UDBTAbilityBase::GetActionCategoryString() const
{
    return UAbilityCategoryUtils::CategoryToText(ActionCategory).ToString();
}

void UDBTAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    IncrementUsageCount();
}

void UDBTAbilityBase::IncrementUsageCount()
{
    UsageCount++;

    if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
    {
        AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
        if (UAbilityCounterComponent* Counter = AvatarActor->FindComponentByClass<UAbilityCounterComponent>())
        {
            Counter->IncrementAbilityCounter(GetClass()->GetName());

            FText CurrentCategory = UAbilityCategoryUtils::CategoryToText(ActionCategory);
            FText OppositeCategory = UAbilityCategoryUtils::GetOppositeCategoryText(CurrentCategory);

            GLog->Logf(ELogVerbosity::Display, TEXT("DBTAbilityBase: %s (Category: %s) used %d times"), *GetClass()->GetName(), *CurrentCategory.ToString(), UsageCount);

            GLog->Logf(ELogVerbosity::Display, TEXT("DBTAbilityBase: Opposite category: %s"), *OppositeCategory.ToString());
        }
    }
    else
    {
        GLog->Logf(ELogVerbosity::Display, TEXT("DBTAbilityBase: %s (Category: %s) used %d times"), *GetClass()->GetName(), *UAbilityCategoryUtils::CategoryToText(ActionCategory).ToString(), UsageCount);
    }

    CheckAllBehaviorTreesOnAbilityUse();
}

void UDBTAbilityBase::CheckAllBehaviorTreesOnAbilityUse()
{
    if (!GetWorld())
    {
        GLog->Logf(ELogVerbosity::Display, TEXT("DBTAbilityBase: No world found for ability check"));
        return;
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== Starting Behavior Tree Check for Ability: %s ==="), *GetClass()->GetName());

    TArray<int32> FoundLimitChanges;

    for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
    {
        AAIController* AIController = *It;
        if (!AIController || !AIController->IsValidLowLevel())
        {
            continue;
        }

        UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(
            AIController->GetBrainComponent());

        if (!BTComponent)
        {
            continue;
        }

        UBehaviorTree* BehaviorTree = BTComponent->GetCurrentTree();
        if (!BehaviorTree || !BehaviorTree->RootNode)
        {
            continue;
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("Checking AI Controller: %s, Behavior Tree: %s"), *AIController->GetName(), *BehaviorTree->GetName());

        CheckCompositeNodeRecursive(BehaviorTree->RootNode);

        CollectCompositeNodeLimitChanges(BehaviorTree->RootNode, FoundLimitChanges);
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== Finished Behavior Tree Check ==="));

    CheckForUsageCountReset(FoundLimitChanges);
}

void UDBTAbilityBase::CollectCompositeNodeLimitChanges(UBTCompositeNode* Node, TArray<int32>& LimitChanges)
{
    if (!Node)
    {
        return;
    }

    UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();
    int32 LimitChange = DataManager.GetLimitChangeForNode(Node);

    if (LimitChange > 0)
    {
        LimitChanges.Add(LimitChange);
        GLog->Logf(ELogVerbosity::Display, TEXT("[LIMIT COLLECTION] Found LimitChange: %d for node: %s"),
            LimitChange, *Node->GetName());
    }

    for (const FBTCompositeChild& Child : Node->Children)
    {
        if (Child.ChildComposite)
        {
            CollectCompositeNodeLimitChanges(Child.ChildComposite, LimitChanges);
        }
    }
}

void UDBTAbilityBase::CheckForUsageCountReset(const TArray<int32>& LimitChanges)
{
    if (LimitChanges.Num() == 0)
    {
        GLog->Logf(ELogVerbosity::Display, TEXT("[RESET CHECK] No LimitChanges found. Skipping reset check."));
        return;
    }

    int32 MaxLimitChange = LimitChanges[0];
    for (int32 i = 1; i < LimitChanges.Num(); i++)
    {
        if (LimitChanges[i] > MaxLimitChange)
        {
            MaxLimitChange = LimitChanges[i];
        }
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("[RESET CHECK] Max LimitChange: %d, Current UsageCount: %d"), MaxLimitChange, UsageCount);

    if (UsageCount == MaxLimitChange + 1)
    {
        FString AbilityOwnerName = TEXT("Unknown");
        if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
        {
            AbilityOwnerName = CurrentActorInfo->AvatarActor->GetName();
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("[RESET TRIGGERED] UsageCount (%d) equals MaxLimitChange (%d)!"), UsageCount, MaxLimitChange);

        GLog->Logf(ELogVerbosity::Display, TEXT("[RESET TRIGGERED] Ability: %s, Owner: %s"), *GetClass()->GetName(), *AbilityOwnerName);

        int32 OldUsageCount = UsageCount;
        UsageCount = 0;

        if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
        {
            AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
            if (UAbilityCounterComponent* Counter = AvatarActor->FindComponentByClass<UAbilityCounterComponent>())
            {
                Counter->ResetAllCounters();
            }
        }

        GLog->Logf(ELogVerbosity::Display, TEXT("[RESET COMPLETE] UsageCount reset from %d to %d"), OldUsageCount, UsageCount);
    }
    else
    {
        GLog->Logf(ELogVerbosity::Display, TEXT("[RESET CHECK] Reset condition not met."));
    }
}

void UDBTAbilityBase::CheckCompositeNodeRecursive(UBTCompositeNode* Node)
{
    if (!Node)
    {
        return;
    }

    CheckSingleCompositeNode(Node);

    for (const FBTCompositeChild& Child : Node->Children)
    {
        if (Child.ChildComposite)
        {
            CheckCompositeNodeRecursive(Child.ChildComposite);
        }
    }
}

bool UDBTAbilityBase::CheckSingleCompositeNode(UBTCompositeNode* Node)
{
    if (!Node)
    {
        return false;
    }

    UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();
    int32 LimitChange = DataManager.GetLimitChangeForNode(Node);

    if (LimitChange > 0)
    {
        bool bConditionMet = (LimitChange >= UsageCount);

        if (!bConditionMet)
        {
            FString AbilityOwnerName = TEXT("Unknown");
            if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
            {
                AbilityOwnerName = CurrentActorInfo->AvatarActor->GetName();
            }

            GLog->Logf(ELogVerbosity::Display, TEXT("[BEHAVIOR TREE CHECK] Condition MET! Ability: %s, Usage: %d, LimitChange: %d"), *GetClass()->GetName(), UsageCount, LimitChange);

            TArray<FTaskNodeInfo> AllTaskNodesInfo;
            GetAllTaskNodesWithInfo(Node, AllTaskNodesInfo);

            TArray<FTaskNodeInfo> MatchingTaskNodes;
            TArray<FTaskNodeInfo> MatchingTaskNodesDiff;

            FText CurrentCategoryText = UAbilityCategoryUtils::CategoryToText(ActionCategory);
            FString CurrentCategoryString = CurrentCategoryText.ToString();

            for (const FTaskNodeInfo& TaskNodeInfo : AllTaskNodesInfo)
            {
                if (TaskNodeInfo.TaskNode && DataManager.GetTaskNodeIsDynamic(TaskNodeInfo.TaskNode))
                {
                    FString TaskCategory = DataManager.GetTaskNodeCategory(TaskNodeInfo.TaskNode);
                    FText TaskCategoryDiff = UAbilityCategoryUtils::GetOppositeCategoryText(CurrentCategoryText);
                    FString TaskCategoryDiffString = TaskCategoryDiff.ToString();
                    if (TaskCategory.Equals(CurrentCategoryString, ESearchCase::IgnoreCase))
                    {
                        MatchingTaskNodes.Add(TaskNodeInfo);
                        GLog->Logf(ELogVerbosity::Display, TEXT("[TASK NODE MATCH] Found matching Task Node: %s (Category: %s)"), *TaskNodeInfo.TaskNode->GetName(), *TaskCategory);
                    }
                    if (TaskCategory.Equals(TaskCategoryDiffString, ESearchCase::IgnoreCase))
                    {
                        MatchingTaskNodesDiff.Add(TaskNodeInfo);
                        GLog->Logf(ELogVerbosity::Display, TEXT("[TASK NODE DIFF MATCH] Found matching Task Node: %s (Category: %s)"),*TaskNodeInfo.TaskNode->GetName(), *TaskCategory);
                    }
                }
            }

            GLog->Logf(ELogVerbosity::Display, TEXT("[TASK NODE COLLECTION] Matching: %d, Diff: %d"), MatchingTaskNodes.Num(), MatchingTaskNodesDiff.Num());

            SwapTaskNodePriorities(MatchingTaskNodes, MatchingTaskNodesDiff);

            GLog->Logf(ELogVerbosity::Display, TEXT("[PRIORITY SWAP] Task node priorities have been swapped for ability: %s"), *GetClass()->GetName());

            return true;
        }
        else
        {
            GLog->Logf(ELogVerbosity::Display, TEXT("[BEHAVIOR TREE CHECK] Condition IS waiting. Ability: %s, Usage: %d, LimitChange: %d, Node: %s"), *GetClass()->GetName(), UsageCount, LimitChange, *Node->GetName());
        }
    }

    return false;
}

void UDBTAbilityBase::GetAllTaskNodesWithInfo(UBTCompositeNode* Composite, TArray<FTaskNodeInfo>& OutTaskNodes)
{
    if (!Composite) return;

    for (int32 i = 0; i < Composite->Children.Num(); ++i)
    {
        const FBTCompositeChild& Child = Composite->Children[i];

        if (Child.ChildTask)
        {
            OutTaskNodes.Add(FTaskNodeInfo(Child.ChildTask, Composite, i));
        }

        if (Child.ChildComposite)
        {
            GetAllTaskNodesWithInfo(Child.ChildComposite, OutTaskNodes);
        }
    }
}

void UDBTAbilityBase::GetAllTaskNodesFromComposite(UBTCompositeNode* Composite, TArray<UBTTaskNode*>& OutTaskNodes)
{
    if (!Composite) return;

    for (const FBTCompositeChild& Child : Composite->Children)
    {
        if (Child.ChildTask)
        {
            OutTaskNodes.Add(Child.ChildTask);
        }

        if (Child.ChildComposite)
        {
            GetAllTaskNodesFromComposite(Child.ChildComposite, OutTaskNodes);
        }
    }
}