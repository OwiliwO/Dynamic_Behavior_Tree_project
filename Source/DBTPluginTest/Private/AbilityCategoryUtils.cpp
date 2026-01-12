// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityCategoryUtils.h"
#include "Engine/Engine.h"

FText UAbilityCategoryUtils::GetOppositeCategoryText(const FText& CategoryText)
{
    FString CategoryString = CategoryText.ToString();

    if (CategoryString.Equals(TEXT("Offensive Action"), ESearchCase::IgnoreCase))
    {
        return FText::FromString(TEXT("Defensive Action"));
    }
    else if (CategoryString.Equals(TEXT("Defensive Action"), ESearchCase::IgnoreCase))
    {
        return FText::FromString(TEXT("Offensive Action"));
    }
    else if (CategoryString.Equals(TEXT("Supporting Action"), ESearchCase::IgnoreCase))
    {
        return FText::FromString(TEXT("Offensive Action"));
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("UAbilityCategoryUtils::GetOppositeCategoryText: Unknown category '%s', returning default"), *CategoryString);
    return FText::FromString(TEXT("Offensive Action"));
}

EAbilityCategory UAbilityCategoryUtils::GetOppositeCategory(EAbilityCategory Category)
{
    switch (Category)
    {
    case EAbilityCategory::OffensiveAction:
        return EAbilityCategory::DefensiveAction;
    case EAbilityCategory::DefensiveAction:
        return EAbilityCategory::OffensiveAction;
    case EAbilityCategory::SupportingAction:
        return EAbilityCategory::OffensiveAction;
    default:
        return EAbilityCategory::OffensiveAction;
    }
}

FText UAbilityCategoryUtils::CategoryToText(EAbilityCategory Category)
{
    switch (Category)
    {
    case EAbilityCategory::OffensiveAction:
        return FText::FromString(TEXT("Offensive Action"));
    case EAbilityCategory::DefensiveAction:
        return FText::FromString(TEXT("Defensive Action"));
    case EAbilityCategory::SupportingAction:
        return FText::FromString(TEXT("Supporting Action"));
    default:
        return FText::FromString(TEXT("Offensive Action"));
    }
}

EAbilityCategory UAbilityCategoryUtils::TextToCategory(const FText& CategoryText)
{
    FString CategoryString = CategoryText.ToString();

    if (CategoryString.Equals(TEXT("Offensive Action"), ESearchCase::IgnoreCase))
    {
        return EAbilityCategory::OffensiveAction;
    }
    else if (CategoryString.Equals(TEXT("Defensive Action"), ESearchCase::IgnoreCase))
    {
        return EAbilityCategory::DefensiveAction;
    }
    else if (CategoryString.Equals(TEXT("Supporting Action"), ESearchCase::IgnoreCase))
    {
        return EAbilityCategory::SupportingAction;
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("UAbilityCategoryUtils::TextToCategory: Unknown category text '%s', returning default"), *CategoryString);
    return EAbilityCategory::OffensiveAction;
}

TArray<FText> UAbilityCategoryUtils::GetAllCategoryOptions()
{
    TArray<FText> Categories;
    Categories.Add(FText::FromString(TEXT("Offensive Action")));
    Categories.Add(FText::FromString(TEXT("Defensive Action")));
    Categories.Add(FText::FromString(TEXT("Supporting Action")));
    return Categories;
}