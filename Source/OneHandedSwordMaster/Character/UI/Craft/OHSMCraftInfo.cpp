// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCraftInfo.h"

#include "OHSMCraftMaterialItem.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCraftData.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"

void UOHSMCraftInfo::SetCraftItemData(const FOHSMCraftItemData& InData, FName InRecipeID)
{
	// InventoryComponent 조회 — Pawn 또는 PlayerController 어디서든 안전하게
	UOHSMInventoryComponent* InvComp = nullptr;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			InvComp = Pawn->FindComponentByClass<UOHSMInventoryComponent>();
		}
	}

	// ── 인벤토리 변경 구독 ────────────────────────────────────────────────────
	// 이전 컴포넌트 바인딩 해제 후 새 컴포넌트에 재구독.
	// AddUniqueDynamic: 같은 (오브젝트, 함수명) 쌍이 이미 있으면 스킵 → 중복 방지.
	// 파생 클래스(UOHSMCraftDetailInfo)가 NativeConstruct에서 먼저 구독해둔 경우에도
	// UE 리플렉션이 가장 파생된 OnInventoryUpdated를 호출하므로 동작이 보장된다.
	if (IsValid(CachedInventoryComp))
	{
		CachedInventoryComp->OnInventoryUpdated.RemoveAll(this);
	}
	if (IsValid(InvComp))
	{
		CachedInventoryComp = InvComp;
		InvComp->OnInventoryUpdated.AddUniqueDynamic(this, &UOHSMCraftInfo::OnInventoryUpdated);
	}

	// 결과 아이템 정보 표시
	if (IsValid(InvComp))
	{
		const FItemData* ItemData = InvComp->GetItemData(InData.ResultItemID);
		if (ItemData)
		{
			if (Label_ResultName)
			{
				Label_ResultName->SetText(ItemData->ItemName);
			}
			if (Img_ResultItem && ItemData->ItemIcon)
			{
				Img_ResultItem->SetBrushFromTexture(ItemData->ItemIcon);
			}
		}
	}

	// 재료 목록
	if (!WrapBox_Material || !MaterialWidgetClass)
	{
		return;
	}

	WrapBox_Material->ClearChildren();

	for (const FCraftMaterialData& MaterialData : InData.Materials)
	{
		UOHSMCraftMaterialItem* MaterialWidget =
			CreateWidget<UOHSMCraftMaterialItem>(this, MaterialWidgetClass);
		if (IsValid(MaterialWidget))
		{
			MaterialWidget->SetMaterialData(MaterialData);
			WrapBox_Material->AddChild(MaterialWidget);
		}
	}
}

void UOHSMCraftInfo::ResetInfo()
{
	if (WrapBox_Material)
	{
		WrapBox_Material->ClearChildren();
	}

	if (Label_ResultName)
	{
		Label_ResultName->SetText(FText::GetEmpty());
	}

	if (Img_ResultItem)
	{
		Img_ResultItem->SetBrushFromTexture(nullptr);
	}
}

void UOHSMCraftInfo::NativeDestruct()
{
	if (IsValid(CachedInventoryComp))
	{
		CachedInventoryComp->OnInventoryUpdated.RemoveAll(this);
		CachedInventoryComp = nullptr;
	}
	Super::NativeDestruct();
}

void UOHSMCraftInfo::OnInventoryUpdated(int32 /*SlotIndex*/)
{
	// 베이스 구현: 재료 보유 수량 갱신
	// 파생 클래스(UOHSMCraftDetailInfo)는 이를 override해 버튼 상태도 함께 갱신한다.
	RefreshMaterialCounts();
}

void UOHSMCraftInfo::RefreshMaterialCounts()
{
	if (!WrapBox_Material) return;

	for (UWidget* Child : WrapBox_Material->GetAllChildren())
	{
		if (UOHSMCraftMaterialItem* MaterialItem = Cast<UOHSMCraftMaterialItem>(Child))
		{
			MaterialItem->RefreshCount();
		}
	}
}
