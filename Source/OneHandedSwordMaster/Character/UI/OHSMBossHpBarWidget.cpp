// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMBossHpBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "OneHandedSwordMaster/Character/Enemy/OHSMEnemyBase.h"
#include "OneHandedSwordMaster/Character/Components/OHSMHealthComponent.h"

void UOHSMBossHpBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 숨김
	SetVisibility(ESlateVisibility::Hidden);
}

void UOHSMBossHpBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// SearchInterval마다 보스 탐색
	SearchTimer += InDeltaTime;
	if (SearchTimer < SearchInterval)
	{
		return;
	}
	SearchTimer = 0.0f;

	FindNearestBoss();
}

// ============================================================
// 근처 보스 탐색
// ============================================================
void UOHSMBossHpBarWidget::FindNearestBoss()
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn)
	{
		SetCurrentBoss(nullptr);
		return;
	}

	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOHSMEnemyBase::StaticClass(), AllEnemies);

	AOHSMEnemyBase* NearestBoss = nullptr;
	float NearestDist = FLT_MAX;

	for (AActor* Actor : AllEnemies)
	{
		AOHSMEnemyBase* Enemy = Cast<AOHSMEnemyBase>(Actor);
		if (!Enemy || !Enemy->bIsBoss || Enemy->IsDead())
		{
			continue;
		}

		float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), Enemy->GetActorLocation());
		if (Dist <= BossHpBarRange && Dist < NearestDist)
		{
			NearestDist = Dist;
			NearestBoss = Enemy;
		}
	}

	SetCurrentBoss(NearestBoss);
}

// ============================================================
// 보스 설정 (nullptr이면 숨김)
// ============================================================
void UOHSMBossHpBarWidget::SetCurrentBoss(AOHSMEnemyBase* NewBoss)
{
	// 같은 보스면 패스
	if (CurrentBoss == NewBoss)
	{
		return;
	}

	// ─── 기존 보스 구독 해제 ───
	if (CurrentBoss)
	{
		UOHSMHealthComponent* OldHealth = CurrentBoss->GetHealthComponent();
		if (OldHealth)
		{
			OldHealth->OnHealthChanged.RemoveDynamic(this, &UOHSMBossHpBarWidget::OnBossHpChanged);
			OldHealth->OnDeath.RemoveDynamic(this, &UOHSMBossHpBarWidget::OnBossDied);
		}
	}

	CurrentBoss = NewBoss;

	// ─── 보스 없음 → 숨김 ───
	if (!CurrentBoss)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	// ─── 새 보스 구독 ───
	UOHSMHealthComponent* NewHealth = CurrentBoss->GetHealthComponent();
	if (NewHealth)
	{
		// 초기 체력값 즉시 반영
		if (BossHpProgressBar)
		{
			BossHpProgressBar->SetPercent(NewHealth->GetHealthPercent());
		}
		if (TextBossHp)
		{
			TextBossHp->SetText(FText::FromString(
				FString::Printf(TEXT("%.0f / %.0f"),
					NewHealth->GetCurrentHealth(),
					NewHealth->GetMaxHealth())
			));
		}

		NewHealth->OnHealthChanged.AddDynamic(this, &UOHSMBossHpBarWidget::OnBossHpChanged);
		NewHealth->OnDeath.AddDynamic(this, &UOHSMBossHpBarWidget::OnBossDied);
	}

	// ─── 보스 이름 설정 ───
	if (TextBossName)
	{
		// BossDisplayName이 비어있으면 Actor 이름 사용
		TextBossName->SetText(
			CurrentBoss->BossDisplayName.IsEmpty()
			? FText::FromString(CurrentBoss->GetName())
			: CurrentBoss->BossDisplayName
		);
	}

	SetVisibility(ESlateVisibility::Visible);
}

// ============================================================
// HealthComponent 콜백
// ============================================================
void UOHSMBossHpBarWidget::OnBossHpChanged(float CurrentHp, float MaxHp, float Damage, AActor* DamageCauser)
{
	if (BossHpProgressBar && MaxHp > 0.0f)
	{
		BossHpProgressBar->SetPercent(CurrentHp / MaxHp);
	}

	if (TextBossHp)
	{
		TextBossHp->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), CurrentHp, MaxHp)
		));
	}
}

void UOHSMBossHpBarWidget::OnBossDied(AActor* Killer)
{
	// 보스 사망 → 바 숨김
	SetCurrentBoss(nullptr);
}
