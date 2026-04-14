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

	// 위젯 재활용 시 이전 상태 전체 초기화
	if (Img_Selection)
	{
		Img_Selection->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 데이터 오브젝트에 캐시된 펼침 상태로 화살표 동기화
	if (SwitcherArrow && Img_Collapsed && Img_Expanded)
	{
		UOHSMCraftTreeItemEntry* EntryForArrow = Cast<UOHSMCraftTreeItemEntry>(ListItemObject);
		const bool bExpanded = EntryForArrow ? EntryForArrow->bIsExpanded : false;
		SwitcherArrow->SetActiveWidget(bExpanded ? Img_Expanded : Img_Collapsed);
	}

	UOHSMCraftTreeItemEntry* ItemEntry = Cast<UOHSMCraftTreeItemEntry>(ListItemObject);
	if (!ItemEntry || !ItemEntry->CraftItemData)
	{
		return;
	}

	const bool bIsCategory = ItemEntry->HasChildren(); // TreeDepth 0 = 카테고리

	// SwitcherTreeItem 인덱스 설정
	if (SwitcherTreeItem)
	{
		SwitcherTreeItem->SetActiveWidgetIndex(ItemEntry->TreeDepth);
	}

	// CraftInfo 명시적 Visibility 제어 (SwitcherTreeItem 만으로 부족할 경우 대비)
	if (CraftInfo)
	{
		if (bIsCategory)
		{
			CraftInfo->ResetInfo();
			CraftInfo->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			CraftInfo->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	if (bIsCategory)
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

	// 데이터 오브젝트에 펼침 상태 캐시 (재활용 시 화살표 복원에 사용)
	if (UOHSMCraftTreeItemEntry* Entry = GetListItem<UOHSMCraftTreeItemEntry>())
	{
		Entry->bIsExpanded = bIsExpanded;
	}

	if (SwitcherArrow)
	{
		SwitcherArrow->SetActiveWidget(bIsExpanded ? Img_Expanded : Img_Collapsed);
	}
}

void UOHSMCraftItemEntry::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);

	if (Img_Selection)
	{
		Img_Selection->SetVisibility(bIsSelected
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	}
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

	// 선택된 상태면 Hover 이탈해도 Img_Selection 유지
	const bool bIsSelected = IsListItemSelected();
	if (Img_Selection && !bIsSelected)
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
