// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMSkillPanel.generated.h"

class UOHSMSkillComponent;
class UOHSMSkillNodeWidget;
struct FOHSMSkillData;

/**
 * 스킬 트리 패널.
 *
 * 왼쪽: Canvas_SkillTree — 노드(WBP_OHSMSkillNode)를 TreeTier/TreeColumn 기반으로 배치
 *       노드 사이 연결선도 Canvas에 Image로 그린다.
 * 오른쪽: 선택된 스킬 상세 정보 + 습득 버튼
 *
 * [Blueprint BindWidget]
 * 트리 영역
 *   Canvas_SkillTree   - 노드 배치용 캔버스
 * 상세 패널
 *   Img_SelectedIcon   - 선택한 스킬 아이콘
 *   Label_SkillName    - 스킬 이름
 *   Label_SkillType    - 스킬 종류 (패시브/버프/타격/참격)
 *   Label_SkillDesc    - 스킬 설명
 *   Label_EffectInfo   - 효과 수치 요약
 *   Label_ManaCost     - 마나 소모량
 *   Label_Cooldown     - 쿨다운
 *   Label_LearnCost    - 습득 포인트 비용
 *   Label_SkillPoints  - 현재 보유 스킬 포인트
 *   Btn_Learn          - 습득 버튼
 *   Label_Prerequisite - 전제 스킬 안내 (배울 수 없을 때 표시)
 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMSkillPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 스킬 컴포넌트 연결 — PlayerController에서 패널 열 때 호출 */
	void SetSkillComponent(UOHSMSkillComponent* InSkillComp);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ─── BindWidget — 트리 ────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> Canvas_SkillTree;

	// ─── BindWidget — 상세 패널 ───────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_SelectedIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_SkillName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_SkillType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_SkillDesc;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_EffectInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_ManaCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_Cooldown;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_LearnCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_SkillPoints;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Learn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label_Prerequisite;

	// ─── 설정 ────────────────────────────────────────────────────
	/** 스킬 노드 위젯 클래스 (WBP_OHSMSkillNode) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|UI")
	TSubclassOf<UOHSMSkillNodeWidget> SkillNodeWidgetClass;

	/** 노드 크기 (px) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|UI")
	float NodeSize = 90.f;

	/** 노드 간 가로 간격 (Column 사이) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|UI")
	float NodeSpacingX = 130.f;

	/** 노드 간 세로 간격 (Tier 사이) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|UI")
	float NodeSpacingY = 150.f;

	/** 연결선 두께 (px) */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|UI")
	float LineThickness = 3.f;

	/** 연결선 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|UI")
	FLinearColor LineColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);

private:
	UPROPERTY()
	TObjectPtr<UOHSMSkillComponent> SkillComponent;

	/** SkillID → 노드 위젯 */
	UPROPERTY()
	TMap<FName, TObjectPtr<UOHSMSkillNodeWidget>> NodeWidgets;

	FName SelectedSkillID;

	// ─── 트리 구성 ────────────────────────────────────────────────
	void BuildSkillTree();
	void DrawConnectionLine(FVector2D From, FVector2D To);
	FVector2D GetNodeCenterPosition(const FOHSMSkillData* Data) const;

	// ─── 상태 갱신 ────────────────────────────────────────────────
	void RefreshAllNodeStates();
	void RefreshDetailPanel(FName SkillID);
	void RefreshLearnButton();
	void RefreshSkillPoints();

	// ─── 이벤트 ──────────────────────────────────────────────────
	void OnNodeClicked(FName SkillID);
	void OnSkillComponentLearned(FName SkillID);
	void OnSkillPointsChanged();

	UFUNCTION()
	void OnBtnLearnClicked();
};
