// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "AbilityCategoryUtils.generated.h"

UENUM(BlueprintType)
enum class EAbilityCategory : uint8
{
    OffensiveAction    UMETA(DisplayName = "Offensive Action"),
    DefensiveAction    UMETA(DisplayName = "Defensive Action"),
    SupportingAction   UMETA(DisplayName = "Supporting Action")
};

UCLASS()
class DBTPLUGINTEST_API UAbilityCategoryUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability Category", meta = (DisplayName = "Get Opposite Category (Text)"))
    static FText GetOppositeCategoryText(const FText& CategoryText);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability Category", meta = (DisplayName = "Get Opposite Category (Enum)"))
    static EAbilityCategory GetOppositeCategory(EAbilityCategory Category);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability Category")
    static FText CategoryToText(EAbilityCategory Category);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability Category")
    static EAbilityCategory TextToCategory(const FText& CategoryText);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability Category")
    static TArray<FText> GetAllCategoryOptions();
};