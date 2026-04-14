// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerController.h"

#include "OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/UI/OHSMHUDWidget.h"
#include "OneHandedSwordMaster/Character/UI/OHSMEquipmentWidget.h"
#include "OneHandedSwordMaster/Character/Inventory/OHSMInventoryWidget.h"
#include "OneHandedSwordMaster/Character/Components/OHSMInventoryComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"
#include "OneHandedSwordMaster/Character/UI/Craft/OHSMCraftPanel.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMInteractableInterface.h"
#include "OneHandedSwordMaster/Character/UI/OHSMInteractionWidget.h"
#include "OneHandedSwordMaster/Character/UI/Skill/OHSMSkillPanel.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMQuestComponent.h"
#include "OneHandedSwordMaster/Character/UI/Quest/OHSMQuestPanelWidget.h"
#include "OneHandedSwordMaster/NPC/OHSMQuestNPC.h"

AOHSMPlayerController::AOHSMPlayerController()
{
	static ConstructorHelpers::FClassFinder<UOHSMHUDWidget> HUDWidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/OneHandedSwordMaster/UI/WBP_OHSMHUD.WBP_OHSMHUD_C'"));
	if (HUDWidgetClassRef.Class)
	{
		HUDWidgetClass = HUDWidgetClassRef.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UOHSMInventoryWidget> InventoryWidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/OneHandedSwordMaster/UI/WBP_Inventory.WBP_Inventory_C'"));
	if (InventoryWidgetClassRef.Class)
	{
		InventoryWidgetClass = InventoryWidgetClassRef.Class;
	}
}

void AOHSMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly GameOnlyInputMode;

	SetInputMode(GameOnlyInputMode);

	InitializeHUDWidget();
	InitializeInventoryWidget();
	InitializeEquipmentWidget();
	InitializeCraftWidget();
	InitializeInteractionWidget();
	InitializeSkillPanel();
	InitializeQuestPanel();
}

void AOHSMPlayerController::InitializeHUDWidget()
{
	if (!HUDWidgetClass)
	{
		return;
	}
	
	OHSMHUDWidget = CreateWidget<UOHSMHUDWidget>(this, HUDWidgetClass);
	if (!OHSMHUDWidget)
	{
		return;
	}
	
	OHSMHUDWidget->AddToViewport(0);
}

void AOHSMPlayerController::InitializeInventoryWidget()
{
	if (!InventoryWidgetClass)
	{
		return;
	}
	
	InventoryWidget = CreateWidget<UOHSMInventoryWidget>(this, InventoryWidgetClass);
	
	if (!InventoryWidget)
	{
		return;
	}
	
	InventoryWidget->AddToPlayerScreen(10);
	
	AOHSMPlayerCharacter* PlayerCharacter = Cast<AOHSMPlayerCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		UOHSMInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
		if (InventoryComp)
		{
			InventoryWidget->InitializeInventory(InventoryComp);

		// 아이템 획득 시 HUD에 알림 표시
		InventoryComp->OnItemAdded.AddDynamic(this, &AOHSMPlayerController::OnItemPickedUp);
		}
	}

	InventoryWidget->CloseInventory();
}

void AOHSMPlayerController::InitializeEquipmentWidget()
{
	if (!EquipmentWidgetClass)
	{
		return;
	}

	EquipmentWidget = CreateWidget<UOHSMEquipmentWidget>(this, EquipmentWidgetClass);
	if (!EquipmentWidget)
	{
		return;
	}

	EquipmentWidget->AddToPlayerScreen(10);

	AOHSMPlayerCharacter* PlayerCharacter = Cast<AOHSMPlayerCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		UOHSMEquipmentComponent* EquipmentComp = PlayerCharacter->GetEquipmentComponent();
		UOHSMInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
		UOHSMPlayerStatComponent* StatComp     = PlayerCharacter->GetStatComponent();
		if (EquipmentComp && InventoryComp)
		{
			EquipmentWidget->InitializeEquipment(EquipmentComp, InventoryComp, StatComp);
		}
	}

	EquipmentWidget->CloseEquipment();
}

