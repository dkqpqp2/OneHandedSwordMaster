#include "OHSMNPCBase.h"

#include "OHSMNPCDialogueWidget.h"
#include "OHSMNPCInfoWidget.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

AOHSMNPCBase::AOHSMNPCBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// NPC는 AI 불필요 — AIController 자동 생성 비활성화
	AutoPossessAI = EAutoPossessAI::Disabled;

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

	// 이름 위젯 표시 범위 구체 (InteractionRange보다 크게)
	VisibilityRange = CreateDefaultSubobject<USphereComponent>(TEXT("VisibilityRange"));
	VisibilityRange->SetupAttachment(RootComponent);
	VisibilityRange->SetSphereRadius(NameVisibilityRadius);
	VisibilityRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisibilityRange->SetCollisionObjectType(ECC_GameTraceChannel6);
	VisibilityRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	VisibilityRange->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

	// 머리 위 정보 위젯 컴포넌트
	InfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidget"));
	InfoWidgetComponent->SetupAttachment(RootComponent);
	InfoWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	InfoWidgetComponent->SetWidgetSpace(EWidgetSpace::World);  // World = 벽에 가려짐
	InfoWidgetComponent->SetDrawSize(FVector2D(200.f, 80.f));
	InfoWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InfoWidgetComponent->SetVisibility(false);  // 기본 숨김
}

void AOHSMNPCBase::BeginPlay()
{
	Super::BeginPlay();

	InteractionRange->SetSphereRadius(InteractionRadius);
	InteractionRange->OnComponentBeginOverlap.AddDynamic(this, &AOHSMNPCBase::OnRangeBeginOverlap);
	InteractionRange->OnComponentEndOverlap.AddDynamic(this, &AOHSMNPCBase::OnRangeEndOverlap);

	VisibilityRange->SetSphereRadius(NameVisibilityRadius);
	VisibilityRange->OnComponentBeginOverlap.AddDynamic(this, &AOHSMNPCBase::OnVisibilityBeginOverlap);
	VisibilityRange->OnComponentEndOverlap.AddDynamic(this, &AOHSMNPCBase::OnVisibilityEndOverlap);

	// NPC 이름 위젯에 설정
	if (UOHSMNPCInfoWidget* InfoWidget = GetInfoWidget())
	{
		InfoWidget->SetNPCName(NPCName);
	}
}

void AOHSMNPCBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 위젯이 보일 때만 카메라 방향으로 회전 (World 공간에서 항상 플레이어를 향하게)
	if (!InfoWidgetComponent->IsVisible()) return;

	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager) return;

	const FVector CamLoc  = PC->PlayerCameraManager->GetCameraLocation();
	const FVector ToCamera = (CamLoc - InfoWidgetComponent->GetComponentLocation()).GetSafeNormal();
	if (!ToCamera.IsNearlyZero())
	{
		InfoWidgetComponent->SetWorldRotation(ToCamera.Rotation());
	}
}

UOHSMNPCInfoWidget* AOHSMNPCBase::GetInfoWidget() const
{
	if (!InfoWidgetComponent) return nullptr;
	return Cast<UOHSMNPCInfoWidget>(InfoWidgetComponent->GetUserWidgetObject());
}

// 플레이어 상호작용 처리
void AOHSMNPCBase::Interact_Implementation(APlayerController* InstigatorController)
{
	if (!IsValid(InstigatorController)) return;

	// 이미 열려있으면 닫기
	if (IsValid(ActiveDialogueWidget) && ActiveDialogueWidget->IsInViewport())
	{
		CloseDialogue(); // 대화창 닫기
		return;
	}

	OpenDialogue(InstigatorController); // 대화창 열기
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

	// 마우스 커서 활성화 + 상호작용 힌트 숨김
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(InstigatorController))
	{
		PC->SetDialogueOpen(true);
		PC->HideInteractionWidget(); // 대화 중 "[G] 대화하기" 숨김
	}
}

void AOHSMNPCBase::CloseDialogue()
{
	if (IsValid(ActiveDialogueWidget))
	{
		ActiveDialogueWidget->RemoveFromParent();
		ActiveDialogueWidget = nullptr;
	}

	// 마우스 커서 비활성화 + 상호작용 힌트 다시 표시
	if (AOHSMPlayerController* PC = Cast<AOHSMPlayerController>(CachedInstigator))
	{
		PC->SetDialogueOpen(false);
		PC->ShowInteractionWidget(InteractionHintText); // 대화 끝나면 힌트 복구
	}

	CachedInstigator = nullptr;
}

// 확인 버튼 콜백
void AOHSMNPCBase::HandleConfirmed()
{
	// CloseDialogue() 전에 미리 저장 (CloseDialogue가 CachedInstigator를 null로 만들기 때문)
	APlayerController* PendingPC = CachedInstigator;
	CloseDialogue(); // 대화창 닫기
	OnDialogueConfirmed(PendingPC); // 자식 클래스 처리
}

void AOHSMNPCBase::HandleCancelled()
{
	APlayerController* PendingPC = CachedInstigator;
	CloseDialogue();
	OnDialogueCancelled(PendingPC);
}

void AOHSMNPCBase::OnVisibilityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherComp)) return;
	if (OtherComp->GetCollisionObjectType() != ECC_GameTraceChannel1) return;

	InfoWidgetComponent->SetVisibility(true);
}

void AOHSMNPCBase::OnVisibilityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherComp)) return;
	if (OtherComp->GetCollisionObjectType() != ECC_GameTraceChannel1) return;

	InfoWidgetComponent->SetVisibility(false);
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
