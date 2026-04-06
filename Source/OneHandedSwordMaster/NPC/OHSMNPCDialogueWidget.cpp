#include "OHSMNPCDialogueWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

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
}

void UOHSMNPCDialogueWidget::SetDialogueMessage(const FText& InMessage)
{
	if (Label_Message)
	{
		Label_Message->SetText(InMessage);
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
