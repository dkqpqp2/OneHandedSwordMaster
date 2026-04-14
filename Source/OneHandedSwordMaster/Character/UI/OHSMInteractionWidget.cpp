#include "OHSMInteractionWidget.h"

#include "Components/TextBlock.h"

void UOHSMInteractionWidget::SetInteractionText(const FText& InText)
{
	if (Label_InteractionHint)
	{
		Label_InteractionHint->SetText(InText);
	}
}
