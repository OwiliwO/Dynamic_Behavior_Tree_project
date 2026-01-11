// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyGameplayAbility_PrintMessage.h"
#include "AbilitySystemComponent.h"

UMyGameplayAbility_PrintMessage::UMyGameplayAbility_PrintMessage()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMyGameplayAbility_PrintMessage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (bHasBlueprintActivate)
    {
        Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("%s"), *MessageToPrint);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("Ability: %s"), *MessageToPrint));
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}