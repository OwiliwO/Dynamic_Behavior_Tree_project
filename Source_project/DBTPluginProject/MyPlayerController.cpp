// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyPlayerController.h"
#include "MyGASCharacter.h"

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("TestAbility", IE_Pressed, this, &AMyPlayerController::ActivateTestAbility);
    InputComponent->BindAction("ShowStats", IE_Pressed, this, &AMyPlayerController::ShowStats);
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();
    EnableInput(this);
}

void AMyPlayerController::ActivateTestAbility()
{
    AMyGASCharacter* MyCharacter = Cast<AMyGASCharacter>(GetPawn());
    if (MyCharacter)
    {
        MyCharacter->ActivatePrintAbility();
    }
}

void AMyPlayerController::ShowStats()
{
    AMyGASCharacter* MyCharacter = Cast<AMyGASCharacter>(GetPawn());
    if (MyCharacter)
    {
        MyCharacter->ShowAbilityStats();
    }
}
