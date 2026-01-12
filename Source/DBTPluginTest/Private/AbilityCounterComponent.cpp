// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityCounterComponent.h"
#include "Engine/Engine.h"

UAbilityCounterComponent::UAbilityCounterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UAbilityCounterComponent::BeginPlay()
{
    Super::BeginPlay();
    GLog->Logf(ELogVerbosity::Display, TEXT("AbilityCounterComponent started for %s"), *GetOwner()->GetName());
}

int32 UAbilityCounterComponent::GetAbilityUsageCount(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!AbilityClass) return 0;

	FString AbilityName = AbilityClass->GetName();
	const int32* Count = AbilityUsageMap.Find(AbilityName);
	return Count ? *Count : 0;
}

int32 UAbilityCounterComponent::GetAbilityUsageCountByName(const FString& AbilityClassName) const
{
    const int32* Count = AbilityUsageMap.Find(AbilityClassName);
    return Count ? *Count : 0;
}

TMap<FString, int32> UAbilityCounterComponent::GetAllAbilityUsageStats() const
{
    return AbilityUsageMap;
}

void UAbilityCounterComponent::ResetAllCounters()
{
    AbilityUsageMap.Empty();
    GLog->Logf(ELogVerbosity::Display, TEXT("AbilityCounter: All counters reset for %s"), *GetOwner()->GetName());
}

void UAbilityCounterComponent::PrintStats() const
{
    GLog->Logf(ELogVerbosity::Display, TEXT("=== Ability Usage Stats for %s ==="), *GetOwner()->GetName());

    if (AbilityUsageMap.Num() == 0)
    {
        GLog->Logf(ELogVerbosity::Display, TEXT("No abilities used yet"));
    }
    else
    {
        TArray<TPair<FString, int32>> SortedStats;
        for (const auto& Pair : AbilityUsageMap)
        {
            SortedStats.Add(TPair<FString, int32>(Pair.Key, Pair.Value));
        }

        SortedStats.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
            return A.Value > B.Value;
            });

        for (const auto& Pair : SortedStats)
        {
            GLog->Logf(ELogVerbosity::Display, TEXT("  %s: %d uses"), *Pair.Key, Pair.Value);
        }
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("=== End Stats ==="));
}

void UAbilityCounterComponent::DisplayStatsOnScreen(float Duration) const
{
    if (GEngine)
    {
        FString StatsText = FString::Printf(TEXT("=== Ability Stats for %s ===\n"), *GetOwner()->GetName());

        if (AbilityUsageMap.Num() == 0)
        {
            StatsText += TEXT("No abilities used yet\n");
        }
        else
        {
            TArray<TPair<FString, int32>> SortedStats;
            for (const auto& Pair : AbilityUsageMap)
            {
                SortedStats.Add(TPair<FString, int32>(Pair.Key, Pair.Value));
            }

            SortedStats.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
                return A.Value > B.Value;
                });

            for (const auto& Pair : SortedStats)
            {
                StatsText += FString::Printf(TEXT("%s: %d uses\n"), *Pair.Key, Pair.Value);
            }
        }

        StatsText += TEXT("========================");

        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Green, StatsText);
    }
}

void UAbilityCounterComponent::ShowAbilityStats()
{
    PrintStats();
    DisplayStatsOnScreen();
}

void UAbilityCounterComponent::IncrementAbilityCounter(const FString& AbilityName)
{
    int32& Count = AbilityUsageMap.FindOrAdd(AbilityName);
    Count++;

    OnAbilityUsed.Broadcast(AbilityName, Count);

    GLog->Logf(ELogVerbosity::Display, TEXT("AbilityCounter: %s used %d times by %s"), *AbilityName, Count, *GetOwner()->GetName());
}