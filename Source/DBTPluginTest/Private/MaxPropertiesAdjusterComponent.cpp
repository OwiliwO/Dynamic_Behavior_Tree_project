// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaxPropertiesAdjusterComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"
#include "DBTBehaviorTreeDataManager.h"

UMaxPropertiesAdjusterComponent::UMaxPropertiesAdjusterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMaxPropertiesAdjusterComponent::BeginPlay()
{
	Super::BeginPlay();

	StartAdjustmentTimer();
}

void UMaxPropertiesAdjusterComponent::StartAdjustmentTimer()
{
	if (GetWorld())
	{
		UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();
		float DelaySeconds = DataManager.GetGlobalAdjustmentDelay();

		if (!DataManager.IsAnyAIControllerDynamicBehaviorEnabled()) 
		{ 
			return;
		}

		if (DelaySeconds <= 0.0f)
		{
			DelaySeconds = 5.0f;
		}

		GetWorld()->GetTimerManager().SetTimer(
			AdjustmentTimerHandle,
			this,
			&UMaxPropertiesAdjusterComponent::ExecuteAdjustment,
			DelaySeconds,
			false
		);

		GLog->Logf(ELogVerbosity::Display, TEXT("MaxPropertiesAdjusterComponent: Adjustment scheduled in %.1f seconds (from global delay)"), DelaySeconds);
	}
}

void UMaxPropertiesAdjusterComponent::ExecuteAdjustment()
{
	GLog->Logf(ELogVerbosity::Display, TEXT("MaxPropertiesAdjusterComponent: Starting adjustment..."));
	ExecuteAdjustmentLogic();

	// StartAdjustmentTimer();
}

