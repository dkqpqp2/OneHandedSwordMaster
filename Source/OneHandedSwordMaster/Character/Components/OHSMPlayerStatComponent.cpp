// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Data/OHSMPlayerStatData.h"

// 스탯 컴포넌트 초기화
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

// 시작 시 HP/마나 최대치 설정
void UOHSMPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyLevelStats(); // 레벨 스탯 적용

	float MaxHp = GetMaxHp();
	SetHp(MaxHp); // HP 최대치로 초기화

	float MaxMana = GetMaxMana();
	CurrentMana = MaxMana; // 마나 최대치로 초기화
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}


float UOHSMPlayerStatComponent::GetMaxHp() const
{
	return GetStat(EPlayerStatType::MaxHealth);
}

// 데미지 적용 후 실제 피해량 반환
float UOHSMPlayerStatComponent::ApplyDamage(float InDamage, AActor* Attacker)
{
	if (CurrentHp <= 0.0f) // 이미 사망 상태
	{
		return 0.0f;
	}

	// 방어력 적용: 받는 데미지 = 공격력 - 방어력 (최소 1은 들어옴)
	const float Defense     = GetDefensePower();
	const float ActualDamage = FMath::Max(1.0f, InDamage - Defense);

	const float PrevHp = CurrentHp;
	SetHp(PrevHp - ActualDamage, ActualDamage, Attacker);

	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast(); // HP 0 알림
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

void UOHSMPlayerStatComponent::RestoreFromSave(int32 InLevel, int32 InExp, float InHp, float InMana)
{
	CurrentLevel = FMath::Max(1, InLevel);
	CurrentExp   = FMath::Max(0, InExp);

	// 해당 레벨의 DataTable 스탯 재적용
	ApplyLevelStats();

	const float MaxHp   = GetMaxHp();
	const float MaxMana = GetMaxMana();

	// HP / 마나 복원 (-1 이면 최대치)
	SetHp  (InHp   >= 0.f ? FMath::Min(InHp,   MaxHp)   : MaxHp);
	SetMana(InMana >= 0.f ? FMath::Min(InMana, MaxMana) : MaxMana);

	// UI 갱신
	OnExpChanged.Broadcast(CurrentExp, RequiredExp);
	OnLevelUp.Broadcast(CurrentLevel, CurrentLevel);
}

// 경험치 추가 및 레벨업 체크
void UOHSMPlayerStatComponent::AddExperience(int32 Amount)
{
	// 만렙이면 경험치 무시
	const int32 MaxLevel = PlayerStatTable ? PlayerStatTable->GetRowNames().Num() : 5;
	if (CurrentLevel >= MaxLevel)
	{
		return;
	}

	CurrentExp += Amount; // 경험치 누적

	OnExpChanged.Broadcast(CurrentExp, RequiredExp); // UI 갱신

	// 레벨업 체크 (만렙 초과 방지)
	while (CurrentExp >= RequiredExp && CurrentLevel < MaxLevel)
	{
		LevelUp(); // 레벨업 처리
	}
}

// 레벨 1 상승 처리
void UOHSMPlayerStatComponent::LevelUp()
{
	int32 OldLevel = CurrentLevel;
	CurrentLevel++; // 레벨 증가

	CurrentExp -= RequiredExp; // 초과 경험치 유지

	ApplyLevelStats(); // 새 레벨 스탯 적용

	float NewMaxHp = GetMaxHp();
	float NewMaxMana = GetMaxMana();

	CurrentHp = NewMaxHp; // HP 최대치 회복
	CurrentMana = NewMaxMana; // 마나 최대치 회복

	OnHpChanged.Broadcast(CurrentHp, NewMaxHp);
	OnManaChanged.Broadcast(CurrentMana, NewMaxMana);

	OnLevelUp.Broadcast(CurrentLevel, OldLevel); // 레벨업 알림
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

		if (StatType == EPlayerStatType::MaxHealth)
		{
			CurrentHp = FMath::Min(CurrentHp, GetMaxHp());
			OnHpChanged.Broadcast(CurrentHp, GetMaxHp());
		}
		else if (StatType == EPlayerStatType::MaxMana)
		{
			OnManaChanged.Broadcast(CurrentMana, GetMaxMana());
		}
		
		OnStatChanged.Broadcast();
	}
}

void UOHSMPlayerStatComponent::RemoveEquipmentBonus(EPlayerStatType StatType, float Amount)
{
	AddEquipmentBonus(StatType, -Amount);
}

void UOHSMPlayerStatComponent::AddTemporaryBonus(EPlayerStatType StatType, float Amount)
{
	FPlayerStat& Stat = PlayerStats.FindOrAdd(StatType);
	Stat.TemporaryBonus += Amount;

	// 버프 적용/해제 시 장비창 즉시 갱신
	OnStatChanged.Broadcast();
}

void UOHSMPlayerStatComponent::RemoveTemporaryBonus(EPlayerStatType StatType, float Amount)
{
	AddTemporaryBonus(StatType, -Amount);
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


