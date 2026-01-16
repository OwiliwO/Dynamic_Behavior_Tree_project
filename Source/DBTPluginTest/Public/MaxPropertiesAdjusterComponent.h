// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MaxPropertiesAdjusterComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DBTPLUGINTEST_API UMaxPropertiesAdjusterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMaxPropertiesAdjusterComponent();

    UFUNCTION(BlueprintCallable, Category = "Max Properties")
    void StartAdjustmentTimer();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void ExecuteAdjustment();

    UFUNCTION()
    void ExecuteAdjustmentLogic();

private:
    FTimerHandle AdjustmentTimerHandle;
};