void AOHSMPlayerController::InitializeCraftWidget()
{
	if (!CraftWidgetClass)
	{
		return;
	}

	CraftWidget = CreateWidget<UOHSMCraftPanel>(this, CraftWidgetClass);
	if (!CraftWidget)
	{
		return;
	}

	CraftWidget->AddToPlayerScreen(10);
	CraftWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AOHSMPlayerController::InitializeInteractionWidget()
{
	if (!InteractionWidgetClass) return;

	InteractionWidget = CreateWidget<UOHSMInteractionWidget>(this, InteractionWidgetClass);
	if (!InteractionWidget) return;

	InteractionWidget->AddToPlayerScreen(5);
	InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AOHSMPlayerController::ShowInteractionWidget(const FText& InHintText)
{
	if (!InteractionWidget) return;

	InteractionWidget->SetInteractionText(InHintText);
	InteractionWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void AOHSMPlayerController::HideInteractionWidget()
{
	if (!InteractionWidget) return;

	InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AOHSMPlayerController::ToggleCraftPanel()
{
	if (!CraftWidget)
	{
		return;
	}

	if (CraftWidget->IsVisible())
	{
		CloseCraftPanel();
	}
	else
	{
		OpenCraftPanel();
	}
}

void AOHSMPlayerController::OpenCraftPanel()
{
	if (!CraftWidget)
	{
		return;
	}

	CraftWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	CraftWidget->SetKeyboardFocus();
	UpdateInputMode();
}

void AOHSMPlayerController::CloseCraftPanel()
{
	if (!CraftWidget)
	{
		return;
	}

	CraftWidget->SetVisibility(ESlateVisibility::Collapsed);
	UpdateInputMode();
}

void AOHSMPlayerController::ToggleEquipment()
{
	if (!EquipmentWidget)
	{
		return;
	}

	EquipmentWidget->ToggleEquipment();
	UpdateInputMode();
}

void AOHSMPlayerController::SetDialogueOpen(bool bOpen)
{
	bIsDialogueOpen = bOpen;
	UpdateInputMode();
}

void AOHSMPlayerController::SetInteractableActor(AActor* InActor)
{
	CurrentInteractableActor = InActor;
}

void AOHSMPlayerController::TryInteract()
{
	if (!IsValid(CurrentInteractableActor)) return;

	IOHSMInteractableInterface* Interactable =
		Cast<IOHSMInteractableInterface>(CurrentInteractableActor);
	if (Interactable)
	{
		Interactable->Execute_Interact(CurrentInteractableActor, this);
	}
}

void AOHSMPlayerController::OnItemPickedUp(FName ItemID, int32 Count)
{
	if (OHSMHUDWidget)
	{
		OHSMHUDWidget->ShowItemPickup(ItemID, Count);
	}
}

void AOHSMPlayerController::ToggleInventory()
{
	if (!InventoryWidget)
	{
		return;
	}

	InventoryWidget->ToggleInventory();
	bIsInventoryOpen = InventoryWidget->IsVisible();
	UpdateInputMode();
}

void AOHSMPlayerController::InitializeSkillPanel()
{
	if (!SkillPanelWidgetClass) return;

	SkillPanelWidget = CreateWidget<UOHSMSkillPanel>(this, SkillPanelWidgetClass);
	if (!SkillPanelWidget) return;

	SkillPanelWidget->AddToViewport(100);

	// 스킬 컴포넌트 연결
	if (AOHSMPlayerCharacter* PC = Cast<AOHSMPlayerCharacter>(GetPawn()))
	{
		if (UOHSMSkillComponent* SkillComp = PC->GetSkillComponent())
		{
			SkillPanelWidget->SetSkillComponent(SkillComp);
		}
	}

	SkillPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AOHSMPlayerController::ToggleSkillPanel()
{
	if (!SkillPanelWidget) return;

	if (SkillPanelWidget->IsVisible())
	{
		CloseSkillPanel();
	}
	else
	{
		OpenSkillPanel();
	}
}

void AOHSMPlayerController::OpenSkillPanel()
{
	if (!SkillPanelWidget) return;

	SkillPanelWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SkillPanelWidget->SetKeyboardFocus();
	bIsSkillPanelOpen = true;
	UpdateInputMode();
}

void AOHSMPlayerController::CloseSkillPanel()
{
	if (!SkillPanelWidget) return;

	SkillPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	bIsSkillPanelOpen = false;
	UpdateInputMode();
}

void AOHSMPlayerController::InitializeQuestPanel()
{
	if (!QuestPanelWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuestPanel] QuestPanelWidgetClass 가 설정되지 않았습니다. BP_PlayerController 에서 지정하세요."));
		return;
	}

	QuestPanelWidget = CreateWidget<UOHSMQuestPanelWidget>(this, QuestPanelWidgetClass);
	if (!QuestPanelWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] 위젯 생성 실패"));
		return;
	}

	QuestPanelWidget->AddToPlayerScreen(20);
	QuestPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	UE_LOG(LogTemp, Log, TEXT("[QuestPanel] 초기화 완료"));
}

void AOHSMPlayerController::OpenQuestPanel(AOHSMQuestNPC* QuestNPC)
{
	if (!QuestPanelWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] QuestPanelWidget 이 null — QuestPanelWidgetClass 를 설정했는지 확인하세요."));
		return;
	}
	if (!QuestNPC)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] QuestNPC 가 null"));
		return;
	}

	AOHSMPlayerCharacter* PC = Cast<AOHSMPlayerCharacter>(GetPawn());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] PlayerCharacter Cast 실패"));
		return;
	}

	UOHSMQuestComponent* QuestComp = PC->GetQuestComponent();
	if (!QuestComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestPanel] QuestComponent 가 null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[QuestPanel] 오픈 성공 — NPC: %s"), *QuestNPC->GetName());
	QuestPanelWidget->InitializePanel(QuestComp, QuestNPC);
	QuestPanelWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	bIsQuestPanelOpen = true;
	UpdateInputMode();
}

