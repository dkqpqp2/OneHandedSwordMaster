// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMInventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Item/OHSMPickupItem.h"
#include "OneHandedSwordMaster/Character/UI/OHSMQuickSlotEntryWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "OneHandedSwordMaster/Character/UI/OHSMEquipmentSlotWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

void UOHSMInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshUI();
}

FReply UOHSMInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InEvent);
	
	if (InEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		UE_LOG(LogTemp, Warning, TEXT("[InvSlot] 우클릭 - SlotIndex:%d  SlotEmpty:%s"),
			SlotIndex, SlotData.IsEmpty() ? TEXT("YES") : TEXT("NO"));

		// 우클릭: 장비 아이템이면 장비창에 자동 장착
		if (!SlotData.IsEmpty())
		{
			const FItemData* ItemData = GetItemData();
			UE_LOG(LogTemp, Warning, TEXT("[InvSlot] ItemID:%s  ItemData:%s  ItemType:%d  EquipSlot:%d"),
				*SlotData.ItemID.ToString(),
				ItemData ? TEXT("OK") : TEXT("NULL"),
				ItemData ? (int32)ItemData->ItemType : -1,
				ItemData ? (int32)ItemData->EquipSlot : -1);

			if (ItemData && ItemData->ItemType == EItemType::Equipment && ItemData->EquipSlot != EEquipmentSlot::None)
			{
				AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetOwningPlayer());
				AOHSMPlayerCharacter* Player = PC ? Cast<AOHSMPlayerCharacter>(PC->GetPawn()) : nullptr;
				UOHSMEquipmentComponent* EquipComp = Player ? Player->GetEquipmentComponent() : nullptr;

				UE_LOG(LogTemp, Warning, TEXT("[InvSlot] PC:%s  Player:%s  EquipComp:%s"),
					PC ? TEXT("OK") : TEXT("NULL"),
					Player ? TEXT("OK") : TEXT("NULL"),
					EquipComp ? TEXT("OK") : TEXT("NULL"));

				if (EquipComp)
				{
					// SlotIndex 기반으로 정확한 슬롯의 아이템을 장착
					EquipComp->EquipItemFromSlot(SlotIndex);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[InvSlot] 장비 아이템 아님 → 장착 스킵"));
			}
		}
		return FReply::Handled();
	}

	if (InEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (!SlotData.IsEmpty())
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}
	return FReply::Handled();
}

void UOHSMInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UDragDropOperation* DragDropOp = NewObject<UDragDropOperation>();
	
	if (DragDropOp)
	{
		DragDropOp->Payload = this;
		
		if (GetClass())
		{
			UOHSMInventorySlotWidget* DragVisual = CreateWidget<UOHSMInventorySlotWidget>(GetOwningPlayer(), GetClass());

			if (DragVisual)
			{
				DragVisual->InitializeSlot(SlotIndex, InventoryComponent);
				DragVisual->SetRenderOpacity(0.7f);
                
				DragDropOp->DefaultDragVisual = DragVisual;
			}
		}
        
		DragDropOp->Pivot = EDragPivot::MouseDown;
        
		OutOperation = DragDropOp;
        
		SetDragging(true);
	}
}

bool UOHSMInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (!InOperation || !InventoryComponent)
	{
		return false;
	}
	
	// 장비 슬롯에서 인벤토리로 드롭 → 장비 해제 후 드롭한 슬롯에 배치
	UOHSMEquipmentSlotWidget* DraggedEquipSlot = Cast<UOHSMEquipmentSlotWidget>(InOperation->Payload);
	if (DraggedEquipSlot)
	{
		UOHSMEquipmentComponent* EquipComp = DraggedEquipSlot->GetEquipmentComponent();
		if (EquipComp)
		{
			// 해제 전에 아이템 ID 저장
			FName ItemID = EquipComp->GetEquippedItem(DraggedEquipSlot->GetSlotType());

			// 장착 해제 (인벤토리 첫 번째 빈 슬롯에 들어감)
			EquipComp->UnequipItem(DraggedEquipSlot->GetSlotType());

			// 아이템이 실제로 들어간 슬롯 찾기
			int32 AddedIndex = InventoryComponent->FindItemSlot(ItemID, false);

			// 드롭한 슬롯(SlotIndex)으로 이동
			if (AddedIndex != INDEX_NONE && AddedIndex != SlotIndex)
			{
				InventoryComponent->SwapSlots(AddedIndex, SlotIndex);
			}
		}
		return true;
	}

	// 퀵슬롯에서 인벤토리로 드롭
	UOHSMQuickSlotEntryWidget* DraggedQuickSlot = Cast<UOHSMQuickSlotEntryWidget>(InOperation->Payload);
	if (DraggedQuickSlot)
	{
		DraggedQuickSlot->SetRenderOpacity(1.0f);

		UOHSMQuickSlotComponent* QuickSlotComp = DraggedQuickSlot->GetQuickSlotComponent();
		if (QuickSlotComp)
		{
			FName ItemID = QuickSlotComp->GetSlotItemID(DraggedQuickSlot->GetSlotIndex());

			// 아이템이 인벤토리 어느 슬롯에 있는지 찾아서 드롭한 슬롯 위치로 이동
			if (!ItemID.IsNone())
			{
				int32 ItemSlotIndex = InventoryComponent->FindItemSlot(ItemID, false);
				int32 TargetSlotIndex = this->GetSlotIndex();

				if (ItemSlotIndex != INDEX_NONE && ItemSlotIndex != TargetSlotIndex)
				{
					InventoryComponent->SwapSlots(ItemSlotIndex, TargetSlotIndex);
				}
			}

			// 퀵슬롯 해제 + UI 즉시 갱신
			QuickSlotComp->ClearSlot(DraggedQuickSlot->GetSlotIndex());
			DraggedQuickSlot->RefreshSlot();
		}
		return true;
	}

	// 인벤토리 슬롯끼리 스왑
	UOHSMInventorySlotWidget* DraggedSlot = Cast<UOHSMInventorySlotWidget>(InOperation->Payload);
	if (!DraggedSlot)
	{
		return false;
	}

	DraggedSlot->SetDragging(false);

	int32 FromIndex = DraggedSlot->GetSlotIndex();
	int32 ToIndex = this->GetSlotIndex();

	if (FromIndex == ToIndex)
	{
		return false;
	}

	bool bSuccess = InventoryComponent->SwapSlots(FromIndex, ToIndex);

	return bSuccess;
}

void UOHSMInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	SetDragging(false);

	// 슬롯에 아이템이 있을 때만 월드에 드롭
	if (!SlotData.IsEmpty())
	{
		DropItemToWorld();
	}
}

void UOHSMInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::HitTestInvisible);
		HoverHighlight->SetRenderOpacity(0.3f);
	}
}

void UOHSMInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (HoverHighlight)
	{
		HoverHighlight->SetVisibility(ESlateVisibility::Hidden);
		HoverHighlight->SetRenderOpacity(0.0f);
	}
}

void UOHSMInventorySlotWidget::InitializeSlot(int32 InSlotIndex, UOHSMInventoryComponent* InInventory)
{
	SlotIndex = InSlotIndex;
	InventoryComponent = InInventory;

	if (TooltipWidgetClass && GetOwningPlayer())
	{
		SlotTooltipWidget = CreateWidget<UOHSMInventoryTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
		if (SlotTooltipWidget)
		{
			SetToolTip(SlotTooltipWidget);
		}
	}

	if (InventoryComponent)
	{
		const FInventorySlot* InventorySlot = InventoryComponent->GetSlot(SlotIndex);
		if (InventorySlot)
		{
			UpdateSlot(*InventorySlot);
		}
	}
}

void UOHSMInventorySlotWidget::UpdateSlot(const FInventorySlot& NewSlotData)
{
	SlotData = NewSlotData;
	RefreshUI();
}

void UOHSMInventorySlotWidget::ClearSlot()
{
	SlotData.SlotClear();
	RefreshUI();
}

void UOHSMInventorySlotWidget::SetDragging(bool bIsDragging)
{
	if (bIsDragging)
	{
		SetRenderOpacity(0.5f);
	}
	else
	{
		SetRenderOpacity(1.0f);
	}
}

void UOHSMInventorySlotWidget::DropItemToWorld()
{
	if (!InventoryComponent || !DropItemClass || SlotData.IsEmpty())
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 플레이어 앞쪽 100 유닛 위치에 스폰
	FVector SpawnLocation = Pawn->GetActorLocation()
		+ Pawn->GetActorForwardVector() * 100.0f;

	// 지면 Line Trace로 정확한 높이 찾기
	FHitResult HitResult;
	FVector TraceStart = FVector(SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z + 200.0f);
	FVector TraceEnd   = FVector(SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z - 500.0f);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Pawn);
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		SpawnLocation.Z = HitResult.ImpactPoint.Z + 20.0f;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AOHSMPickupItem* SpawnedItem = World->SpawnActor<AOHSMPickupItem>(
		DropItemClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (SpawnedItem)
	{
		SpawnedItem->InitializeItem(SlotData.ItemID, SlotData.Count, InventoryComponent->GetItemDataTable());
		InventoryComponent->RemoveItemFromSlot(SlotIndex, SlotData.Count);
	}
}

const FItemData* UOHSMInventorySlotWidget::GetItemData() const
{
	if (InventoryComponent)
	{
		return InventoryComponent->GetItemData(SlotData.ItemID);
	}
	
	return nullptr;
}

void UOHSMInventorySlotWidget::RefreshUI()
{
	if (SlotData.IsEmpty())
	{
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}

		if (ItemCount)
		{
			ItemCount->SetVisibility(ESlateVisibility::Hidden);
		}

		SetToolTip(nullptr);
	}
	else
	{
		const FItemData* ItemData = GetItemData();
		if (ItemData)
		{
			if (ItemIcon && ItemData->ItemIcon)
			{
				ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
				ItemIcon->SetVisibility(ESlateVisibility::Visible);
			}

			if (ItemCount)
			{
				if (SlotData.Count > 1)
				{
					ItemCount->SetText(FText::AsNumber(SlotData.Count));
					ItemCount->SetVisibility(ESlateVisibility::Visible);
				}
				else
				{
					ItemCount->SetVisibility(ESlateVisibility::Hidden);
				}
			}

			if (SlotTooltipWidget)
			{
				SlotTooltipWidget->UpdateTooltip(*ItemData);
				SetToolTip(SlotTooltipWidget);
			}
		}
	}
}
