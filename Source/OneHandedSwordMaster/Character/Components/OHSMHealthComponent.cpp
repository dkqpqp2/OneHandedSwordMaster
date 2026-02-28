// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMHealthComponent.h"
#include "GameFramework/Character.h"

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