void AOHSMPlayerController::CloseQuestPanel()
{
	if (!QuestPanelWidget) return;
	QuestPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	bIsQuestPanelOpen = false;
	UpdateInputMode();
}

bool AOHSMPlayerController::IsAnyWindowOpen() const
{
	const bool bInventoryOpen = InventoryWidget && InventoryWidget->IsVisible();
	const bool bEquipmentOpen = EquipmentWidget && EquipmentWidget->IsVisible();
	const bool bCraftOpen     = CraftWidget     && CraftWidget->IsVisible();
	return bInventoryOpen || bEquipmentOpen || bCraftOpen || bIsDialogueOpen || bIsSkillPanelOpen || bIsQuestPanelOpen;
}

void AOHSMPlayerController::UpdateInputMode()
{
	const bool bInventoryOpen = InventoryWidget && InventoryWidget->IsVisible();
	const bool bEquipmentOpen = EquipmentWidget && EquipmentWidget->IsVisible();
	const bool bCraftOpen     = CraftWidget     && CraftWidget->IsVisible();
	const bool bAnyWindowOpen = bInventoryOpen || bEquipmentOpen || bCraftOpen || bIsDialogueOpen || bIsSkillPanelOpen || bIsQuestPanelOpen;

	if (bAnyWindowOpen)
	{
		// 창이 하나라도 열려있으면 → GameAndUI (마우스 + 키입력 동시 가능)
		FInputModeGameAndUI GameAndUIMode;
		GameAndUIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		GameAndUIMode.SetHideCursorDuringCapture(false);
		SetInputMode(GameAndUIMode);
		SetShowMouseCursor(true);
	}
	else
	{
		// 모든 창이 닫혔을 때만 게임 전용 모드로 복귀
		FInputModeGameOnly GameOnlyMode;
		SetInputMode(GameOnlyMode);
		SetShowMouseCursor(false);
	}
}
