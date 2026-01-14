// Copyright Epic Games, Inc. All Rights Reserved.

#include "DynamicTaskNode.h"

#if WITH_EDITOR
#include "DetailWidgetRow.h" 
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"


TMap<FObjectKey, bool> FTaskNodeCustomization::DynamicBehaviorFlagsMap;
TMap<FObjectKey, TSharedPtr<FString>> FTaskNodeCustomization::DynamicBehaviorCategoriesMap;
TArray<TSharedPtr<FString>> FTaskNodeCustomization::CategoryOptions;

TSharedRef<IDetailCustomization> FTaskNodeCustomization::MakeInstance()
{
    return MakeShareable(new FTaskNodeCustomization);
}

TSharedRef<SWidget> CreateIntegerWidget(
    IDetailLayoutBuilder& DetailBuilder,
    TArray<TWeakObjectPtr<UObject>>& CustomizedObjects,
    TMap<FObjectKey, int32>& IntegerMap,
    const FString& Label,
    const FString& Tooltip,
    int32 MinValue,
    int32 MaxValue,
    int32 DefaultValue)
{
    return SNew(SSpinBox<int32>)
        .ToolTipText(FText::FromString(Tooltip))
        .MinValue(MinValue)
        .MaxValue(MaxValue)
        .MinSliderValue(MinValue)
        .MaxSliderValue(MaxValue)
        .Value_Lambda([&CustomizedObjects, &IntegerMap, DefaultValue]() -> int32 {
        if (CustomizedObjects.Num() == 0) return DefaultValue;

        FObjectKey FirstKey(CustomizedObjects[0].Get());
        int32* FirstValue = IntegerMap.Find(FirstKey);
        int32 FirstVal = FirstValue ? *FirstValue : DefaultValue;

        for (int32 i = 1; i < CustomizedObjects.Num(); i++)
        {
            if (UObject* Obj = CustomizedObjects[i].Get())
            {
                FObjectKey Key(Obj);
                int32* Value = IntegerMap.Find(Key);
                int32 CurrentVal = Value ? *Value : DefaultValue;

                if (CurrentVal != FirstVal)
                {
                    return DefaultValue;
                }
            }
        }

        return FirstVal;
            })
        .OnValueChanged_Lambda([&CustomizedObjects, &IntegerMap, &DetailBuilder, Label](int32 NewValue) {
        for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
        {
            if (UObject* Obj = ObjPtr.Get())
            {
                FObjectKey Key(Obj);
                IntegerMap.Add(Key, NewValue);

                GLog->Logf(ELogVerbosity::Display,
                    TEXT("%s for %s set to: %d"),
                    *Label,
                    *Obj->GetName(),
                    NewValue);
            }
        }

        DetailBuilder.ForceRefreshDetails();
            });
}

void FTaskNodeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    bool bCategoryAlreadyExists = false;

    TArray<FName> CategoryNames;
    DetailBuilder.GetCategoryNames(CategoryNames);

    for (const FName& CategoryName : CategoryNames)
    {
        if (CategoryName == "Dynamic Behavior")
        {
            bCategoryAlreadyExists = true;
            break;
        }
    }

    if (bCategoryAlreadyExists)
    {
        return;
    }

    DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

    if (CategoryOptions.Num() == 0)
    {
        CategoryOptions.Add(MakeShareable(new FString("Offensive Action")));
        CategoryOptions.Add(MakeShareable(new FString("Defensive Action")));
        CategoryOptions.Add(MakeShareable(new FString("Supporting Action")));
    }

    IDetailCategoryBuilder& PluginCategory = DetailBuilder.EditCategory(
        "Dynamic Behavior",
        FText::FromString("Dynamic Behavior"),
        ECategoryPriority::Important
    );
    FSlateFontInfo MyFont = FCoreStyle::Get().GetFontStyle("NormalFont");
    MyFont.Size = 7;

    PluginCategory.AddCustomRow(FText::FromString("Dynamic Behavior Control"))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString(TEXT("Enable Dynamic Behavior")))
                .ToolTipText(FText::FromString(TEXT("Toggle global Dynamic Behavior flag. When enabled, additional options will appear.")))
                .Font(MyFont)
        ]
        .ValueContent()
        [
            SNew(SCheckBox)
                .ToolTipText(FText::FromString(TEXT("Enable/disable Dynamic Behavior functionality for selected nodes")))
                .IsChecked_Lambda([this]() -> ECheckBoxState {

                if (CustomizedObjects.Num() == 0) return ECheckBoxState::Unchecked;

                FObjectKey FirstKey(CustomizedObjects[0].Get());
                bool* FirstFlag = DynamicBehaviorFlagsMap.Find(FirstKey);
                bool bFirstValue = FirstFlag ? *FirstFlag : false;

                for (int32 i = 1; i < CustomizedObjects.Num(); i++)
                {
                    if (UObject* Obj = CustomizedObjects[i].Get())
                    {
                        FObjectKey Key(Obj);
                        bool* Flag = DynamicBehaviorFlagsMap.Find(Key);
                        bool bCurrentValue = Flag ? *Flag : false;

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

                for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
                {
                    if (UObject* Obj = ObjPtr.Get())
                    {
                        FObjectKey Key(Obj);
                        DynamicBehaviorFlagsMap.Add(Key, bNewValue);

                        GLog->Logf(ELogVerbosity::Display, TEXT("Dynamic Behavior Tree Plugin: Dynamic Behavior flag for %s set to: %s"),
                            *Obj->GetName(),
                            bNewValue ? TEXT("True") : TEXT("False"));
                    }
                }

                DetailBuilder.ForceRefreshDetails();
                    })
        ];

    PluginCategory.AddCustomRow(FText::FromString("Action Category"))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString(TEXT("Action Category")))
                .ToolTipText(FText::FromString(TEXT("Select a category for current action for correct change priority")))
                .Font(MyFont)
        ]
        .ValueContent()
        [
            SNew(SComboBox<TSharedPtr<FString>>)
                .OptionsSource(&CategoryOptions)
                .OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
                    {
                        return SNew(STextBlock).Text(FText::FromString(*Item));
                    })
                .OnSelectionChanged_Lambda([this, &DetailBuilder](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectType)
                    {
                        for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
                        {
                            if (UObject* Obj = ObjPtr.Get())
                            {
                                FObjectKey Key(Obj);
                                DynamicBehaviorCategoriesMap.Add(Key, NewValue);

                                GLog->Logf(ELogVerbosity::Display,
                                    TEXT("Category for %s set to: %s"),
                                    *Obj->GetName(),
                                    **NewValue);
                            }
                        }

                        DetailBuilder.ForceRefreshDetails();
                    })
                .Content()
                [
                    SNew(STextBlock)
                        .Text_Lambda([this]() -> FText
                            {
                                if (CustomizedObjects.Num() == 0)
                                {
                                    return FText::FromString(TEXT("None"));
                                }

                                FObjectKey FirstKey(CustomizedObjects[0].Get());
                                TSharedPtr<FString>* FirstCategory = DynamicBehaviorCategoriesMap.Find(FirstKey);

                                if (!FirstCategory && CategoryOptions.Num() > 0)
                                {
                                    TSharedPtr<FString> DefaultCategory = CategoryOptions[0];
                                    DynamicBehaviorCategoriesMap.Add(FirstKey, DefaultCategory);
                                    return FText::FromString(*DefaultCategory);
                                }

                                for (int32 i = 1; i < CustomizedObjects.Num(); i++)
                                {
                                    if (UObject* Obj = CustomizedObjects[i].Get())
                                    {
                                        FObjectKey Key(Obj);
                                        TSharedPtr<FString>* Category = DynamicBehaviorCategoriesMap.Find(Key);

                                        if (!Category || *Category != *FirstCategory)
                                        {
                                            return FText::FromString(TEXT("Multiple Values"));
                                        }
                                    }
                                }

                                return FirstCategory ? FText::FromString(**FirstCategory) :
                                    (CategoryOptions.Num() > 0 ? FText::FromString(*CategoryOptions[0]) : FText::FromString(TEXT("None")));
                            })
                ]
        ];
}
#endif