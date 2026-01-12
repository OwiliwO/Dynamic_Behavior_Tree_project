// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityCategoryUtils.h"
#include "DBTAbilityBase.generated.h"

class UBTCompositeNode;
class UBTTaskNode;
class UBehaviorTreeComponent;

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
    void CheckBehaviorTreesUsage();

    void CheckCompositeNodeForLimit(UBTCompositeNode* CompositeNode, int32 CurrentUsageCount,
        const FString& AbilityCategory, const FString& TreeName);

    bool CheckChildNodesCategory(UBTCompositeNode* CompositeNode, const FString& AbilityCategory,
        const FString& TreeName);

    FString GetTaskNodeCategory(UBTTaskNode* TaskNode);

    /*void ChangeNodePriority(UBTCompositeNode* CompositeNode, const FString& Category,
        const FString& TreeName);*/

    void CheckTaskNodeForUsage(UBTTaskNode* TaskNode, int32 CurrentUsageCount,
        const FString& TreeName);
};