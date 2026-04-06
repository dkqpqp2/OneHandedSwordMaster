#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OHSMNPCDialogueWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ONEHANDEDSWORDMASTER_API UOHSMNPCDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** NPC 대화 메시지 설정 */
	void SetDialogueMessage(const FText& InMessage);

	/** 확인 버튼 눌렸을 때 */
	FSimpleMulticastDelegate OnConfirmed;

	/** 취소 버튼 눌렸을 때 */
	FSimpleMulticastDelegate OnCancelled;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Label_Message;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Cancel;

private:
	UFUNCTION()
	void OnClickedConfirm();

	UFUNCTION()
	void OnClickedCancel();
};
