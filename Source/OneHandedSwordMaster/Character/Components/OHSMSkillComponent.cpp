// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMSkillComponent.h"

#include "OneHandedSwordMaster/Character/Skills/OHSMSkillBase.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "GameFramework/Character.h"

UOHSMSkillComponent::UOHSMSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOHSMSkillComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ─── 스킬 트리 ──────────────────────────────────────────────────────

bool UOHSMSkillComponent::CanLearnSkill(FName SkillID) const
{
	const FOHSMSkillData* Data = GetSkillData(SkillID);
	if (!Data) return false;

	// 이미 배운 스킬
	if (IsSkillLearned(SkillID)) return false;

	// 스킬 포인트 부족
	if (AvailableSkillPoints < Data->SkillPointCost) return false;

	// 전제 스킬 확인
	for (const FName& PrereqID : Data->PrerequisiteSkillIDs)
	{
		if (!IsSkillLearned(PrereqID)) return false;
	}

	return true;
}

bool UOHSMSkillComponent::LearnSkill(FName SkillID)
{
	if (!CanLearnSkill(SkillID)) return false;

	const FOHSMSkillData* Data = GetSkillData(SkillID);
	AvailableSkillPoints -= Data->SkillPointCost;

	LearnedSkills.Add(SkillID);

	// 패시브 스킬이면 습득 즉시 영구 효과 적용
	if (Data->SkillType == ESkillType::Passive)
	{
		ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
		if (IsValid(OwnerChar))
		{
			UOHSMSkillBase* Instance = GetOrCreateSkillInstance(SkillID);
			if (Instance)
			{
				Instance->OnLearned(OwnerChar);
			}
		}
	}

	OnSkillLearned.Broadcast(SkillID);
	OnSkillPointsChanged.Broadcast();

	return true;
}

bool UOHSMSkillComponent::IsSkillLearned(FName SkillID) const
{
	return LearnedSkills.Contains(SkillID);
}

// ─── 스킬 사용 ──────────────────────────────────────────────────────

bool UOHSMSkillComponent::ActivateSkill(FName SkillID)
{
	if (!IsSkillLearned(SkillID)) return false;

	const FOHSMSkillData* Data = GetSkillData(SkillID);
	if (!Data) return false;

	// 패시브 스킬은 수동 발동 불가 (습득 시 자동 적용됨)
	if (Data->SkillType == ESkillType::Passive) return false;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	const float Now = World->GetTimeSeconds();

	// 쿨다운 검사
	UOHSMSkillBase* Instance = GetOrCreateSkillInstance(SkillID);
	if (!Instance) return false;

	if (Instance->IsOnCooldown(Now)) return false;

	// 마나 소모
	if (Data->ManaCost > 0.f)
	{
		UOHSMPlayerStatComponent* StatComp =
			OwnerChar->FindComponentByClass<UOHSMPlayerStatComponent>();
		if (!IsValid(StatComp)) return false;
		if (!StatComp->UseMana(Data->ManaCost)) return false;
	}

	// 쿨다운 시작
	Instance->StartCooldown(Now, Data->Cooldown);

	// 스킬 발동
	Instance->ActivateSkill(OwnerChar);

	// 쿨다운 UI 갱신을 위해 브로드캐스트
	OnSkillActivated.Broadcast(SkillID, Data->Cooldown);

	// 버프 스킬이면 버프 상태 UI 표시를 위해 브로드캐스트
	if (Data->SkillType == ESkillType::Buff)
	{
		OnBuffApplied.Broadcast(SkillID, Data->SkillIcon, Data->Duration);
	}

	return true;
}

void UOHSMSkillComponent::BroadcastBuffExpired(FName SkillID)
{
	OnBuffExpired.Broadcast(SkillID);
}

bool UOHSMSkillComponent::IsSkillOnCooldown(FName SkillID) const
{
	if (const TObjectPtr<UOHSMSkillBase>* Found = SkillInstances.Find(SkillID))
	{
		if (const UWorld* World = GetWorld())
		{
			return (*Found)->IsOnCooldown(World->GetTimeSeconds());
		}
	}
	return false;
}

float UOHSMSkillComponent::GetRemainingCooldown(FName SkillID) const
{
	if (const TObjectPtr<UOHSMSkillBase>* Found = SkillInstances.Find(SkillID))
	{
		if (const UWorld* World = GetWorld())
		{
			const float Remaining = (*Found)->CooldownEndTime - World->GetTimeSeconds();
			return FMath::Max(0.f, Remaining);
		}
	}
	return 0.f;
}

// ─── 스킬 포인트 ────────────────────────────────────────────────────

void UOHSMSkillComponent::AddSkillPoints(int32 Amount)
{
	if (Amount <= 0) return;
	AvailableSkillPoints += Amount;
	OnSkillPointsChanged.Broadcast();
}

// ─── 조회 ───────────────────────────────────────────────────────────

const FOHSMSkillData* UOHSMSkillComponent::GetSkillData(FName SkillID) const
{
	if (!SkillDataTable || SkillID.IsNone()) return nullptr;
	return SkillDataTable->FindRow<FOHSMSkillData>(SkillID, TEXT("UOHSMSkillComponent"));
}

TArray<FName> UOHSMSkillComponent::GetAllSkillIDs() const
{
	if (!SkillDataTable) return {};
	return SkillDataTable->GetRowNames();
}

// ─── 내부 헬퍼 ──────────────────────────────────────────────────────

UOHSMSkillBase* UOHSMSkillComponent::GetOrCreateSkillInstance(FName SkillID)
{
	if (TObjectPtr<UOHSMSkillBase>* Found = SkillInstances.Find(SkillID))
	{
		return *Found;
	}

	const FOHSMSkillData* Data = GetSkillData(SkillID);
	if (!Data || !Data->SkillClass) return nullptr;

	UOHSMSkillBase* Instance = NewObject<UOHSMSkillBase>(this, Data->SkillClass);
	Instance->SkillID   = SkillID;
	Instance->SkillData = Data;

	SkillInstances.Add(SkillID, Instance);
	return Instance;
}
