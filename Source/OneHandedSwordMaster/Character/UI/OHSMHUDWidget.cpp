// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMHUDWidget.h"

#include "OHSMExpBar.h"
#include "OHSMHpBar.h"
#include "OHSMManaBar.h"
#include "OHSMPickupNotifyWidget.h"
#include "OHSMQuickSlotWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuickSlotComponent.h"
#include "Components/ProgressBar.h"
#include "Kismet/GamePlayStatics.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"

UOHSMHUDWidget::UOHSMHUDWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UOHSMHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	IOHSMCharacterHUDInterface* HUDPawn = Cast<IOHSMCharacterHUDInterface>(GetOwningPlayerPawn());
	if (HUDPawn)
	{
		HUDPawn->SetupHUDWidget(this);
	}
	
	InitializeHUD();
}

void UOHSMHUDWidget::InitializeHUD()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}
	
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(PlayerController->GetPawn());
	if (!Player)
	{
		return;
	}
	
	UOHSMPlayerStatComponent* StatComp = Cast<UOHSMPlayerStatComponent>(Player->GetStatComponent());
	if (!StatComp)
	{
		return;
	}
	
	StatComp->OnHpChanged.AddUObject(this, &UOHSMHUDWidget::UpdateHp);

	InitializeQuickSlots();
	
	StatComp->OnManaChanged.AddUObject(this, &UOHSMHUDWidget::UpdateMana);
	
	StatComp->OnExpChanged.AddDynamic(this, &UOHSMHUDWidget::OnExpChanged);
	
	UpdateHp(StatComp->GetCurrentHp(), StatComp->GetMaxHp());
	UpdateMana(StatComp->GetCurrentMana(), StatComp->GetMaxMana());
	OnExpChanged(0, StatComp->GetLevel());
}

void UOHSMHUDWidget::InitializeQuickSlots()
{
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player)
	{
		return;
	}

	UOHSMQuickSlotComponent* QuickSlotComp = Player->GetQuickSlotComponent();
	UOHSMInventoryComponent* InventoryComp = Player->GetInventoryComponent();

	if (WidgetPotionSlots)
	{
		WidgetPotionSlots->InitializeSlots(QuickSlotComp, InventoryComp);
	}

	if (WidgetSkillSlots)
	{
		WidgetSkillSlots->InitializeSlots(QuickSlotComp, InventoryComp);
	}
}

void UOHSMHUDWidget::UpdateHp(float CurrentHp, float MaxHp)
{
	WidgetHpBar->UpdateHpBar(CurrentHp, MaxHp);
	if (TextHp)
	{
		TextHp->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), CurrentHp, MaxHp)
		));
	}
}

void UOHSMHUDWidget::UpdateMana(float CurrentMana, float MaxMana)
{
	WidgetManaBar->UpdateManaBar(CurrentMana, MaxMana);
	if (TextMana)
	{
		TextMana->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), CurrentMana, MaxMana)
		));
	}
}

void UOHSMHUDWidget::ShowItemPickup(FName ItemID, int32 Count)
{
	if (!PickupNotifyWidget)
	{
		return;
	}

	// InventoryComponent에서 ItemData 가져오기
	AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player)
	{
		return;
	}

	UOHSMInventoryComponent* InventoryComp = Player->GetInventoryComponent();
	if (!InventoryComp)
	{
		return;
	}

	const FItemData* ItemData = InventoryComp->GetItemData(ItemID);
	if (!ItemData)
	{
		return;
	}

	PickupNotifyWidget->ShowItemPickup(*ItemData, Count);
}

void UOHSMHUDWidget::OnExpChanged(int32 CurrentExp, int32 RequiredExp)
{
	WidgetExpBar->UpdateExpBar(CurrentExp, RequiredExp);
    
	if (TextExp)
	{
		float Percent = RequiredExp > 0 ? static_cast<float>(CurrentExp) / static_cast<float>(RequiredExp) * 100.0f : 0.0f;
		TextExp->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f%%"), Percent)
		));
	}
}

