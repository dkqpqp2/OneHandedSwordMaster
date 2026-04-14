#include "OHSMNPCDialogueWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"

void UOHSMNPCDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked.AddDynamic(this, &UOHSMNPCDialogueWidget::OnClickedConfirm);
	}

	if (Btn_Cancel)
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &UOHSMNPCDialogueWidget::OnClickedCancel);
	}

	// ESC 키를 받으려면 포커스가 있어야 함
	SetIsFocusable(true);
	SetKeyboardFocus();
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

void UOHSMNPCDialogueWidget::SetButtonTexts(const FText& InConfirm, const FText& InCancel)
{
	if (Btn_Confirm)
	{
		if (UTextBlock* ConfirmLabel = Cast<UTextBlock>(Btn_Confirm->GetChildAt(0)))
		{
			ConfirmLabel->SetText(InConfirm);
		}
	}

	if (Btn_Cancel)
	{
		if (UTextBlock* CancelLabel = Cast<UTextBlock>(Btn_Cancel->GetChildAt(0)))
		{
			CancelLabel->SetText(InCancel);
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
