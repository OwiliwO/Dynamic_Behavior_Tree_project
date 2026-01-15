// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityCategoryUtils.h"
#include "DBTAbilityBase.generated.h"

class UBTCompositeNode;
class UBTTaskNode;
class UBehaviorTreeComponent;
//class AAIController;
//class AActor;

struct FTaskNodeInfo
{
    UBTTaskNode* TaskNode;
    UBTCompositeNode* ParentComposite;
    int32 ChildIndex;

    FTaskNodeInfo(UBTTaskNode* InTaskNode = nullptr, UBTCompositeNode* InParentComposite = nullptr, int32 InChildIndex = -1)
        : TaskNode(InTaskNode)
        , ParentComposite(InParentComposite)
        , ChildIndex(InChildIndex)
    {
    }
};

UCLASS(Abstract, Blueprintable)
class DBTPLUGINTEST_API UDBTAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UDBTAbilityBase();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dynamic Behavior")
    EAbilityCategory ActionCategory = EAbilityCategory::OffensiveAction;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior")
    FString GetActionCategoryString() const;

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override;

    UPROPERTY(BlueprintReadOnly, Category = "Ability Counter")
    int32 UsageCount = 0;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Behavior")
    void IncrementUsageCount();

private:
    void CheckAllBehaviorTreesOnAbilityUse();
    
    void CheckCompositeNodeRecursive(UBTCompositeNode* Node);

    bool CheckSingleCompositeNode(UBTCompositeNode* Node);

    void CollectCompositeNodeLimitChanges(UBTCompositeNode* Node, TArray<int32>& LimitChanges);

    void CheckForUsageCountReset(const TArray<int32>& LimitChanges);

    void GetAllTaskNodesFromComposite(UBTCompositeNode* Composite, TArray<class UBTTaskNode*>& OutTaskNodes);

    void GetAllTaskNodesWithInfo(UBTCompositeNode* Composite, TArray<struct FTaskNodeInfo>& OutTaskNodes);
    
    void SwapTaskNodePriorities(TArray<struct FTaskNodeInfo>& FirstArray, TArray<struct FTaskNodeInfo>& SecondArray);
};