// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"
#include "OHSMSkillComponent.generated.h"

class UOHSMSkillBase;

DECLARE_MULTICAST_DELEGATE(FOnSkillPointsChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillLearned, FName /*SkillID*/);
/** 스킬 발동 성공 시 — 쿨다운 UI 갱신에 사용 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillActivated, FName /*SkillID*/, float /*CooldownDuration*/);
/** 버프 스킬 발동 시 — 버프 상태 UI 표시용 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBuffApplied, FName /*SkillID*/, UTexture2D* /*Icon*/, float /*Duration*/);
/** 버프 만료 시 — 버프 상태 UI 제거용 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBuffExpired, FName /*SkillID*/);

/**
 * 스킬 시스템 컴포넌트.
 *
 * - DataTable(DT_SkillData)에서 스킬 정보를 읽는다.
 * - 스킬 포인트로 스킬을 습득하며, 전제 스킬(PrerequisiteSkillIDs)을 모두
 *   배워야 상위 스킬을 배울 수 있다 (트리 구조).
 * - 마나 소모 및 쿨다운을 처리한 뒤 UOHSMSkillBase::ActivateSkill을 호출한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOHSMSkillComponent();

protected:
	virtual void BeginPlay() override;

	// 스킬 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TObjectPtr<UDataTable> SkillDataTable;

public:
	// ─── 스킬 트리 ─────────────────────────────────────────────────

	// 습득 가능 여부 확인
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool CanLearnSkill(FName SkillID) const;

	// 스킬 습득
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool LearnSkill(FName SkillID);

	// 습득 여부 확인
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillLearned(FName SkillID) const;

	// ─── 스킬 사용 ─────────────────────────────────────────────────

	// 스킬 발동
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ActivateSkill(FName SkillID);

	// 쿨다운 중 여부
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillOnCooldown(FName SkillID) const;

	// 남은 쿨다운 시간
	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetRemainingCooldown(FName SkillID) const;

	// ─── 스킬 포인트 ───────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetAvailableSkillPoints() const { return AvailableSkillPoints; }

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void AddSkillPoints(int32 Amount);

	// ─── 조회 ─────────────────────────────────────────────────────

	// 스킬 데이터 조회
	const FOHSMSkillData* GetSkillData(FName SkillID) const;

	// 저장 데이터로 복원
	void RestoreFromSave(const TArray<FName>& SkillIDs, int32 Points);

	// 발동 스킬 데미지 배율
	float GetActiveSkillEffectValue() const { return ActiveSkillEffectValue; }

	// 전체 스킬 ID 반환
	UFUNCTION(BlueprintCallable, Category = "Skill")
	TArray<FName> GetAllSkillIDs() const;

	// 습득 스킬 목록 반환
	UFUNCTION(BlueprintPure, Category = "Skill")
	TArray<FName> GetLearnedSkillIDs() const { return LearnedSkills.Array(); }

	// ─── 델리게이트 ────────────────────────────────────────────────

	FOnSkillPointsChanged OnSkillPointsChanged;
	FOnSkillLearned       OnSkillLearned;
	/** ActivateSkill 성공 시 Broadcast — (SkillID, CooldownDuration) */
	FOnSkillActivated     OnSkillActivated;
	/** 버프 스킬 발동 시 Broadcast — (SkillID, Icon, Duration) */
	FOnBuffApplied        OnBuffApplied;
	/** 버프 만료 시 Broadcast — OHSMBuffSkill 에서 호출 */
	FOnBuffExpired        OnBuffExpired;

	/** OHSMBuffSkill::OnBuffExpired 에서 만료 알림을 전달받아 브로드캐스트 */
	void BroadcastBuffExpired(FName SkillID);

private:
	/** 배운 스킬 집합 */
	UPROPERTY()
	TSet<FName> LearnedSkills;

	UPROPERTY()
	int32 AvailableSkillPoints = 0;

	// 스킬 인스턴스 캐시
	UPROPERTY()
	TMap<FName, TObjectPtr<UOHSMSkillBase>> SkillInstances;

	UOHSMSkillBase* GetOrCreateSkillInstance(FName SkillID);

	// 마지막 발동 스킬 배율
	float ActiveSkillEffectValue = 1.0f;
};