void UMaxPropertiesAdjusterComponent::ExecuteAdjustmentLogic()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        GLog->Logf(ELogVerbosity::Warning, TEXT("ExecuteAdjustment: Cannot get World"));
        return;
    }

    TArray<APlayerController*> PlayerControllers;
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PlayerController = Iterator->Get();
        if (PlayerController)
        {
            PlayerControllers.Add(PlayerController);
        }
    }

    if (PlayerControllers.Num() == 0)
    {
        GLog->Logf(ELogVerbosity::Warning, TEXT("ExecuteAdjustment: No PlayerControllers found in world"));
        return;
    }

    GLog->Logf(ELogVerbosity::Display, TEXT("ExecuteAdjustment: Found %d PlayerController(s)"), PlayerControllers.Num());

    FString ConfigFilePath = FPaths::ProjectConfigDir() / TEXT("AIControllerMaxValues.json");

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ConfigFilePath))
	{
		GLog->Logf(ELogVerbosity::Warning, TEXT("AdjustMaxPropertiesFromConfig: Could not load config file %s"), *ConfigFilePath);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		GLog->Logf(ELogVerbosity::Warning, TEXT("AdjustMaxPropertiesFromConfig: Could not parse JSON from file %s"), *ConfigFilePath);
		return;
	}

	for (APlayerController* PlayerController : PlayerControllers)
	{
		APawn* ControlledPawn = PlayerController->GetPawn();
		if (!ControlledPawn)
		{
			GLog->Logf(ELogVerbosity::Warning, TEXT("AdjustMaxPropertiesFromConfig: PlayerController %s does not control any Pawn"), *PlayerController->GetName());
			continue;
		}

		ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn);
		if (!ControlledCharacter)
		{
			GLog->Logf(ELogVerbosity::Warning, TEXT("AdjustMaxPropertiesFromConfig: PlayerController %s controls pawn that is not ACharacter"), *PlayerController->GetName());
			continue;
		}

		GLog->Logf(ELogVerbosity::Display, TEXT("Processing PlayerController: %s, Character: %s"), *PlayerController->GetName(), *ControlledCharacter->GetName());

		UClass* CharacterClass = ControlledCharacter->GetClass();

		TArray<FNumericProperty*> MaxProperties;
		TArray<double> CurrentValues;
		TArray<double> FileValues;

		for (UClass* CurrentClass = CharacterClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
		{
			for (TFieldIterator<FProperty> PropIt(CurrentClass, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
			{
				FProperty* Property = *PropIt;
				FString PropertyName = Property->GetName();

				if (PropertyName.Contains(TEXT("Max")) && Property->IsA(FNumericProperty::StaticClass()))
				{
					FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
					if (NumericProperty)
					{
						bool bAlreadyAdded = false;
						for (FNumericProperty* AddedProp : MaxProperties)
						{
							if (AddedProp->GetFName() == NumericProperty->GetFName())
							{
								bAlreadyAdded = true;
								break;
							}
						}

						if (bAlreadyAdded) continue;

						MaxProperties.Add(NumericProperty);

						void* PropertyValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(ControlledCharacter);
						double CurrentValue = 0.0;

						if (NumericProperty->IsFloatingPoint())
						{
							CurrentValue = NumericProperty->GetFloatingPointPropertyValue(PropertyValuePtr);
						}
						else if (NumericProperty->IsInteger())
						{
							try
							{
								CurrentValue = (double)NumericProperty->GetSignedIntPropertyValue(PropertyValuePtr);
							}
							catch (...)
							{
								CurrentValue = (double)NumericProperty->GetUnsignedIntPropertyValue(PropertyValuePtr);
							}
						}

						CurrentValues.Add(CurrentValue);

						double FileValue = 0.0;
						if (JsonObject->HasField(PropertyName))
						{
							FileValue = JsonObject->GetNumberField(PropertyName);
						}
						FileValues.Add(FileValue);

						GLog->Logf(ELogVerbosity::Display, TEXT("Found MAX property: %s (from class %s) | Current: %f | File: %f"),
							*PropertyName, *CurrentClass->GetName(), CurrentValue, FileValue);
					}
				}
			}

			if (CurrentClass == ACharacter::StaticClass())
			{
				break;
			}
		}

		if (MaxProperties.Num() == 0)
		{
			GLog->Logf(ELogVerbosity::Display, TEXT("No MAX properties found in character hierarchy"));
			continue;
		}

		double SumDifferences = 0.0;
		int ValidPropertiesCount = 0;

		for (int32 i = 0; i < MaxProperties.Num(); i++)
		{
			double Difference = FMath::Abs(CurrentValues[i] - FileValues[i]);
			if (FileValues[i] != 0.0)
			{
				SumDifferences += Difference;
				ValidPropertiesCount++;
			}
		}

		double Coefficient = 1.0;
		if (ValidPropertiesCount > 0 && SumDifferences > 0.0)
		{
			Coefficient = SumDifferences / ValidPropertiesCount;
		}

		GLog->Logf(ELogVerbosity::Display, TEXT("Properties found: %d, Valid for calculation: %d"), MaxProperties.Num(), ValidPropertiesCount);
		GLog->Logf(ELogVerbosity::Display, TEXT("Sum of differences: %f, Coefficient: %f"), SumDifferences, Coefficient);

		for (int32 i = 0; i < MaxProperties.Num(); i++)
		{
			FNumericProperty* NumericProperty = MaxProperties[i];
			FString PropertyName = NumericProperty->GetName();

			void* PropertyValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(ControlledCharacter);
			double NewValue = CurrentValues[i] * Coefficient;

			if (NumericProperty->IsFloatingPoint())
			{
				NumericProperty->SetFloatingPointPropertyValue(PropertyValuePtr, NewValue);
			}
			else if (NumericProperty->IsInteger())
			{
				int64 IntValue = FMath::RoundToInt(NewValue);
				NumericProperty->SetIntPropertyValue(PropertyValuePtr, IntValue);
			}

			GLog->Logf(ELogVerbosity::Display, TEXT("Adjusted %s: %f -> %f"), *PropertyName, CurrentValues[i], NewValue);
		}

		GLog->Logf(ELogVerbosity::Display, TEXT("PlayerController %s: All MAX properties adjusted by coefficient %f"),
			*PlayerController->GetName(), Coefficient);
	}

	GLog->Logf(ELogVerbosity::Display, TEXT("AdjustMaxPropertiesFromConfig: Completed processing all PlayerControllers"));

    GLog->Logf(ELogVerbosity::Display, TEXT("ExecuteAdjustment: Completed"));
}