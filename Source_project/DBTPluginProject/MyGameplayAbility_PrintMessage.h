// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBTAbilityBase.h"
#include "MyGameplayAbility_PrintMessage.generated.h"

UCLASS()
// TTest ability that is inherited from the modified class
class DBTPLUGINPROJECT_API UMyGameplayAbility_PrintMessage : public UDBTAbilityBase
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
