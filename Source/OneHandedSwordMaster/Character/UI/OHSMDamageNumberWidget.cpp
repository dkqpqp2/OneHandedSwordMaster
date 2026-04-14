// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMDamageNumberWidget.h"

#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"

void UOHSMDamageNumberWidget::ShowDamage(float Amount, APlayerController* InPC, FVector InWorldLocation)
{
	OwnerPC       = InPC;
	WorldLocation = InWorldLocation;
	ElapsedTime   = 0.f;
	bStarted      = true;

	if (TextDamage)
	{
		// 데미지 수치 표시 (소수점 없이)
		TextDamage->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Amount)));
		TextDamage->SetColorAndOpacity(NormalColor);
	}

	SetIsFocusable(false);
}

void UOHSMDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bStarted || !IsValid(OwnerPC)) return;

	ElapsedTime += InDeltaTime;

	// 진행률 (0 → 1)
	const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);

	// 월드 좌표 → 화면 좌표
	FVector2D ScreenPos;
	const bool bProjected = UGameplayStatics::ProjectWorldToScreen(OwnerPC, WorldLocation, ScreenPos);

	if (bProjected)
	{
		// 위로 떠오르기 (Alpha에 비례)
		ScreenPos.Y -= FloatDistance * Alpha;

		// DPI 스케일 역보정
		const float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
		ScreenPos /= DPIScale;

		SetPositionInViewport(ScreenPos, false);
	}

	// 후반부 0.4초 동안 서서히 투명해짐
	const float FadeStart = 1.f - (0.4f / Duration);
	if (Alpha >= FadeStart)
	{
		const float FadeAlpha = (Alpha - FadeStart) / (1.f - FadeStart);
		SetRenderOpacity(1.f - FadeAlpha);
	}

	// 애니메이션 완료 → 위젯 제거
	if (ElapsedTime >= Duration)
	{
		RemoveFromParent();
	}
}
