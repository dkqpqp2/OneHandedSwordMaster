#include "OHSMNPCBase.h"

#include "OHSMNPCDialogueWidget.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

AOHSMNPCBase::AOHSMNPCBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 메시 공통 설정 (위치 / 회전 / 콜리전)
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -90.f),
		FRotator(0.f, -90.f, 0.f)
	);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(RootComponent);
	InteractionRange->SetSphereRadius(InteractionRadius);
	InteractionRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionRange->SetCollisionObjectType(ECC_GameTraceChannel6);        // NPC 채널
	InteractionRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionRange->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // Player만 Overlap

}

void AOHSMNPCBase::BeginPlay()
{
	Super::BeginPlay();

	InteractionRange->SetSphereRadius(InteractionRadius);
	InteractionRange->OnComponentBeginOverlap.AddDynamic(this, &AOHSMNPCBase::OnRangeBeginOverlap);
	InteractionRange->OnComponentEndOverlap.AddDynamic(this, &AOHSMNPCBase::OnRangeEndOverlap);
}

void AOHSMNPCBase::Interact_Implementation(APlayerController* InstigatorController)
{
	if (!IsValid(InstigatorController)) return;

	// 이미 열려있으면 닫기
	if (IsValid(ActiveDialogueWidget) && ActiveDialogueWidget->IsInViewport())
	{
		CloseDialogue();
		return;
	}

	OpenDialogue(InstigatorController);
}

void AOHSMNPCBase::OpenDialogue(APlayerController* InstigatorController)
{
	if (!DialogueWidgetClass) return;

	CachedInstigator = InstigatorController;

	ActiveDialogueWidget = CreateWidget<UOHSMNPCDialogueWidget>(InstigatorController, DialogueWidgetClass);
	if (!IsValid(ActiveDialogueWidget)) return;

	ActiveDialogueWidget->SetDialogueMessage(DialogueMessage);
	ActiveDialogueWidget->SetButtonTexts(ConfirmButtonText, CancelButtonText);
	ActiveDialogueWidget->OnConfirmed.AddUObject(this, &AOHSMNPCBase::HandleConfirmed);
	ActiveDialogueWidget->OnCancelled.AddUObject(this, &AOHSMNPCBase::HandleCancelled);
	ActiveDialogueWidget->AddToViewport(10);

	// 마우스 커서 활성화
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(InstigatorController))
	{
		PC->SetDialogueOpen(true);
	}
}

void AOHSMNPCBase::CloseDialogue()
{
	if (IsValid(ActiveDialogueWidget))
	{
		ActiveDialogueWidget->RemoveFromParent();
		ActiveDialogueWidget = nullptr;
	}

	// 마우스 커서 비활성화
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(CachedInstigator))
	{
		PC->SetDialogueOpen(false);
	}

	CachedInstigator = nullptr;
}

void AOHSMNPCBase::HandleConfirmed()
{
	// CloseDialogue() 전에 미리 저장 (CloseDialogue가 CachedInstigator를 null로 만들기 때문)
	APlayerController* PendingPC = CachedInstigator;
	CloseDialogue();
	OnDialogueConfirmed(PendingPC);
}

void AOHSMNPCBase::HandleCancelled()
{
	APlayerController* PendingPC = CachedInstigator;
	CloseDialogue();
	OnDialogueCancelled(PendingPC);
}

void AOHSMNPCBase::OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !IsValid(OtherComp)) return;

	// Player 채널 오브젝트만 처리
	if (OtherComp->GetCollisionObjectType() != ECC_GameTraceChannel1) return;

	AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(
		Cast<APawn>(OtherActor) ? Cast<APawn>(OtherActor)->GetController() : nullptr);
	if (!IsValid(PC)) return;

	PC->SetInteractableActor(this);
	PC->ShowInteractionWidget(InteractionHintText);
}

void AOHSMNPCBase::OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || !IsValid(OtherComp)) return;

	// Player 채널 오브젝트만 처리
	if (OtherComp->GetCollisionObjectType() != ECC_GameTraceChannel1) return;

	AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(
		Cast<APawn>(OtherActor) ? Cast<APawn>(OtherActor)->GetController() : nullptr);
	if (!IsValid(PC)) return;

	PC->SetInteractableActor(nullptr);
	PC->HideInteractionWidget();
	CloseDialogue();
}
