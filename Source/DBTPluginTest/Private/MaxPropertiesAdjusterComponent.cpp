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
#include "AIController.h"
#include "EngineUtils.h"

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

    FString ConfigFilePath = FPaths::ProjectConfigDir() / TEXT("GASCharacterMaxValues.json");

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

						GLog->Logf(ELogVerbosity::Display, TEXT("Found MAX property: %s (from class %s) | Current: %f | File: %f"), *PropertyName, *CurrentClass->GetName(), CurrentValue, FileValue);
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

		double Coefficient = 1.0;
		const double Beta = 3.0;
		const double Epsilon = 1e-8;

		TArray<double> LogRatios;
		LogRatios.Reserve(MaxProperties.Num());

		int32 ValidPropertiesCount = 0;

		for (int32 i = 0; i < MaxProperties.Num(); i++)
		{
			if (FileValues[i] != 0.0)
			{
				double Ratio = CurrentValues[i] / (FileValues[i] + Epsilon);
				double LogRatio = FMath::Loge(Ratio);
				LogRatios.Add(LogRatio);
				ValidPropertiesCount++;
			}
		}

		if (ValidPropertiesCount > 0)
		{
			double SumLog = 0.0;
			for (double LogVal : LogRatios)
			{
				SumLog += LogVal;
			}
			double MeanLog = SumLog / ValidPropertiesCount;

			double SumSqDiff = 0.0;
			for (double LogVal : LogRatios)
			{
				double Diff = LogVal - MeanLog;
				SumSqDiff += Diff * Diff;
			}
			double Variance = SumSqDiff / ValidPropertiesCount;
			double Sigma = FMath::Sqrt(Variance);

			Coefficient = FMath::Exp(-Beta * Sigma);
		}

		GLog->Logf(ELogVerbosity::Display, TEXT("Properties found: %d, Valid for calculation: %d"), MaxProperties.Num(), ValidPropertiesCount);
		GLog->Logf(ELogVerbosity::Display, TEXT("Coefficient: %f"), Coefficient);

		TArray<AAIController*> AIControllers;
		for (TActorIterator<AAIController> It(World); It; ++It)
		{
			AAIController* AIController = *It;
			if (AIController && AIController->IsValidLowLevel())
			{
				AIControllers.Add(AIController);
			}
		}

		if (AIControllers.Num() == 0)
		{
			GLog->Logf(ELogVerbosity::Warning, TEXT("No AIControllers found in world"));
			return;
		}

		GLog->Logf(ELogVerbosity::Display, TEXT("Found %d AIController(s) to adjust"), AIControllers.Num());

		for (AAIController* AIController : AIControllers)
		{
			APawn* AIControlledPawn = AIController->GetPawn();
			if (!AIControlledPawn)
			{
				GLog->Logf(ELogVerbosity::Warning, TEXT("AIController %s does not control any Pawn"), *AIController->GetName());
				continue;
			}

			ACharacter* AIControlledCharacter = Cast<ACharacter>(AIControlledPawn);
			if (!AIControlledCharacter)
			{
				GLog->Logf(ELogVerbosity::Warning, TEXT("AIController %s controls pawn that is not ACharacter"), *AIController->GetName());
				continue;
			}

			GLog->Logf(ELogVerbosity::Display, TEXT("Processing AIController: %s, Character: %s"), *AIController->GetName(), *AIControlledCharacter->GetName());

			UClass* AICharacterClass = AIControlledCharacter->GetClass();
			TArray<FNumericProperty*> AIMaxProperties;
			TArray<double> AICurrentValues;

			for (UClass* CurrentClass = AICharacterClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
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
							for (FNumericProperty* AddedProp : AIMaxProperties)
							{
								if (AddedProp->GetFName() == NumericProperty->GetFName())
								{
									bAlreadyAdded = true;
									break;
								}
							}

							if (bAlreadyAdded) continue;

							AIMaxProperties.Add(NumericProperty);

							void* PropertyValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(AIControlledCharacter);
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

							AICurrentValues.Add(CurrentValue);
						}
					}
				}

				if (CurrentClass == ACharacter::StaticClass())
				{
					break;
				}
			}

			if (AIMaxProperties.Num() == 0)
			{
				GLog->Logf(ELogVerbosity::Display, TEXT("No MAX properties found in AI character hierarchy"));
				continue;
			}

			for (int32 i = 0; i < AIMaxProperties.Num(); i++)
			{
				FNumericProperty* NumericProperty = AIMaxProperties[i];
				FString PropertyName = NumericProperty->GetName();

				void* PropertyValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(AIControlledCharacter);
				double NewValue = AICurrentValues[i] * (1 + Coefficient);

				if (NumericProperty->IsFloatingPoint())
				{
					NumericProperty->SetFloatingPointPropertyValue(PropertyValuePtr, NewValue);
				}
				else if (NumericProperty->IsInteger())
				{
					int64 IntValue = FMath::RoundToInt(NewValue);
					NumericProperty->SetIntPropertyValue(PropertyValuePtr, IntValue);
				}

				GLog->Logf(ELogVerbosity::Display, TEXT("Adjusted %s in AI character: %f -> %f (Coefficient: %f)"), *PropertyName, AICurrentValues[i], NewValue, Coefficient);
			}

			GLog->Logf(ELogVerbosity::Display, TEXT("AIController %s: All MAX properties adjusted by coefficient %f"), *AIController->GetName(), Coefficient);
		}

		for (int32 i = 0; i < MaxProperties.Num(); i++)
		{
			FNumericProperty* NumericProperty = MaxProperties[i];
			FString PropertyName = NumericProperty->GetName();
			double CurrentValue = CurrentValues[i];
			JsonObject->SetNumberField(PropertyName, CurrentValue);
		}

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
		{
			if (FFileHelper::SaveStringToFile(OutputString, *ConfigFilePath))
			{
				GLog->Logf(ELogVerbosity::Display, TEXT("Saved updated Max properties to %s"), *ConfigFilePath);
			}
			else
			{
				GLog->Logf(ELogVerbosity::Warning, TEXT("Failed to write config file %s"), *ConfigFilePath);
			}
		}
		else
		{
			GLog->Logf(ELogVerbosity::Warning, TEXT("Failed to serialize JSON for saving"));
		}

		GLog->Logf(ELogVerbosity::Display, TEXT("AdjustMaxPropertiesFromConfig: Completed processing all AIControllers"));
		GLog->Logf(ELogVerbosity::Display, TEXT("ExecuteAdjustment: Completed"));
	}

	StartAdjustmentTimer();
}