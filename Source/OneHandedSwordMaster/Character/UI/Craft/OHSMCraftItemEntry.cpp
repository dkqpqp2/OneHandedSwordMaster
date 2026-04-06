// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftItemEntry.h"

#include "OHSMCraftInfo.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

// ─── UOHSMTreeItemEntry ─────────────────────────────────────────

bool UOHSMTreeItemEntry::HasChildren() const
{
	return TreeChildren.Num() > 0;
}

TArray<UOHSMTreeItemEntry*> UOHSMTreeItemEntry::GetAllChildren() const
{
	return TreeChildren;
}

void UOHSMTreeItemEntry::AddChild(UOHSMTreeItemEntry* InChild)
{
	TreeChildren.AddUnique(InChild);
}

void UOHSMTreeItemEntry::RemoveChild(UOHSMTreeItemEntry* InChild)
{
	TreeChildren.Remove(InChild);
}

void UOHSMTreeItemEntry::ClearChildren()
{
	TreeChildren.Reset();
}

// ─── UOHSMCraftItemEntry ────────────────────────────────────────

void UOHSMCraftItemEntry::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 상태 숨김
	if (Img_Selection)
	{
		Img_Selection->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UOHSMCraftItemEntry::NativeDestruct()
{
	Super::NativeDestruct();
}

void UOHSMCraftItemEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// 위젯 재활용(recycling) 시 이전 hover 상태 초기화
	if (Img_Selection)
	{
		Img_Selection->SetVisibility(ESlateVisibility::Collapsed);
	}

	UOHSMCraftTreeItemEntry* ItemEntry = Cast<UOHSMCraftTreeItemEntry>(ListItemObject);
	if (!ItemEntry || !ItemEntry->CraftItemData)
	{
		return;
	}

	SwitcherTreeItem->SetActiveWidgetIndex(ItemEntry->TreeDepth);

	if (ItemEntry->HasChildren())
	{
		UpdateCategory(*ItemEntry->CraftItemData);
	}
	else
	{
		UpdateItem(*ItemEntry->CraftItemData, ItemEntry->RecipeID);
	}
}

void UOHSMCraftItemEntry::NativeOnItemExpansionChanged(bool bIsExpanded)
{
	IUserObjectListEntry::NativeOnItemExpansionChanged(bIsExpanded);

	if (SwitcherArrow)
	{
		SwitcherArrow->SetActiveWidget(bIsExpanded ? Img_Expanded : Img_Collapsed);
	}
}

void UOHSMCraftItemEntry::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);
	// 선택 강조는 Hover로 처리 — 여기선 별도 처리 없음
}

void UOHSMCraftItemEntry::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (Img_Selection)
	{
		Img_Selection->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UOHSMCraftItemEntry::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (Img_Selection)
	{
		Img_Selection->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UOHSMCraftItemEntry::UpdateCategory(const FOHSMCraftItemData& InData)
{
	UOHSMCraftTreeItemEntry* Entry = GetListItem<UOHSMCraftTreeItemEntry>();
	if (!Entry || !Label_CategoryName)
	{
		return;
	}

	// EItemType의 DisplayName 사용 (한글 표시: 장비, 소비, 재료, 퀘스트)
	FText CategoryText = UEnum::GetDisplayValueAsText(Entry->CategoryType);
	Label_CategoryName->SetText(CategoryText);
}

void UOHSMCraftItemEntry::UpdateItem(const FOHSMCraftItemData& InData, FName InRecipeID)
{
	if (CraftInfo)
	{
		CraftInfo->SetCraftItemData(InData, InRecipeID);
	}
}
