// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBuffStatusWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void UOHSMBuffStatusWidget::InitBuff(FName InSkillID, UTexture2D* InIcon, float InDuration)
{
	SkillID       = InSkillID;
	TotalDuration = InDuration;
	RemainingTime = InDuration;
	bExpired      = false;

	// 아이콘 설정
	if (Img_BuffIcon && InIcon)
	{
		Img_BuffIcon->SetBrushFromTexture(InIcon);
	}

	// 라디얼 타이머 머티리얼 인스턴스 생성 (최초 1회)
	if (TimerMaterial && Img_TimerRadial)
	{
		if (!IsValid(TimerMID))
		{
			TimerMID = UMaterialInstanceDynamic::Create(TimerMaterial, this);
			Img_TimerRadial->SetBrushFromMaterial(TimerMID);
		}

		// 버프 시작 → Progress = 1 (꽉 찬 원)
		TimerMID->SetScalarParameterValue(TEXT("Progress"), 1.f);
		Img_TimerRadial->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// 텍스트 초기화
	if (Text_Duration)
	{
		Text_Duration->SetText(
			FText::FromString(FString::Printf(TEXT("%.1fs"), InDuration)));
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UOHSMBuffStatusWidget::ForceExpire()
{
	bExpired = true;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UOHSMBuffStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bExpired || TotalDuration <= 0.f)
	{
		return;
	}

	RemainingTime -= InDeltaTime;

	if (RemainingTime <= 0.f)
	{
		RemainingTime = 0.f;
		bExpired      = true;
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Progress: 1(시작) → 0(만료), 시계 방향으로 원이 줄어듦
	const float Progress = RemainingTime / TotalDuration;

	if (IsValid(TimerMID))
	{
		TimerMID->SetScalarParameterValue(TEXT("Progress"), Progress);
	}

	// 남은 시간 텍스트
	if (Text_Duration)
	{
		Text_Duration->SetText(
			FText::FromString(FString::Printf(TEXT("%.1fs"), RemainingTime)));
	}
}
