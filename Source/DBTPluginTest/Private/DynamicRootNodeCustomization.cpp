// Copyright Epic Games, Inc. All Rights Reserved.

#include "DynamicRootNodeCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "BehaviorTree/BTCompositeNode.h"

TMap<FObjectKey, int32> FBehaviorTreeRootNodeCustomization::RootLimitChangeMap;

TSharedRef<IDetailCustomization> FBehaviorTreeRootNodeCustomization::MakeInstance()
{
    return MakeShareable(new FBehaviorTreeRootNodeCustomization);
}

void FBehaviorTreeRootNodeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

    if (CustomizedObjects.Num() == 0)
        return;

    bool bIsRoot = false;
    for (auto& Obj : CustomizedObjects)
    {
        if (UBTCompositeNode* Node = Cast<UBTCompositeNode>(Obj.Get()))
        {
            UBTCompositeNode* ParentNode = Node->GetParentNode();
            if (ParentNode == nullptr)
            {
                bIsRoot = true;
            }
            else
            {
                bIsRoot = false;
                break;
            }
        }
    }

    if (!bIsRoot) return;

    IDetailCategoryBuilder& RootCategory = DetailBuilder.EditCategory(
        "Root Settings",
        FText::FromString("Root Settings"),
        ECategoryPriority::Important
    );

    RootCategory.AddCustomRow(FText::FromString("Limit Change"))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString(TEXT("Limit Change")))
                .ToolTipText(FText::FromString(TEXT("Global limit change for dynamic behavior tree")))
        ]
        .ValueContent()
        [
            SNew(SSpinBox<int32>)
                .ToolTipText(FText::FromString(TEXT("Set limit change value (0-100)")))
                .MinValue(0)
                .MaxValue(100)
                .Value_Lambda([this]() -> int32 {
                if (CustomizedObjects.Num() == 0) return 0;

                FObjectKey FirstKey(CustomizedObjects[0].Get());
                int32* FirstValue = RootLimitChangeMap.Find(FirstKey);
                int32 FirstVal = FirstValue ? *FirstValue : 0;

                for (int32 i = 1; i < CustomizedObjects.Num(); i++)
                {
                    if (UObject* Obj = CustomizedObjects[i].Get())
                    {
                        FObjectKey Key(Obj);
                        int32* Value = RootLimitChangeMap.Find(Key);
                        int32 CurrentVal = Value ? *Value : 0;

                        if (CurrentVal != FirstVal)
                        {
                            return 0;
                        }
                    }
                }

                return FirstVal;
                    })
                .OnValueChanged_Lambda([this, &DetailBuilder](int32 NewValue) {
                for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
                {
                    if (UObject* Obj = ObjPtr.Get())
                    {
                        FObjectKey Key(Obj);
                        RootLimitChangeMap.Add(Key, NewValue);

                        GLog->Logf(ELogVerbosity::Display, TEXT("Root Limit Change for %s set to: %d"), *Obj->GetName(), NewValue);
                    }
                }

                DetailBuilder.ForceRefreshDetails();
                    })
        ];
}