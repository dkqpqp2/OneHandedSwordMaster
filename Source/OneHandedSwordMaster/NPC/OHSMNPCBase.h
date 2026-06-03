#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Interface/OHSMInteractableInterface.h"
#include "OHSMNPCBase.generated.h"

class USphereComponent;
class UWidgetComponent;
class UOHSMNPCDialogueWidget;
class UOHSMNPCInfoWidget;

UCLASS(Abstract)
class ONEHANDEDSWORDMASTER_API AOHSMNPCBase : public ACharacter, public IOHSMInteractableInterface
{
	GENERATED_BODY()

public:
	AOHSMNPCBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	/** IOHSMInteractableInterface — 플레이어가 E 키를 눌렀을 때 */
	virtual void Interact_Implementation(APlayerController* InstigatorController) override;

	// ─── 공통 설정 ────────────────────────────────────────────────
protected:
	// NPC 이름
	UPROPERTY(EditAnywhere, Category = "NPC")
	FText NPCName = FText::FromString(TEXT("NPC"));

	// 상호작용 범위
	UPROPERTY(EditAnywhere, Category = "NPC|Interaction")
	float InteractionRadius = 200.f;

	// 이름 표시 범위
	UPROPERTY(EditAnywhere, Category = "NPC|UI")
	float NameVisibilityRadius = 600.f;

	// 대화 메시지
	UPROPERTY(EditAnywhere, Category = "NPC|Dialogue")
	FText DialogueMessage;

	// 확인 버튼 텍스트
	UPROPERTY(EditAnywhere, Category = "NPC|Dialogue")
	FText ConfirmButtonText = FText::FromString(TEXT("확인"));

	// 취소 버튼 텍스트
	UPROPERTY(EditAnywhere, Category = "NPC|Dialogue")
	FText CancelButtonText = FText::FromString(TEXT("취소"));

	// ─── 자식 클래스 오버라이드 ───────────────────────────────────
protected:
	/** 대화창 확인 버튼 눌렸을 때 — 자식 클래스에서 구현 */
	virtual void OnDialogueConfirmed(APlayerController* InstigatorController) {}

	/** 대화창 취소 버튼 눌렸을 때 — 필요 시 오버라이드 */
	virtual void OnDialogueCancelled(APlayerController* InstigatorController) {}

	// ─── 컴포넌트 ─────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	TObjectPtr<USphereComponent> InteractionRange;

	/** 이름 위젯 표시 범위 구체 (NameVisibilityRadius) */
	UPROPERTY(VisibleAnywhere, Category = "NPC|UI")
	TObjectPtr<USphereComponent> VisibilityRange;

	/** 머리 위 정보 위젯 컴포넌트 (이름 + 퀘스트 인디케이터) */
	UPROPERTY(VisibleAnywhere, Category = "NPC|UI")
	TObjectPtr<UWidgetComponent> InfoWidgetComponent;

protected:
	/** InfoWidgetComponent 에서 UOHSMNPCInfoWidget 반환 (없으면 nullptr) */
	UOHSMNPCInfoWidget* GetInfoWidget() const;

	/** 화면에 표시할 상호작용 힌트 텍스트 (기본: "[G] 상호작용") */
	UPROPERTY(EditAnywhere, Category = "NPC|Interaction")
	FText InteractionHintText = FText::FromString(TEXT("[G] 대화하기"));

	UPROPERTY(EditDefaultsOnly, Category = "NPC|Dialogue")
	TSubclassOf<UOHSMNPCDialogueWidget> DialogueWidgetClass;

	// ─── 내부 ─────────────────────────────────────────────────────
private:
	UPROPERTY()
	TObjectPtr<UOHSMNPCDialogueWidget> ActiveDialogueWidget;

	UPROPERTY()
	TObjectPtr<APlayerController> CachedInstigator;

	void OpenDialogue(APlayerController* InstigatorController);
	void CloseDialogue();

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 이름 위젯 표시 범위 오버랩
	UFUNCTION()
	void OnVisibilityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVisibilityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 대화창 버튼 콜백
	void HandleConfirmed();
	void HandleCancelled();
};
