// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMCombatComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OneHandedSwordMaster/Data/OHSMCombatData.h"


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
	
	// 콤보 데이터 확인
	if (BasicAttackRows.Num() == 0)
	{
		return;
	}
	
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
		if (CurrentComboIndex >= BasicAttackRows.Num())
		{
			// 마지막 콤보 후 처음으로
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
	CurrentComboIndex = 0;
	bCanNextCombo = false;
	bCanReceiveInput = false;
	bIsAttacking = false;
}

void UOHSMCombatComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

