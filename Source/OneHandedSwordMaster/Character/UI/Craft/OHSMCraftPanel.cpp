// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftPanel.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

#include "OHSMCraftDetailInfo.h"
#include "OHSMCraftItemEntry.h"
#include "Components/Button.h"
#include "Components/TreeView.h"
#include "OneHandedSwordMaster/Character/Components/OHSMCraftComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

void UOHSMCraftPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// ESC 키 수신을 위해 포커스 설정
	SetIsFocusable(true);

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UOHSMCraftPanel::OnBtnCloseClicked);
	}

	// CraftDataTable이 없으면 CraftComponent에서 가져오기
	if (!IsValid(CraftDataTable))
	{
		APawn* Pawn = GetOwningPlayerPawn();
		if (IsValid(Pawn))
		{
			UOHSMCraftComponent* CraftComp = Pawn->FindComponentByClass<UOHSMCraftComponent>();
			if (IsValid(CraftComp))
			{
				CraftDataTable = CraftComp->GetCraftDataTable();
			}
		}
	}

	if (!IsValid(CraftDataTable))
	{
		return;
	}

	// InventoryComponent로 아이템 데이터 조회
	UOHSMInventoryComponent* InvComp = nullptr;
	APawn* Pawn = GetOwningPlayerPawn();
	if (IsValid(Pawn))
	{
		InvComp = Pawn->FindComponentByClass<UOHSMInventoryComponent>();
	}

	// 레시피 → 아이템 타입별 카테고리 분류
	TMap<EItemType, UOHSMCraftTreeItemEntry*> CategoryMap;
	TArray<UOHSMCraftTreeItemEntry*> RootItems;

	const TArray<FName> RecipeIDs = CraftDataTable->GetRowNames();
	for (const FName& RecipeID : RecipeIDs)
	{
		const FOHSMCraftItemData* Data = CraftDataTable->FindRow<FOHSMCraftItemData>(RecipeID, TEXT(""));
		if (!Data)
		{
			continue;
		}

		// 결과 아이템 타입으로 카테고리 결정
		EItemType ItemType = EItemType::Material;
		if (IsValid(InvComp))
		{
			const FItemData* ItemData = InvComp->GetItemData(Data->ResultItemID);
			if (ItemData)
			{
				ItemType = ItemData->ItemType;
			}
		}

		// 카테고리 Root 생성 또는 기존 사용
		UOHSMCraftTreeItemEntry*& RootEntry = CategoryMap.FindOrAdd(ItemType);
		if (!IsValid(RootEntry))
		{
			RootEntry = NewObject<UOHSMCraftTreeItemEntry>(this);
			RootEntry->CraftItemData = Data;
			RootEntry->RecipeID      = RecipeID;
			RootEntry->TreeDepth     = 0;
			RootEntry->CategoryType  = ItemType;
			RootItems.Add(RootEntry);
		}

		// 레시피 Child 생성
		UOHSMCraftTreeItemEntry* ChildEntry = NewObject<UOHSMCraftTreeItemEntry>(this);
		ChildEntry->RootItem      = RootEntry;
		ChildEntry->CraftItemData = Data;
		ChildEntry->RecipeID      = RecipeID;
		ChildEntry->TreeDepth     = 1;
		RootEntry->AddChild(ChildEntry);
	}

	CraftItemView->SetListItems(RootItems);
	CraftItemView->CollapseAll();
	CraftItemView->OnItemClicked().AddUObject(this, &UOHSMCraftPanel::OnCraftItemClicked);
	CraftItemView->OnItemExpansionChanged().AddUObject(this, &UOHSMCraftPanel::OnCraftItemExpansionChanged);
	CraftItemView->SetOnGetItemChildren(this, &UOHSMCraftPanel::OnCraftItemGetChildren);

	// 초기에는 상세창 숨김
	if (IsValid(CraftDetailInfo))
	{
		CraftDetailInfo->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UOHSMCraftPanel::OnCraftItemClicked(UObject* InItem)
{
	UOHSMCraftTreeItemEntry* Entry = Cast<UOHSMCraftTreeItemEntry>(InItem);
	if (!Entry || !Entry->CraftItemData)
	{
		return;
	}

	// Category(Root)를 클릭하면 상세창 숨김
	if (!Entry->RootItem)
	{
		CraftDetailInfo->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 레시피 클릭 → 상세창 표시
	CraftDetailInfo->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	CraftDetailInfo->SetCraftItemData(*Entry->CraftItemData, Entry->RecipeID);
}

void UOHSMCraftPanel::OnCraftItemExpansionChanged(UObject* InItem, bool bIsExpanded)
{
	// 필요 시 추가 처리
}

void UOHSMCraftPanel::OnCraftItemGetChildren(UObject* InItem, TArray<UObject*>& OutChildren)
{
	UOHSMCraftTreeItemEntry* Entry = Cast<UOHSMCraftTreeItemEntry>(InItem);
	if (!Entry)
	{
		return;
	}

	if (Entry->HasChildren())
	{
		for (UOHSMTreeItemEntry* Child : Entry->GetAllChildren())
		{
			OutChildren.Add(Child);
		}
	}
}

FReply UOHSMCraftPanel::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetOwningPlayer()))
		{
			PC->CloseCraftPanel();
		}
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UOHSMCraftPanel::OnBtnCloseClicked()
{
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetOwningPlayer()))
	{
		PC->CloseCraftPanel();
	}
}
