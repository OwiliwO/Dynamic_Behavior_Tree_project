// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyGASCharacter.h"
#include "MyAbilitySystemComponent.h"
#include "MyGameplayAbility_PrintMessage.h"
#include "DBTAbilityBase.h"
#include "MyAttributeSet.h"

AMyGASCharacter::AMyGASCharacter()
{
    AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));

    // Automatically add the counter component from the plugin
    AbilityCounterComponent = CreateDefaultSubobject<UAbilityCounterComponent>(TEXT("AbilityCounterComponent"));
}

UAbilitySystemComponent* AMyGASCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AMyGASCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        if (PrintAbilityClass)
        {
            PrintAbilityHandle = AbilitySystemComponent->GiveAbility(
                FGameplayAbilitySpec(PrintAbilityClass, 1, 0));
        }
    }
}

void AMyGASCharacter::ActivatePrintAbility()
{
    if (AbilitySystemComponent && PrintAbilityHandle.IsValid())
    {
        AbilitySystemComponent->TryActivateAbility(PrintAbilityHandle);
    }
}

void AMyGASCharacter::ShowAbilityStats()
{
    if (AbilityCounterComponent)
    {
        AbilityCounterComponent->ShowAbilityStats();
    }
}