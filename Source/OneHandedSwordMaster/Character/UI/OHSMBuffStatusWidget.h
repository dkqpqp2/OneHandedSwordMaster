// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMBuffStatusWidget.generated.h"

/**
 * 화면 좌상단에 표시되는 버프 1개 아이콘 위젯.
 *
 * [Blueprint BindWidget]
 *   Img_BuffIcon   - 스킬 아이콘 이미지
 *   Bar_Duration   - 남은 시간 진행 바 (1→0 으로 감소)
 *   Text_Duration  - 남은 시간 텍스트 (예: "8.5s")
 *
 * NativeTick 으로 매 프레임 갱신되며, 시간이 0 이 되면 스스로 숨김 처리한다.
 * 외부(OHSMBuffBarWidget)에서 버프 만료 신호를 받으면 ForceExpire() 로 즉시 제거.
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMBuffStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 버프 아이콘 초기화
	 * @param InSkillID   버프 스킬 Row 이름 (식별자)
	 * @param InIcon      표시할 아이콘 텍스처
	 * @param InDuration  버프 총 지속 시간 (초)
	 */
	void InitBuff(FName InSkillID, UTexture2D* InIcon, float InDuration);

	/** 현재 표시 중인 스킬 ID */
	FName GetSkillID() const { return SkillID; }

	/** 버프가 외부(컴포넌트)에서 강제 만료됐을 때 호출 — 위젯 즉시 숨김 */
	void ForceExpire();

protected:
	// ─── BindWidget ──────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_BuffIcon;

	/**
	 * 시계 방향 라디얼 타이머 이미지.
	 * M_RadialCooldown(또는 동일 구조) 머티리얼을 적용하고
	 * Progress 파라미터를 1→0 으로 줄여 남은 시간을 표현한다.
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_TimerRadial;

	/** 남은 시간 텍스트 ("8.5s" 형식) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Duration;

	/**
	 * BP 디폴트에서 라디얼 타이머 머티리얼 지정.
	 * M_RadialCooldown 과 동일 구조(스칼라 파라미터 "Progress") 사용.
	 * 쿨다운과 색상을 다르게 하려면 별도 머티리얼(M_RadialBuff 등)을 만들면 된다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Buff|FX")
	TObjectPtr<class UMaterialInterface> TimerMaterial;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	FName  SkillID;
	float  TotalDuration  = 0.f;
	float  RemainingTime  = 0.f;
	bool   bExpired       = false;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> TimerMID;
};
