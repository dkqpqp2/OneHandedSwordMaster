// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCombatComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Components/OHSMEquipmentComponent.h"


UOHSMCombatComponent::UOHSMCombatComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UOHSMCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	}
	
	InitializeComboRows();
}

void UOHSMCombatComponent::InitializeComboRows()
{
	if (!ComboDataTable)
	{
		return;
	}
	
	TArray<FOHSMComboAnimData*> AllRows;
	ComboDataTable->GetAllRows<FOHSMComboAnimData>(TEXT("ComboDataTable"), AllRows);
	
	BasicAttackRows.Empty();
	DodgeRows.Empty();
	
	for (FOHSMComboAnimData* Row : AllRows)
	{
		if (!Row)
		{
			continue;
		}
		
		switch (Row->ActionType)
		{
			case EOHSMActionType::BasicAttack:
				BasicAttackRows.Add(Row);
				break;
			case EOHSMActionType::Dodge:
				DodgeRows.Add(Row);
				break;
			default:
				break;
		}
	}
	
	BasicAttackRows.Sort([](const FOHSMComboAnimData& A, const FOHSMComboAnimData& B)
	{
		return A.ComboIndex < B.ComboIndex;
	});
	
	DodgeRows.Sort([](const FOHSMComboAnimData& A, const FOHSMComboAnimData& B)
	{
		return A.ComboIndex < B.ComboIndex;
	});
}

void UOHSMCombatComponent::PerformBasicAttack()
{
	if (OwnerCharacter && OwnerCharacter->GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// 무기 장착 여부 확인
	if (AOHSMPlayerCharacter* PlayerChar = Cast<AOHSMPlayerCharacter>(OwnerCharacter))
	{
		UOHSMEquipmentComponent* EquipComp = PlayerChar->GetEquipmentComponent();
		if (!EquipComp || EquipComp->IsSlotEmpty(EEquipmentSlot::Weapon))
		{
			return; // 무기가 없으면 공격 불가
		}
	}

	// 콤보 데이터 확인
	if (BasicAttackRows.Num() == 0)
	{
		return;
	}

	// 현재 허용된 최대 콤보 수 (MaxComboCount와 데이터 행 수 중 작은 값)
	const int32 AllowedCombo = FMath::Min(MaxComboCount, BasicAttackRows.Num());

	// 처음 입력 또는 콤보가 끊긴 상태
	if (!bCanNextCombo)
	{
		CurrentComboIndex = 0;
		bCanNextCombo = true;
		bCanReceiveInput = false;
	}
	else
	{
		// 입력 가능 시간이 아니면 무시
		if (!bCanReceiveInput)
		{
			return;
		}

		CurrentComboIndex++;
		if (CurrentComboIndex >= AllowedCombo)
		{
			// 허용된 콤보 횟수 초과 → 즉시 리셋 (다음 프레임부터 다시 공격 가능)
			ResetCombo();
			return;
		}
		bCanReceiveInput = false;
	}
	
	FOHSMComboAnimData* ComboData = BasicAttackRows[CurrentComboIndex];
	if (!ComboData || !ComboData->AnimationMontage)
	{
		return;
	}
	
	bIsAttacking = true;
	CurrentMontage = ComboData->AnimationMontage;

	if (OwnerCharacter)
	{
		UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
		if (Movement)
		{
			if (Movement->MaxWalkSpeed > 0.0f)
			{
				CachedMaxWalkSpeed = Movement->MaxWalkSpeed;
				CachedMaxAcceleration = Movement->MaxAcceleration;
			}

			// 공격 중 이동 잠금
			Movement->MaxWalkSpeed = 0.0f;
			Movement->MaxAcceleration = 0.0f;
		}
	}
	
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CurrentMontage, 1.0f);
		
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UOHSMCombatComponent::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);

	}
	else
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		ComboResetTimerHandle,
		this,
		&UOHSMCombatComponent::ResetCombo,
		ComboData->ComboInputTime,
		false
	);
}

void UOHSMCombatComponent::PerformDodge()
{
}

void UOHSMCombatComponent::EnableNextAttackInput()
{
	bCanReceiveInput = true;
}

void UOHSMCombatComponent::EnableInputReceive()
{
	bCanReceiveInput = true;
}

void UOHSMCombatComponent::DisableInputReceive()
{
	bCanReceiveInput = false;
}

void UOHSMCombatComponent::ResetCombo()
{
	// 타이머가 남아있으면 취소 (OnMontageEnded에서 호출될 때 이중 실행 방지)
	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);

	CurrentComboIndex = 0;
	bCanNextCombo = false;
	bCanReceiveInput = false;
	bIsAttacking = false;

	if (OwnerCharacter)
	{
		UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
		if (Movement)
		{
			Movement->MaxWalkSpeed = CachedMaxWalkSpeed;
			Movement->MaxAcceleration = CachedMaxAcceleration;
		}
	}
}

void UOHSMCombatComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;

	if (bInterrupted)
	{
		// 콤보 체인으로 다음 몽타주가 시작됐을 때 → 이동만 잠깐 복구
		// (다음 몽타주 시작 시 다시 잠김)
		if (OwnerCharacter)
		{
			UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
			if (Movement)
			{
				Movement->MaxWalkSpeed    = CachedMaxWalkSpeed;
				Movement->MaxAcceleration = CachedMaxAcceleration;
			}
		}
	}
	else
	{
		// 몽타주가 자연스럽게 끝남 → 콤보 완전 리셋
		// (이동 복구 + bCanNextCombo = false → 다음 공격 즉시 가능)
		ResetCombo();
	}
}

void UOHSMCombatComponent::SetMaxComboCount(int32 Count)
{
	// 현재값보다 클 때만 갱신 (콤보는 스킬로 단계적으로 늘어남)
	if (Count > MaxComboCount)
	{
		MaxComboCount = Count;
	}
}

