// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MyGamesTypes.generated.h"

USTRUCT(BlueprintType)
struct FMyCharacterAttributeDefaults : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Mana = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxMana = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeed = 600.0f;
};