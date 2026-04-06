#include "OHSMCraftNPC.h"

#include "OHSMNPCDialogueWidget.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

AOHSMCraftNPC::AOHSMCraftNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	// 상호작용 범위 (반경 200)
	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(RootComponent);
	InteractionRange->SetSphereRadius(200.f);
	InteractionRange->SetCollisionProfileName(TEXT("Trigger"));

	// NPC 머리 위 "E 상호작용" 안내 위젯
	InteractPromptComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractPrompt"));
	InteractPromptComponent->SetupAttachment(GetMesh());
	InteractPromptComponent->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	InteractPromptComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractPromptComponent->SetDrawSize(FVector2D(200.f, 50.f));
	InteractPromptComponent->SetVisibility(false);
}

void AOHSMCraftNPC::BeginPlay()
{
	Super::BeginPlay();

	InteractionRange->OnComponentBeginOverlap.AddDynamic(this, &AOHSMCraftNPC::OnRangeBeginOverlap);
	InteractionRange->OnComponentEndOverlap.AddDynamic(this, &AOHSMCraftNPC::OnRangeEndOverlap);

	// 프롬프트 위젯 클래스 설정
	if (InteractPromptWidgetClass)
	{
		InteractPromptComponent->SetWidgetClass(InteractPromptWidgetClass);
	}
}

void AOHSMCraftNPC::Interact_Implementation(APlayerController* InstigatorController)
{
	if (!IsValid(InstigatorController) || !DialogueWidgetClass) return;

	CurrentInstigator = InstigatorController;

	// 이미 대화창이 열려있으면 닫기
	if (IsValid(DialogueWidget) && DialogueWidget->IsInViewport())
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
		return;
	}

	// 대화창 생성 및 표시
	DialogueWidget = CreateWidget<UOHSMNPCDialogueWidget>(InstigatorController, DialogueWidgetClass);
	if (!IsValid(DialogueWidget)) return;

	DialogueWidget->SetDialogueMessage(DialogueMessage);
	DialogueWidget->OnConfirmed.AddUObject(this, &AOHSMCraftNPC::OnDialogueConfirmed);
	DialogueWidget->OnCancelled.AddUObject(this, &AOHSMCraftNPC::OnDialogueCancelled);
	DialogueWidget->AddToViewport(10);
}

void AOHSMCraftNPC::OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor)) return;

	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (!IsValid(PlayerChar)) return;

	APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
	if (!IsValid(PC)) return;

	// 상호작용 가능 NPC로 등록
	if (AOHSMPlayerController* OHSMController = Cast<AOHSMPlayerController>(PC))
	{
		OHSMController->SetInteractableActor(this);
	}

	// 안내 위젯 표시
	InteractPromptComponent->SetVisibility(true);
}

void AOHSMCraftNPC::OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor)) return;

	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (!IsValid(PlayerChar)) return;

	APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
	if (!IsValid(PC)) return;

	// 상호작용 NPC 해제
	if (AOHSMPlayerController* OHSMController = Cast<AOHSMPlayerController>(PC))
	{
		OHSMController->SetInteractableActor(nullptr);
	}

	// 안내 위젯 숨김
	InteractPromptComponent->SetVisibility(false);

	// 대화창도 닫기
	if (IsValid(DialogueWidget))
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}
}

void AOHSMCraftNPC::OnDialogueConfirmed()
{
	if (IsValid(DialogueWidget))
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}

	if (AOHSMPlayerController* OHSMController = Cast<AOHSMPlayerController>(CurrentInstigator))
	{
		OHSMController->OpenCraftPanel();
	}
}

void AOHSMCraftNPC::OnDialogueCancelled()
{
	if (IsValid(DialogueWidget))
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}
}
