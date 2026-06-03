// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OHSMPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AOHSMPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
	// HUD 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<class UOHSMHUDWidget> HUDWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<class UOHSMHUDWidget> OHSMHUDWidget;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<class UOHSMInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<class UOHSMInventoryWidget> InventoryWidget;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<class UOHSMEquipmentWidget> EquipmentWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<class UOHSMEquipmentWidget> EquipmentWidget;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<class UOHSMCraftPanel> CraftWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Craft")
	TObjectPtr<class UOHSMCraftPanel> CraftWidget;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSubclassOf<class UOHSMInteractionWidget> InteractionWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<class UOHSMInteractionWidget> InteractionWidget;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<class UOHSMSkillPanel> SkillPanelWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<class UOHSMSkillPanel> SkillPanelWidget;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TSubclassOf<class UOHSMQuestPanelWidget> QuestPanelWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<class UOHSMQuestPanelWidget> QuestPanelWidget;

protected:
	void InitializeHUDWidget();
	void InitializeInventoryWidget();
	void InitializeEquipmentWidget();
	void InitializeCraftWidget();
	void InitializeInteractionWidget();
	void InitializeSkillPanel();
	void InitializeQuestPanel();

public:
	// 맵 이동 후 위젯-컴포넌트 재연결 (LoadFromGameInstance 완료 후 호출)
	void RefreshWidgetConnections();

	/** 현재 열린 창 상태에 따라 입력 모드 / 마우스 커서 자동 갱신 */
	void UpdateInputMode();

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI|Equipment")
	void ToggleEquipment();

	UFUNCTION(BlueprintCallable, Category = "UI|Craft")
	void ToggleCraftPanel();

	UFUNCTION(BlueprintCallable, Category = "UI|Craft")
	void OpenCraftPanel();

	UFUNCTION(BlueprintCallable, Category = "UI|Craft")
	void CloseCraftPanel();

	UFUNCTION(BlueprintCallable, Category = "UI|Skill")
	void ToggleSkillPanel();

	UFUNCTION(BlueprintCallable, Category = "UI|Skill")
	void OpenSkillPanel();

	UFUNCTION(BlueprintCallable, Category = "UI|Skill")
	void CloseSkillPanel();

	UFUNCTION(BlueprintCallable, Category = "UI|Quest")
	void OpenQuestPanel(class AOHSMQuestNPC* QuestNPC);

	UFUNCTION(BlueprintCallable, Category = "UI|Quest")
	void CloseQuestPanel();

	/** B 키 — 자동이동 토글 (이동 중이면 중지, 아니면 첫 추적 퀘스트로 이동) */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void TriggerAutoMove();

	/** 자동이동 중 여부 */
	UFUNCTION(BlueprintPure, Category = "Navigation")
	bool IsAutoMoving() const { return bIsAutoMoving; }

	/** 지정 위치로 자동이동 시작 (상태 플래그 포함) */
	void StartAutoMoveTo(const FVector& TargetLocation);

	/** 자동이동 중지 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void StopAutoMove();

// ─── 사망 / 리스폰 ────────────────────────────────────────────────────────
public:
	/** PlayerCharacter::SetDead 에서 호출 — 보유 부활 포션 개수 전달 */
	void HandlePlayerDeath(int32 PotionCount);

	/** 부활 포션 사용 (사망 화면의 버튼에서 호출) */
	void UseRevivalPotion();

protected:
	void InitializeDeathWidget();

	/** 사망 화면 표시 */
	void ShowDeathScreen(int32 PotionCount);

	/** 사망 화면 숨김 */
	void HideDeathScreen();

	/** 카운트다운 0.1초마다 UI 업데이트 */
	void UpdateCountdown();

	/** 카운트다운 종료 → 리스폰 구역으로 이동 */
	void ExecuteRespawn();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	TSubclassOf<class UOHSMDeathWidget> DeathWidgetClass;

	/** 리스폰 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn", meta = (ClampMin = "1"))
	float RespawnDelay = 5.0f;

	/** 부활 포션 사용 시 회복할 HP 비율 (0~1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float RevivalHealPercent = 0.3f;

private:
	UPROPERTY()
	TObjectPtr<class UOHSMDeathWidget> DeathWidget;

	FTimerHandle RespawnTimerHandle;
	FTimerHandle CountdownTimerHandle;
	float RespawnTimeRemaining = 0.0f;

	
	UFUNCTION(BlueprintPure, Category = "UI")
	UOHSMInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }
    
	UFUNCTION(BlueprintPure, Category = "UI")
	UOHSMHUDWidget* GetHUDWidget() const { return OHSMHUDWidget; }

protected:
	UPROPERTY()
	bool bIsInventoryOpen = false;

public:
	UFUNCTION(BlueprintPure, Category = "UI|Inventory")
	bool IsInventoryOpen() const { return bIsInventoryOpen; }

	/** 인벤토리 또는 장비창 중 하나라도 열려있으면 true */
	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsAnyWindowOpen() const;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetIsInventoryOpen(bool bOpen) { bIsInventoryOpen = bOpen; }

	// ─── 상호작용 ─────────────────────────────────────────────────
public:
	/** NPC 범위 진입/이탈 시 호출 */
	void SetInteractableActor(AActor* InActor);

	/** 상호작용 힌트 위젯 표시 / 숨김 */
	void ShowInteractionWidget(const FText& InHintText);
	void HideInteractionWidget();

	/** 플레이어가 E 키를 눌렀을 때 호출 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	/** NPC 대화창 열림/닫힘 알림 — 입력 모드 갱신용 */
	void SetDialogueOpen(bool bOpen);

private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractableActor;

	bool bIsDialogueOpen    = false;
	bool bIsSkillPanelOpen  = false;
	bool bIsQuestPanelOpen  = false;
	bool bIsAutoMoving      = false;

protected:
	UFUNCTION()
	void OnItemPickedUp(FName ItemID, int32 Count);
};
