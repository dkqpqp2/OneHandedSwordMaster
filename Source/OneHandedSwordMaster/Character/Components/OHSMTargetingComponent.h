// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OHSMTargetingComponent.generated.h"

// 타겟 변경 시 브로드캐스트 (새 타겟, nullptr이면 해제)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONEHANDEDSWORDMASTER_API UOHSMTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOHSMTargetingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Targeting")
	FOnTargetChanged OnTargetChanged;

protected:
	// 타겟 탐색 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float MaxTargetingRange = 1500.0f;

	// 타겟이 이 거리 이상 멀어지면 자동 해제
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float AutoReleaseRange = 2000.0f;

	// 카메라 회전 보간 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float CameraInterpSpeed = 7.0f;

	// 캐릭터 회전 보간 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float CharacterInterpSpeed = 10.0f;

private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	bool bIsTargeting = false;

public:
	// 가장 가까운 적으로 락온 / 락온 해제 토글
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ToggleLockOn();

	// 마우스 X 이동량 누적 → 임계값 초과 시 해당 방향 적으로 전환
	void AddMouseDeltaX(float DeltaX);

	// 마우스 X 이동량 임계값 (이 값 이상 움직이면 타겟 전환)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	float SwitchThreshold = 200.0f;

	UFUNCTION(BlueprintPure, Category = "Targeting")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsTargeting() const { return bIsTargeting; }

private:
	// 범위 내 적 목록 수집
	TArray<AActor*> GetEnemiesInRange() const;

	// 카메라 기준으로 가장 중앙에 있는 적 반환
	AActor* FindBestTarget(const TArray<AActor*>& Candidates) const;

	// 타겟 설정 (nullptr이면 해제)
	void SetTarget(AActor* NewTarget);

	// 화면 기준 오른쪽/왼쪽 방향 적으로 전환
	void SwitchTargetByDirection(bool bRight);

	// 누적 마우스 X
	float AccumulatedMouseX = 0.0f;

	// 타겟 유효성 검사 (죽었거나 범위 벗어나면 해제)
	void ValidateTarget();

	// Tick에서 카메라 / 캐릭터 회전 업데이트
	void UpdateRotations(float DeltaTime);

	// PlayerController 가져오기
	APlayerController* GetPlayerController() const;
};
