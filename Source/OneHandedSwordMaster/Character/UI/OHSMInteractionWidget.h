#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMInteractionWidget.generated.h"

class UTextBlock;

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetInteractionText(const FText& InText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_InteractionHint;
};
