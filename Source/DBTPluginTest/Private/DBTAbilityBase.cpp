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

    if (UsageCount == MaxLimitChange)
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

        if (bConditionMet)
        {
            FString AbilityOwnerName = TEXT("Unknown");
            if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
            {
                AbilityOwnerName = CurrentActorInfo->AvatarActor->GetName();
            }

            GLog->Logf(ELogVerbosity::Display, TEXT("[BEHAVIOR TREE CHECK] Condition MET! Ability: %s, Usage: %d, LimitChange: %d"), *GetClass()->GetName(), UsageCount, LimitChange);

            return true;
        }
        else
        {
            GLog->Logf(ELogVerbosity::Display, TEXT("[BEHAVIOR TREE CHECK] Condition NOT met. Ability: %s, Usage: %d, LimitChange: %d, Node: %s"), *GetClass()->GetName(), UsageCount, LimitChange, *Node->GetName());
        }
    }

    return false;
}