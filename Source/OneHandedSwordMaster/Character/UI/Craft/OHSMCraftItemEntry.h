
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Data/OHSMItemData.h"
#include "OHSMCraftItemEntry.generated.h"


struct FOHSMCraftItemData;
class UOHSMCraftInfo;

class UTextBlock;
class UImage;
class UWidgetSwitcher;

// ─── 트리 데이터 오브젝트 ─────────────────────────────────────────

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMTreeItemEntry : public UObject
{
	GENERATED_BODY()

public:
	int32 TreeDepth = 0;

	UPROPERTY()
	TObjectPtr<UOHSMTreeItemEntry> RootItem;

	bool HasChildren() const;
	TArray<UOHSMTreeItemEntry*> GetAllChildren() const;
	void AddChild(UOHSMTreeItemEntry* InChild);
	void RemoveChild(UOHSMTreeItemEntry* InChild);
	void ClearChildren();

private:
	TArray<UOHSMTreeItemEntry*> TreeChildren;
};

/** 제작 레시피 1행을 담는 데이터 오브젝트 */
UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMCraftTreeItemEntry : public UOHSMTreeItemEntry
{
	GENERATED_BODY()

public:
	const FOHSMCraftItemData* CraftItemData = nullptr;
	FName RecipeID;
	EItemType CategoryType = EItemType::Material;
};

// ─── 트리뷰 Entry 위젯 ───────────────────────────────────────────

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMCraftItemEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	virtual void NativeOnItemExpansionChanged(bool bIsExpanded) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	/** Depth 0 = Category 패널, Depth 1 = 레시피 패널 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SwitcherTreeItem;

	// ─── Category 패널 ─────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_CategoryName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SwitcherArrow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Img_Collapsed;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Img_Expanded;

	// ─── 레시피 패널 ───────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOHSMCraftInfo> CraftInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Selection;

private:
	void UpdateCategory(const FOHSMCraftItemData& InData);
	void UpdateItem(const FOHSMCraftItemData& InData, FName InRecipeID);
};
