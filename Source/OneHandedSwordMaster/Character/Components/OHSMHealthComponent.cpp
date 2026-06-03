// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMHealthComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

UOHSMHealthComponent::UOHSMHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


// HP 최대치 초기화
void UOHSMHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth; // 최대 HP로 시작
}

// 데미지 수신 및 처리
void UOHSMHealthComponent::TakeDamage(float Damage, AActor* DamageCauser)
{
	if (bIsDead || Damage <= 0.0f) // 사망 또는 유효하지 않은 데미지
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth); // HP 차감

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, Damage, DamageCauser);

	// 데미지 숫자 위젯 스폰
	if (DamageNumberWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC))
		{
			UOHSMDamageNumberWidget* DmgWidget =
				CreateWidget<UOHSMDamageNumberWidget>(PC, DamageNumberWidgetClass);
			if (DmgWidget)
			{
				// 피격 위치 위 100cm에 표시
				const FVector SpawnLoc = GetOwner()
					? GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 100.f)
					: FVector::ZeroVector;

				DmgWidget->AddToViewport(10);
				DmgWidget->ShowDamage(Damage, PC, SpawnLoc);
			}
		}
	}

	if (CurrentHealth <= 0.0f)
	{
		Die(DamageCauser);
	}
}

// HP 회복
void UOHSMHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth); // HP 증가

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, -Amount, nullptr);
}

// 사망 처리
void UOHSMHealthComponent::Die(AActor* Killer)
{
	if (bIsDead) // 중복 방지
	{
		return;
	}

	bIsDead = true; // 사망 플래그
	CurrentHealth = 0.0f;

	OnDeath.Broadcast(Killer); // 사망 알림
}


