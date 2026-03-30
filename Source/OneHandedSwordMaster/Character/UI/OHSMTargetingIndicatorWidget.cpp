// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMTargetingIndicatorWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SkeletalMeshComponent.h"

void UOHSMTargetingIndicatorWidget::SetTarget(AActor* InTarget)
{
	Target = InTarget;
	SetVisibility(Target ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UOHSMTargetingIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Target)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	// spine_02 소켓 위치 사용 (없으면 머리 위 오프셋으로 대체)
	FVector WorldLocation = GetTargetWorldLocation();

	FVector2D ScreenPosition;
	bool bProjected = PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition);

	if (!bProjected)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	// DPI 스케일 반영
	float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	ScreenPosition /= DPIScale;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (CanvasSlot)
	{
		CanvasSlot->SetPosition(ScreenPosition);
	}
}

FVector UOHSMTargetingIndicatorWidget::GetTargetWorldLocation() const
{
	// 스켈레탈 메시에서 spine_02 소켓 위치 조회
	ACharacter* TargetChar = Cast<ACharacter>(Target);
	if (TargetChar)
	{
		USkeletalMeshComponent* Mesh = TargetChar->GetMesh();
		if (Mesh && Mesh->DoesSocketExist(SocketName))
		{
			return Mesh->GetSocketLocation(SocketName);
		}
	}

	// 소켓이 없으면 머리 위 오프셋으로 대체
	return Target->GetActorLocation() + FVector(0.0f, 0.0f, WorldHeightOffset);
}
