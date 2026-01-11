// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MyGameplayAbility_PrintMessage.generated.h"

UCLASS()
class DBTPLUGINPROJECT_API UMyGameplayAbility_PrintMessage : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMyGameplayAbility_PrintMessage();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Print Ability")
    FString MessageToPrint = TEXT("Ability Activated!");
};
