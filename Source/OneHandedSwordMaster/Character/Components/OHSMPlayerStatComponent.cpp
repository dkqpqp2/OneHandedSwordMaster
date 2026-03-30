// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Data/OHSMPlayerStatData.h"

// Sets default values for this component's properties
UOHSMPlayerStatComponent::UOHSMPlayerStatComponent()
{
	
	CurrentHp = 0.0f;
	
	// bWantsInitializeComponen = true; 해야 함수 실행함.
	bWantsInitializeComponent = true;
}

void UOHSMPlayerStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	InitializeStats();
}

void UOHSMPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyLevelStats();
	
	float MaxHp = GetMaxHp();
	SetHp(MaxHp);
	
	float MaxMana = GetMaxMana();
	CurrentMana = MaxMana;
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}


float UOHSMPlayerStatComponent::GetMaxHp() const
{
	return GetStat(EPlayerStatType::MaxHealth);
}

float UOHSMPlayerStatComponent::ApplyDamage(float InDamage, AActor* Attacker)
{
	if (CurrentHp <= 0.0f)
	{
		return 0.0f;
	}
	
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);
	
	SetHp(PrevHp - ActualDamage, ActualDamage, Attacker);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}
	
	return ActualDamage;
}

void UOHSMPlayerStatComponent::Heal(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}
    
	float MaxHP = GetMaxHp();
	SetHp(FMath::Min(CurrentHp + Amount, MaxHP));
}

void UOHSMPlayerStatComponent::SetHp(float NewHp, float Damage, AActor* Attacker)
{
	float MaxHP = GetMaxHp();
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHP);
 
	OnHpChanged.Broadcast(CurrentHp, MaxHP);
}

bool UOHSMPlayerStatComponent::UseMana(float Amount)
{
	if (CurrentMana < Amount)
	{
		return false;
	}
	
	SetMana(CurrentMana - Amount);
	
	return true;
}

void UOHSMPlayerStatComponent::RestoreMana(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}
	
	float MaxMana = GetMaxMana();
	SetMana(FMath::Min(CurrentMana + Amount, MaxMana));
}

void UOHSMPlayerStatComponent::SetMana(float NewMana)
{
	float MaxMana = GetMaxMana();
	
	CurrentMana = FMath::Clamp(NewMana, 0.0f, MaxMana);
	
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

void UOHSMPlayerStatComponent::AddExperience(int32 Amount)
{
	CurrentExp += Amount;
	
	OnExpChanged.Broadcast(CurrentExp, RequiredExp);
    
	// 레벨업 체크
	while (CurrentExp >= RequiredExp)
	{
		LevelUp();
	}
}

void UOHSMPlayerStatComponent::LevelUp()
{
	int32 OldLevel = CurrentLevel;
	CurrentLevel++;
    
	CurrentExp -= RequiredExp;
	
	ApplyLevelStats();
    
	float NewMaxHp = GetMaxHp();
	float NewMaxMana = GetMaxMana();
	
	CurrentHp = NewMaxHp; 
	CurrentMana = NewMaxMana;
	
	OnHpChanged.Broadcast(CurrentHp, NewMaxHp);
	OnManaChanged.Broadcast(CurrentMana, NewMaxMana);
    
	OnLevelUp.Broadcast(CurrentLevel, OldLevel);
	OnExpChanged.Broadcast(CurrentExp, RequiredExp);
}

float UOHSMPlayerStatComponent::GetExpPercent() const
{
	if (RequiredExp <= 0)
	{
		return 0.0f;
	}
    
	return static_cast<float>(CurrentExp) / static_cast<float>(RequiredExp);
}

float UOHSMPlayerStatComponent::GetStat(EPlayerStatType StatType) const
{
	if (const FPlayerStat* Stat = PlayerStats.Find(StatType))
	{
		return Stat->GetFinalValue();
	}
    
	return 0.0f;
}

void UOHSMPlayerStatComponent::SetBaseStat(EPlayerStatType StatType, float Value)
{
	if (FPlayerStat* Stat = PlayerStats.Find(StatType))
	{
		Stat->BaseValue = Value;
	}
}

void UOHSMPlayerStatComponent::AddEquipmentBonus(EPlayerStatType StatType, float Amount)
{
	if (FPlayerStat* Stat = PlayerStats.Find(StatType))
	{
		Stat->EquipmentBonus += Amount;

		// HP/Mana 최댓값이 바뀌면 UI 갱신
		if (StatType == EPlayerStatType::MaxHealth)
		{
			OnHpChanged.Broadcast(CurrentHp, GetMaxHp());
		}
		else if (StatType == EPlayerStatType::MaxMana)
		{
			OnManaChanged.Broadcast(CurrentMana, GetMaxMana());
		}
	}
}

void UOHSMPlayerStatComponent::RemoveEquipmentBonus(EPlayerStatType StatType, float Amount)
{
	AddEquipmentBonus(StatType, -Amount);
}

void UOHSMPlayerStatComponent::InitializeStats()
{
	PlayerStats.Add(EPlayerStatType::MaxHealth, FPlayerStat(100.0f));
	PlayerStats.Add(EPlayerStatType::MaxMana, FPlayerStat(50.0f));
	PlayerStats.Add(EPlayerStatType::MaxStamina, FPlayerStat(100.0f));
    
	PlayerStats.Add(EPlayerStatType::Damage, FPlayerStat(10.0f));
	PlayerStats.Add(EPlayerStatType::Defense, FPlayerStat(5.0f));
	PlayerStats.Add(EPlayerStatType::CriticalChance, FPlayerStat(5.0f));
	PlayerStats.Add(EPlayerStatType::CriticalDamage, FPlayerStat(150.0f));
    
	PlayerStats.Add(EPlayerStatType::HealthRegen, FPlayerStat(1.0f));
	PlayerStats.Add(EPlayerStatType::ManaRegen, FPlayerStat(1.0f));
	PlayerStats.Add(EPlayerStatType::StaminaRegen, FPlayerStat(10.0f));
    
	PlayerStats.Add(EPlayerStatType::MoveSpeed, FPlayerStat(500.0f));
	PlayerStats.Add(EPlayerStatType::AttackSpeed, FPlayerStat(1.0f));
	PlayerStats.Add(EPlayerStatType::CooldownReduction, FPlayerStat(0.0f));
}

void UOHSMPlayerStatComponent::ApplyLevelStats()
{
	if (!PlayerStatTable)
	{
		return;
	}
	
	FName RowName = FName(*FString::Printf(TEXT("Level_%d"), CurrentLevel));
	
	FString ContextString;
	FPlayerStatTableRow* Row = PlayerStatTable->FindRow<FPlayerStatTableRow>(RowName, ContextString);
    
	if (Row)
	{
		RequiredExp = Row->RequiredExp;
        
		// DataTable 값 적용
		SetBaseStat(EPlayerStatType::MaxHealth, Row->BaseHealth);
		SetBaseStat(EPlayerStatType::MaxMana, Row->BaseMana);
		SetBaseStat(EPlayerStatType::MaxStamina, Row->BaseStamina);
		SetBaseStat(EPlayerStatType::Damage, Row->BaseAttack);
		SetBaseStat(EPlayerStatType::Defense, Row->BaseDefense);
	}
}


