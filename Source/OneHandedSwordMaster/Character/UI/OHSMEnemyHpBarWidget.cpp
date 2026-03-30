// Fill out your copyright notice in the Description page of Project Settings.

#include "OHSMEnemyHpBarWidget.h"

#include "Components/ProgressBar.h"

void UOHSMEnemyHpBarWidget::UpdateHpBar(float CurrentHealth, float MaxHealth, float Damage, AActor* DamageCauser)
{
	if (HpProgressBar && MaxHealth > 0.0f)
	{
		HpProgressBar->SetPercent(CurrentHealth / MaxHealth);
	}
}
