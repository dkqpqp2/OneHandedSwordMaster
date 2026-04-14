// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMSkillPanel.h"

#include "OHSMSkillNodeWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/Events.h"
#include "OneHandedSwordMaster/Character/Components/OHSMSkillComponent.h"
#include "OneHandedSwordMaster/Character/Components/OHSMPlayerStatComponent.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerCharacter.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"
#include "OneHandedSwordMaster/Data/OHSMSkillData.h"

// ─── 초기화 ──────────────────────────────────────────────────────────

void UOHSMSkillPanel::SetSkillComponent(UOHSMSkillComponent* InSkillComp)
{
	// 이전 연결 해제
	if (SkillComponent)
	{
		SkillComponent->OnSkillLearned.RemoveAll(this);
		SkillComponent->OnSkillPointsChanged.RemoveAll(this);
	}

	SkillComponent = InSkillComp;

	if (!SkillComponent)
	{
		return;
	}

	SkillComponent->OnSkillLearned.AddUObject(
		this, &UOHSMSkillPanel::OnSkillComponentLearned);
	SkillComponent->OnSkillPointsChanged.AddUObject(
		this, &UOHSMSkillPanel::OnSkillPointsChanged);

	BuildSkillTree();
	RefreshSkillPoints();
}

void UOHSMSkillPanel::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (Btn_Learn)
	{
		Btn_Learn->OnClicked.AddDynamic(this, &UOHSMSkillPanel::OnBtnLearnClicked);
	}

	// 상세 패널 초기 상태 — 아무것도 선택 안 된 상태
	if (Label_SkillName)
	{
		Label_SkillName->SetText(FText::GetEmpty());
	}
	if (Label_SkillDesc)
	{
		Label_SkillDesc->SetText(FText::GetEmpty());
	}
	if (Label_SkillType)
	{
		Label_SkillType->SetText(FText::GetEmpty());
	}
	if (Label_EffectInfo)
	{
		Label_EffectInfo->SetText(FText::GetEmpty());
	}
	if (Label_ManaCost)
	{
		Label_ManaCost->SetText(FText::GetEmpty());
	}
	if (Label_Cooldown)
	{
		Label_Cooldown->SetText(FText::GetEmpty());
	}
	if (Label_LearnCost)
	{
		Label_LearnCost->SetText(FText::GetEmpty());
	}
	if (Label_Prerequisite)
	{
		Label_Prerequisite->SetText(FText::GetEmpty());
	}
	if (Btn_Learn)
	{
		Btn_Learn->SetIsEnabled(false);
	}
}

