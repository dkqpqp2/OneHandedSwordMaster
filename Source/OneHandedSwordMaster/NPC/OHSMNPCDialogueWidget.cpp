#include "OHSMNPCDialogueWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"

// 위젯 생성 시 버튼 바인딩
void UOHSMNPCDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked.AddDynamic(this, &UOHSMNPCDialogueWidget::OnClickedConfirm); // 확인 클릭
	}

	if (Btn_Cancel)
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &UOHSMNPCDialogueWidget::OnClickedCancel); // 취소 클릭
	}

	// ESC 키를 받으려면 포커스가 있어야 함
	SetIsFocusable(true);
	SetKeyboardFocus(); // 키보드 포커스 획득
}

FReply UOHSMNPCDialogueWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnClickedCancel();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UOHSMNPCDialogueWidget::SetDialogueMessage(const FText& InMessage)
{
	if (Label_Message)
	{
		Label_Message->SetText(InMessage);
	}
}

// 버튼 텍스트 설정
void UOHSMNPCDialogueWidget::SetButtonTexts(const FText& InConfirm, const FText& InCancel)
{
	if (Btn_Confirm)
	{
		if (UTextBlock* ConfirmLabel = Cast<UTextBlock>(Btn_Confirm->GetChildAt(0)))
		{
			ConfirmLabel->SetText(InConfirm); // 확인 텍스트 설정
		}
	}

	if (Btn_Cancel)
	{
		// 취소 텍스트가 비어있으면 버튼 자체를 숨김
		if (InCancel.IsEmpty())
		{
			Btn_Cancel->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Btn_Cancel->SetVisibility(ESlateVisibility::Visible);
			if (UTextBlock* CancelLabel = Cast<UTextBlock>(Btn_Cancel->GetChildAt(0)))
			{
				CancelLabel->SetText(InCancel);
			}
		}
	}
}

void UOHSMNPCDialogueWidget::OnClickedConfirm()
{
	OnConfirmed.Broadcast();
}

void UOHSMNPCDialogueWidget::OnClickedCancel()
{
	OnCancelled.Broadcast();
}
