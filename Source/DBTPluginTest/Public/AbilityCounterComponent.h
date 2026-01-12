// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityCounterComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityUsedSignature, const FString&, AbilityName, int32, UsageCount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DBTPLUGINTEST_API UAbilityCounterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAbilityCounterComponent();

    UPROPERTY(BlueprintAssignable, Category = "Ability Counter")
    FOnAbilityUsedSignature OnAbilityUsed;

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    int32 GetAbilityUsageCount(TSubclassOf<class UGameplayAbility> AbilityClass) const;

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    int32 GetAbilityUsageCountByName(const FString& AbilityClassName) const;

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    TMap<FString, int32> GetAllAbilityUsageStats() const;

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    void ResetAllCounters();

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    void PrintStats() const;

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    void DisplayStatsOnScreen(float Duration = 5.0f) const;

    UFUNCTION(Exec)
    void ShowAbilityStats();

    UFUNCTION(BlueprintCallable, Category = "Ability Counter")
    void IncrementAbilityCounter(const FString& AbilityName);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TMap<FString, int32> AbilityUsageMap;
};