void UOHSMSkillPanel::NativeDestruct()
{
	if (SkillComponent)
	{
		SkillComponent->OnSkillLearned.RemoveAll(this);
		SkillComponent->OnSkillPointsChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

FReply UOHSMSkillPanel::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(GetOwningPlayer()))
		{
			PC->CloseSkillPanel();
		}
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ─── 트리 구성 ───────────────────────────────────────────────────────

void UOHSMSkillPanel::BuildSkillTree()
{
	if (!Canvas_SkillTree || !SkillComponent || !SkillNodeWidgetClass)
	{
		return;
	}

	Canvas_SkillTree->ClearChildren();
	NodeWidgets.Empty();

	TArray<FName> AllSkillIDs = SkillComponent->GetAllSkillIDs();

	// ① 연결선 먼저 그리기 (노드 뒤에 위치하도록)
	for (const FName& SkillID : AllSkillIDs)
	{
		const FOHSMSkillData* Data = SkillComponent->GetSkillData(SkillID);
		if (!Data)
		{
			continue;
		}

		const FVector2D ChildCenter = GetNodeCenterPosition(Data);

		for (const FName& PrereqID : Data->PrerequisiteSkillIDs)
		{
			const FOHSMSkillData* PrereqData = SkillComponent->GetSkillData(PrereqID);
			if (!PrereqData)
			{
				continue;
			}

			const FVector2D ParentCenter = GetNodeCenterPosition(PrereqData);
			DrawConnectionLine(ParentCenter, ChildCenter);
		}
	}

	// ② 노드 위젯 생성 및 배치
	for (const FName& SkillID : AllSkillIDs)
	{
		const FOHSMSkillData* Data = SkillComponent->GetSkillData(SkillID);
		if (!Data)
		{
			continue;
		}

		UOHSMSkillNodeWidget* NodeWidget =
			CreateWidget<UOHSMSkillNodeWidget>(this, SkillNodeWidgetClass);
		if (!NodeWidget)
		{
			continue;
		}

		NodeWidget->InitNode(SkillID, Data);
		NodeWidget->OnNodeClicked.AddUObject(this, &UOHSMSkillPanel::OnNodeClicked);

		// Canvas에 추가
		UCanvasPanelSlot* CanvasSlot =
			Cast<UCanvasPanelSlot>(Canvas_SkillTree->AddChild(NodeWidget));
		if (CanvasSlot)
		{
			const FVector2D NodePos(
				Data->TreeColumn * NodeSpacingX,
				Data->TreeTier   * NodeSpacingY
			);
			CanvasSlot->SetPosition(NodePos);
			CanvasSlot->SetSize(FVector2D(NodeSize, NodeSize));
			CanvasSlot->SetAutoSize(false);
		}

		NodeWidgets.Add(SkillID, NodeWidget);
	}

	RefreshAllNodeStates();

	// 패널을 열 때 루트 스킬(Tier 0 중 Column이 가장 작은 것)을 자동 선택
	FName FirstSkillID = NAME_None;
	int32 MinTier   = INT_MAX;
	int32 MinColumn = INT_MAX;

	for (const FName& SkillID : AllSkillIDs)
	{
		const FOHSMSkillData* Data = SkillComponent->GetSkillData(SkillID);
		if (!Data) continue;

		if (Data->TreeTier < MinTier ||
			(Data->TreeTier == MinTier && Data->TreeColumn < MinColumn))
		{
			MinTier      = Data->TreeTier;
			MinColumn    = Data->TreeColumn;
			FirstSkillID = SkillID;
		}
	}

	if (!FirstSkillID.IsNone())
	{
		OnNodeClicked(FirstSkillID);
	}
}

void UOHSMSkillPanel::DrawConnectionLine(FVector2D From, FVector2D To)
{
	if (!Canvas_SkillTree)
	{
		return;
	}

	// From → To 벡터
	const FVector2D Delta    = To - From;
	const float     Length   = Delta.Size();
	const float     AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
	const FVector2D MidPoint = (From + To) * 0.5f;

	UImage* Line = NewObject<UImage>(this);
	Line->SetColorAndOpacity(LineColor);

	UCanvasPanelSlot* LineSlot = Cast<UCanvasPanelSlot>(Canvas_SkillTree->AddChild(Line));
	if (LineSlot)
	{
		LineSlot->SetPosition(MidPoint - FVector2D(Length * 0.5f, LineThickness * 0.5f));
		LineSlot->SetSize(FVector2D(Length, LineThickness));
		LineSlot->SetAutoSize(false);
	}

	FWidgetTransform Transform;
	Transform.Angle       = AngleDeg;
	Transform.Translation = FVector2D::ZeroVector;
	Transform.Scale       = FVector2D::UnitVector;
	Transform.Shear       = FVector2D::ZeroVector;
	Line->SetRenderTransform(Transform);
	Line->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
}

FVector2D UOHSMSkillPanel::GetNodeCenterPosition(const FOHSMSkillData* Data) const
{
	if (!Data)
	{
		return FVector2D::ZeroVector;
	}

	return FVector2D(
		Data->TreeColumn * NodeSpacingX + NodeSize * 0.5f,
		Data->TreeTier   * NodeSpacingY + NodeSize * 0.5f
	);
}

// ─── 상태 갱신 ───────────────────────────────────────────────────────

void UOHSMSkillPanel::RefreshAllNodeStates()
{
	if (!SkillComponent)
	{
		return;
	}

	for (auto& Pair : NodeWidgets)
	{
		const FName& SkillID       = Pair.Key;
		UOHSMSkillNodeWidget* Node = Pair.Value;
		if (!Node)
		{
			continue;
		}

		ESkillNodeState State;
		if (SkillComponent->IsSkillLearned(SkillID))
		{
			State = ESkillNodeState::Learned;
		}
		else if (SkillComponent->CanLearnSkill(SkillID))
		{
			State = ESkillNodeState::Available;
		}
		else
		{
			State = ESkillNodeState::Locked;
		}
		Node->RefreshState(State);
	}
}

void UOHSMSkillPanel::RefreshDetailPanel(FName SkillID)
{
	if (!SkillComponent)
	{
		return;
	}

	const FOHSMSkillData* Data = SkillComponent->GetSkillData(SkillID);
	if (!Data)
	{
		return;
	}

	// 아이콘
	if (Img_SelectedIcon && Data->SkillIcon)
	{
		Img_SelectedIcon->SetBrushFromTexture(Data->SkillIcon);
	}

	// 이름
	if (Label_SkillName)
	{
		Label_SkillName->SetText(Data->SkillName);
	}

	// 종류
	if (Label_SkillType)
	{
		Label_SkillType->SetText(UEnum::GetDisplayValueAsText(Data->SkillType));
	}

	// 설명
	if (Label_SkillDesc)
	{
		Label_SkillDesc->SetText(Data->Description);
	}

	// 효과 수치 — 스킬 종류에 따라 다르게 표시
	if (Label_EffectInfo)
	{
		// 플레이어 현재 공격력 가져오기
		float AttackPower = 0.f;
		if (IsValid(SkillComponent))
		{
			if (AOHSMPlayerCharacter* Player = Cast<AOHSMPlayerCharacter>(SkillComponent->GetOwner()))
			{
				if (UOHSMPlayerStatComponent* StatComp = Player->GetStatComponent())
				{
					AttackPower = StatComp->GetAttackPower();
				}
			}
		}

		FText EffectText;
		switch (Data->SkillType)
		{
		case ESkillType::Passive:
		case ESkillType::Buff:
			EffectText = FText::Format(
				FText::FromString(TEXT("{0} +{1}")),
				UEnum::GetDisplayValueAsText(Data->BuffStatType),
				FText::AsNumber(FMath::RoundToInt(Data->EffectValue))
			);
			break;
		case ESkillType::Attack:
			// 실제 데미지 = 공격력 × 배율
			EffectText = FText::Format(
				FText::FromString(TEXT("데미지: {0}  (공격력 × {1}배)")),
				FText::AsNumber(FMath::RoundToInt(AttackPower * Data->EffectValue)),
				FText::AsNumber(Data->EffectValue)
			);
			break;
		case ESkillType::Slash:
			// 실제 투사체 데미지 = 공격력 × 배율
			EffectText = FText::Format(
				FText::FromString(TEXT("투사체 데미지: {0}  (공격력 × {1}배)")),
				FText::AsNumber(FMath::RoundToInt(AttackPower * Data->EffectValue)),
				FText::AsNumber(Data->EffectValue)
			);
			break;
		}
		Label_EffectInfo->SetText(EffectText);
	}

	// 마나 소모
	if (Label_ManaCost)
	{
		if (Data->SkillType == ESkillType::Passive)
		{
			Label_ManaCost->SetText(FText::FromString(TEXT("마나: -")));
		}
		else
		{
			Label_ManaCost->SetText(FText::Format(
				FText::FromString(TEXT("마나: {0}")),
				FText::AsNumber(FMath::RoundToInt(Data->ManaCost))
			));
		}
	}

	// 쿨다운
	if (Label_Cooldown)
	{
		if (Data->SkillType == ESkillType::Passive)
		{
			Label_Cooldown->SetText(FText::FromString(TEXT("쿨다운: -")));
		}
		else
		{
			Label_Cooldown->SetText(FText::Format(
				FText::FromString(TEXT("쿨다운: {0}초")),
				FText::AsNumber(FMath::RoundToInt(Data->Cooldown))
			));
		}
	}

	// 습득 비용
	if (Label_LearnCost)
	{
		Label_LearnCost->SetText(FText::Format(
			FText::FromString(TEXT("필요 포인트: {0}")),
			FText::AsNumber(Data->SkillPointCost)
		));
	}

	// 전제 스킬 안내
	if (Label_Prerequisite)
	{
		if (!SkillComponent->CanLearnSkill(SkillID) && !SkillComponent->IsSkillLearned(SkillID))
		{
			TArray<FText> MissingNames;
			for (const FName& PrereqID : Data->PrerequisiteSkillIDs)
			{
				if (!SkillComponent->IsSkillLearned(PrereqID))
				{
					if (const FOHSMSkillData* PrereqData = SkillComponent->GetSkillData(PrereqID))
					{
						MissingNames.Add(PrereqData->SkillName);
					}
				}
			}
			if (!MissingNames.IsEmpty())
			{
				FString Missing;
				for (const FText& Name : MissingNames)
				{
					if (!Missing.IsEmpty())
					{
						Missing += TEXT(", ");
					}
					Missing += Name.ToString();
				}
				Label_Prerequisite->SetText(FText::Format(
					FText::FromString(TEXT("선행 스킬 필요: {0}")),
					FText::FromString(Missing)
				));
			}
			else
			{
				Label_Prerequisite->SetText(FText::FromString(TEXT("스킬 포인트가 부족합니다")));
			}
		}
		else
		{
			Label_Prerequisite->SetText(FText::GetEmpty());
		}
	}

	RefreshLearnButton();
}

void UOHSMSkillPanel::RefreshLearnButton()
{
	if (!Btn_Learn || !SkillComponent || SelectedSkillID.IsNone())
	{
		return;
	}

	const bool bLearned  = SkillComponent->IsSkillLearned(SelectedSkillID);
	const bool bCanLearn = SkillComponent->CanLearnSkill(SelectedSkillID);

	Btn_Learn->SetIsEnabled(bCanLearn);

	if (UTextBlock* BtnLabel = Cast<UTextBlock>(Btn_Learn->GetChildAt(0)))
	{
		if (bLearned)
		{
			BtnLabel->SetText(FText::FromString(TEXT("습득 완료")));
		}
		else if (bCanLearn)
		{
			BtnLabel->SetText(FText::FromString(TEXT("습득")));
		}
		else
		{
			BtnLabel->SetText(FText::FromString(TEXT("습득 불가")));
		}
	}
}

void UOHSMSkillPanel::RefreshSkillPoints()
{
	if (!Label_SkillPoints || !SkillComponent)
	{
		return;
	}

	Label_SkillPoints->SetText(FText::Format(
		FText::FromString(TEXT("스킬 포인트: {0}")),
		FText::AsNumber(SkillComponent->GetAvailableSkillPoints())
	));
}

// ─── 이벤트 ──────────────────────────────────────────────────────────

void UOHSMSkillPanel::OnNodeClicked(FName SkillID)
{
	SelectedSkillID = SkillID;
	RefreshDetailPanel(SkillID);
}

void UOHSMSkillPanel::OnSkillComponentLearned(FName SkillID)
{
	RefreshAllNodeStates();
	if (SelectedSkillID == SkillID)
	{
		RefreshDetailPanel(SkillID);
	}
	RefreshSkillPoints();
}

void UOHSMSkillPanel::OnSkillPointsChanged()
{
	RefreshSkillPoints();
	RefreshAllNodeStates();
	RefreshLearnButton();
}

void UOHSMSkillPanel::OnBtnLearnClicked()
{
	if (!SkillComponent || SelectedSkillID.IsNone())
	{
		return;
	}
	SkillComponent->LearnSkill(SelectedSkillID);
}
