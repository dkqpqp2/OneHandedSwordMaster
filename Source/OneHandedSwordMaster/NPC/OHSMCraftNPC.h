#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMInteractableInterface.h"
#include "OHSMCraftNPC.generated.h"

class USphereComponent;
class UWidgetComponent;
class UOHSMNPCDialogueWidget;

UCLASS()
class ONEHANDEDSWORDMASTER_API AOHSMCraftNPC : public ACharacter, public IOHSMInteractableInterface
{
	GENERATED_BODY()

public:
	AOHSMCraftNPC();

protected:
	virtual void BeginPlay() override;

public:
	/** 플레이어가 E 키를 눌렀을 때 호출 — 대화창 열기 */
	virtual void Interact_Implementation(APlayerController* InstigatorController) override;

	// ─── 상호작용 범위 ────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<USphereComponent> InteractionRange;

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ─── 상호작용 키 안내 위젯 ────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<UWidgetComponent> InteractPromptComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractPromptWidgetClass;

	// ─── 대화창 ──────────────────────────────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Dialogue")
	TSubclassOf<UOHSMNPCDialogueWidget> DialogueWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Dialogue")
	FText DialogueMessage = FText::FromString(TEXT("어떤 아이템을 만드시겠습니까?"));

private:
	UPROPERTY()
	TObjectPtr<UOHSMNPCDialogueWidget> DialogueWidget;

	/** 현재 상호작용 중인 PlayerController */
	UPROPERTY()
	TObjectPtr<APlayerController> CurrentInstigator;

	/** 대화창 확인 버튼 → CraftPanel 오픈 */
	void OnDialogueConfirmed();

	/** 대화창 취소 버튼 → 닫기 */
	void OnDialogueCancelled();
};
