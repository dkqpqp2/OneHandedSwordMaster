// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMHealthComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

UOHSMHealthComponent::UOHSMHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UOHSMHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	
}

void UOHSMHealthComponent::TakeDamage(float Damage, AActor* DamageCauser)
{
	if (bIsDead || Damage <= 0.0f)
	{
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

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

void UOHSMHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.0f)
	{
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, -Amount, nullptr);
}

void UOHSMHealthComponent::Die(AActor* Killer)
{
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;
	CurrentHealth = 0.0f;
	
	OnDeath.Broadcast(Killer);
}


