// Copyright Epic Games, Inc. All Rights Reserved.

#include "DBTAbilityBase.h"
#include "AbilityCounterComponent.h"
#include "AbilityCategoryUtils.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BTTaskNode.h"

#if WITH_EDITOR
#include "DynamicRootNodeCustomization.h"
#include "DynamicTaskNode.h"
#endif

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
        GLog->Logf(ELogVerbosity::Display,
            TEXT("DBTAbilityBase: %s (Category: %s) used %d times"),
            *GetClass()->GetName(),
            *UAbilityCategoryUtils::CategoryToText(ActionCategory).ToString(),
            UsageCount);
    }

    CheckBehaviorTreesUsage();
}

void UDBTAbilityBase::CheckBehaviorTreesUsage()
{
    TArray<UBehaviorTree*> AllBehaviorTrees;

    for (TObjectIterator<UBehaviorTree> It; It; ++It)
    {
        if (It->GetWorld() == GetWorld())
        {
            AllBehaviorTrees.Add(*It);
        }
    }

    FString AbilityCategoryString = UAbilityCategoryUtils::CategoryToText(ActionCategory).ToString();

    GLog->Logf(ELogVerbosity::Display, TEXT("DBTAbilityBase: Checking %d Behavior Trees for UsageCount %d, Ability Category: %s"),
        AllBehaviorTrees.Num(), UsageCount, *AbilityCategoryString);

    for (UBehaviorTree* BehaviorTree : AllBehaviorTrees)
    {
        if (!BehaviorTree || !BehaviorTree->RootNode)
            continue;

        CheckCompositeNodeForLimit(BehaviorTree->RootNode, UsageCount, AbilityCategoryString, BehaviorTree->GetName());
    }
}

void UDBTAbilityBase::CheckCompositeNodeForLimit(UBTCompositeNode* CompositeNode, int32 CurrentUsageCount,
    const FString& AbilityCategory, const FString& TreeName)
{
    if (!CompositeNode)
        return;

#if WITH_EDITOR
    FObjectKey NodeKey(CompositeNode);
    const TMap<FObjectKey, int32>& RootLimitChangeMap = FBehaviorTreeRootNodeCustomization::GetRootLimitChangeMap();
    const int32* LimitChangePtr = RootLimitChangeMap.Find(NodeKey);

    if (LimitChangePtr)
    {
        int32 LimitChange = *LimitChangePtr;

        GLog->Logf(ELogVerbosity::Display,
            TEXT("DBTAbilityBase: Tree '%s' - Node '%s' has LimitChange: %d, Current UsageCount: %d, Ability Category: %s"),
            *TreeName, *CompositeNode->GetName(), LimitChange, CurrentUsageCount, *AbilityCategory);

        if (CurrentUsageCount >= LimitChange && LimitChange > 0)
        {
            bool bHasMatchingCategory = CheckChildNodesCategory(CompositeNode, AbilityCategory, TreeName);

            if (bHasMatchingCategory)
            {
                GLog->Logf(ELogVerbosity::Warning,
                    TEXT("DBTAbilityBase: USAGE LIMIT REACHED with matching category! Tree '%s' - Node '%s': ")
                    TEXT("UsageCount %d >= LimitChange %d, Category: %s"),
                    *TreeName, *CompositeNode->GetName(), CurrentUsageCount, LimitChange, *AbilityCategory);

                //ChangeNodePriority(CompositeNode, AbilityCategory, TreeName);
            }
            else
            {
                GLog->Logf(ELogVerbosity::Display,
                    TEXT("DBTAbilityBase: Usage limit reached but category doesn't match for any child node. Ability Category: %s"),
                    *AbilityCategory);
            }
        }
    }
#endif

    for (const FBTCompositeChild& Child : CompositeNode->Children)
    {
        if (Child.ChildComposite)
        {
            CheckCompositeNodeForLimit(Child.ChildComposite, CurrentUsageCount, AbilityCategory, TreeName);
        }
    }
}

bool UDBTAbilityBase::CheckChildNodesCategory(UBTCompositeNode* CompositeNode, const FString& AbilityCategory, const FString& TreeName)
{
    bool bFoundMatchingCategory = false;

    for (const FBTCompositeChild& Child : CompositeNode->Children)
    {
        if (Child.ChildTask)
        {
            FString TaskCategory = GetTaskNodeCategory(Child.ChildTask);

            if (!TaskCategory.IsEmpty())
            {
                GLog->Logf(ELogVerbosity::Verbose,
                    TEXT("DBTAbilityBase: Tree '%s' - Task '%s' has category: %s, Ability category: %s"),
                    *TreeName, *Child.ChildTask->GetName(), *TaskCategory, *AbilityCategory);

                if (TaskCategory.Equals(AbilityCategory, ESearchCase::IgnoreCase))
                {
                    bFoundMatchingCategory = true;
                    GLog->Logf(ELogVerbosity::Display,
                        TEXT("DBTAbilityBase: Category match found! Task '%s' category '%s' matches ability category"),
                        *Child.ChildTask->GetName(), *TaskCategory);
                }
            }
        }
    }

    return bFoundMatchingCategory;
}

FString UDBTAbilityBase::GetTaskNodeCategory(UBTTaskNode* TaskNode)
{
    if (!TaskNode)
        return FString();

#if WITH_EDITOR
    FObjectKey NodeKey(TaskNode);
    const TMap<FObjectKey, TSharedPtr<FString>>& DynamicBehaviorCategoriesMap = FTaskNodeCustomization::GetDynamicBehaviorCategoriesMap();
    const TSharedPtr<FString>* CategoryPtr = DynamicBehaviorCategoriesMap.Find(NodeKey);

    if (CategoryPtr && CategoryPtr->IsValid())
    {
        return **CategoryPtr;
    }
#endif

    return FString();
}

void UDBTAbilityBase::CheckTaskNodeForUsage(UBTTaskNode* TaskNode, int32 CurrentUsageCount, const FString& TreeName)
{
    if (!TaskNode)
        return;

    FString TaskCategory = GetTaskNodeCategory(TaskNode);

    GLog->Logf(ELogVerbosity::Verbose,
        TEXT("DBTAbilityBase: Tree '%s' - Task '%s' (Category: %s) checked with UsageCount: %d"),
        *TreeName, *TaskNode->GetName(), *TaskCategory, CurrentUsageCount);
}