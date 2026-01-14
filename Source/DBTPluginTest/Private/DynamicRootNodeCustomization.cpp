// Copyright Epic Games, Inc. All Rights Reserved.

#include "DynamicRootNodeCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "DBTBehaviorTreeDataManager.h"

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

                UObject* FirstObj = CustomizedObjects[0].Get();
                if (!FirstObj) return 0;

                UDBTBehaviorTreeDataManager& DataManager = UDBTBehaviorTreeDataManager::Get();
                int32 FirstVal = DataManager.GetLimitChangeForNode(FirstObj);

                for (int32 i = 1; i < CustomizedObjects.Num(); i++)
                {
                    if (UObject* Obj = CustomizedObjects[i].Get())
                    {
                        int32 CurrentVal = DataManager.GetLimitChangeForNode(Obj);
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
                        DataManager.SetLimitChangeForNode(Obj, NewValue);

                        GLog->Logf(ELogVerbosity::Display, TEXT("Root Limit Change for %s set to: %d"), *Obj->GetName(), NewValue);
                    }
                }

                for (TWeakObjectPtr<UObject> ObjPtr : CustomizedObjects)
                {
                    if (UObject* Obj = ObjPtr.Get())
                    {
                        Obj->MarkPackageDirty();
                    }
                }

                DetailBuilder.ForceRefreshDetails();
                    })
        ];
}