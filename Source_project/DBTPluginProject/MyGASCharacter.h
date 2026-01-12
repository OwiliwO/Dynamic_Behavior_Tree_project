// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilityCounterComponent.h"
#include "GameplayAbilitySpec.h"
#include "MyGASCharacter.generated.h"

class UMyAbilitySystemComponent;
class UMyAttributeSet;

UCLASS()
class DBTPLUGINPROJECT_API AMyGASCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
	AMyGASCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "GAS")
    void ActivatePrintAbility();

	UFUNCTION(BlueprintCallable, Category = "GAS")
	UMyAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// Show activate stats of character's abilities
	UFUNCTION(Exec)
    void ShowAbilityStats();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UMyAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityCounterComponent* AbilityCounterComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
    TSubclassOf<class UGameplayAbility> PrintAbilityClass;

	UPROPERTY()
	UMyAttributeSet* AttributeSet;

private:
	FGameplayAbilitySpecHandle PrintAbilityHandle;
};
