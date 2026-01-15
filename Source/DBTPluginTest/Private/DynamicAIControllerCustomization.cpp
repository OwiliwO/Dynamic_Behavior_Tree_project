// Copyright Epic Games, Inc. All Rights Reserved.

#include "DynamicAIControllerCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "AIController.h"
#include "DBTBehaviorTreeDataManager.h"

TSharedRef<IDetailCustomization> FDynamicAIControllerCustomization::MakeInstance()
{
    return MakeShareable(new FDynamicAIControllerCustomization);
}

void FDynamicAIControllerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

    if (CustomizedObjects.Num() == 0)
        return;

    bool bAllAIControllers = true;
    for (auto& Obj : CustomizedObjects)
    {
        if (!Obj.IsValid() || !Obj->IsA<AAIController>())
        {
            bAllAIControllers = false;
            break;
        }
    }

    if (!bAllAIControllers)
        return;

    IDetailCategoryBuilder& DynamicBehaviorCategory = DetailBuilder.EditCategory(
        "Dynamic Behavior",
        FText::FromString("Dynamic Behavior"),
        ECategoryPriority::Important
    );
    FSlateFontInfo MyFont = FCoreStyle::Get().GetFontStyle("NormalFont");
    MyFont.Size = 7;

    DynamicBehaviorCategory.AddCustomRow(FText::FromString("Enable Dynamic Behavior"))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString(TEXT("Enable Dynamic Behavior")))
                .ToolTipText(FText::FromString(TEXT("Enables dynamic behavior tree modifications for this AI Controller")))
                .Font(MyFont)
        ]
        .ValueContent()
        [
            SNew(SCheckBox)
                .ToolTipText(FText::FromString(TEXT("Toggle dynamic behavior functionality")))
                .IsChecked_Lambda([this]() -> ECheckBoxState {
                if (CustomizedObjects.Num() == 0) return ECheckBoxState::Unchecked;

                UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();
                UObject* FirstObj = CustomizedObjects[0].Get();
                bool bFirstValue = DataManager.GetAIControllerDynamicBehaviorFlag(FirstObj);

                for (int32 i = 1; i < CustomizedObjects.Num(); i++)
                {
                    if (UObject* Obj = CustomizedObjects[i].Get())
                    {
                        bool bCurrentValue = DataManager.GetAIControllerDynamicBehaviorFlag(Obj);
                        if (bCurrentValue != bFirstValue)
                        {
                            return ECheckBoxState::Undetermined;
                        }
                    }
                }

                return bFirstValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                    })
                .OnCheckStateChanged_Lambda([this, &DetailBuilder](ECheckBoxState NewState) {
                bool bNewValue = (NewState == ECheckBoxState::Checked);
                UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();

                for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
                {
                    if (UObject* Obj = ObjPtr.Get())
                    {
                        DataManager.SetAIControllerDynamicBehaviorFlag(Obj, bNewValue);
                        GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior flag for AI Controller %s set to: %s"), *Obj->GetName(), bNewValue ? TEXT("True") : TEXT("False"));
                    }
                }

                DetailBuilder.ForceRefreshDetails();
                    })
        ];

    DynamicBehaviorCategory.AddCustomRow(FText::FromString("Time Limit"))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString(TEXT("Time Limit")))
                .ToolTipText(FText::FromString(TEXT("Time limit for dynamic behavior adjustments (in seconds)")))
                .Font(MyFont)
        ]
        .ValueContent()
        [
            SNew(SSpinBox<int32>)
                .ToolTipText(FText::FromString(TEXT("Set time limit value (0-3600 seconds)")))
                .MinValue(0)
                .MaxValue(3600)
                .Value_Lambda([this]() -> int32 {
                if (CustomizedObjects.Num() == 0) return 0;

                UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();
                UObject* FirstObj = CustomizedObjects[0].Get();
                int32 FirstVal = DataManager.GetAIControllerTimeLimit(FirstObj);

                for (int32 i = 1; i < CustomizedObjects.Num(); i++)
                {
                    if (UObject* Obj = CustomizedObjects[i].Get())
                    {
                        int32 CurrentVal = DataManager.GetAIControllerTimeLimit(Obj);
                        if (CurrentVal != FirstVal)
                        {
                            return 0;
                        }
                    }
                }

                return FirstVal;
                    })
                .OnValueChanged_Lambda([this, &DetailBuilder](int32 NewValue) {
                UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();

                for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
                {
                    if (UObject* Obj = ObjPtr.Get())
                    {
                        DataManager.SetAIControllerTimeLimit(Obj, NewValue);
                        GLog->Logf(ELogVerbosity::Display, TEXT("Time Limit for AI Controller %s set to: %d"), *Obj->GetName(), NewValue);
                    }
                }

                DetailBuilder.ForceRefreshDetails();
                    })
        ];
}