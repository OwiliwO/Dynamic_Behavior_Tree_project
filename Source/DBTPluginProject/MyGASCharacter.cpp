// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyGASCharacter.h"
#include "MyAbilitySystemComponent.h"
#include "MyGameplayAbility_PrintMessage.h"
#include "MyAttributeSet.h"

AMyGASCharacter::AMyGASCharacter()
{
    AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
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
        // We'll initialize attributes through a GameplayEffect later

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