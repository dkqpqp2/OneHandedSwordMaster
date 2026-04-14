#include "OHSMCraftNPC.h"

#include "OneHandedSwordMaster/Character/Player/OHSMPlayerController.h"

AOHSMCraftNPC::AOHSMCraftNPC()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/OneHandedSwordMaster/Character/Mixamo/CraftNPCMesh.CraftNPCMesh'"));
	if (MeshRef.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshRef.Object);
	}
	
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimRef(TEXT("/Game/OneHandedSwordMaster/Animations/ABP_NPC.ABP_NPC_C"));
	if (AnimRef.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimRef.Class);
	}

	// 기본 대화 메시지 설정
	DialogueMessage   = FText::FromString(TEXT("어떤 아이템을 만드시겠습니까?"));
	ConfirmButtonText = FText::FromString(TEXT("제작하기"));
	CancelButtonText  = FText::FromString(TEXT("아니요"));
}

void AOHSMCraftNPC::OnDialogueConfirmed(APlayerController* InstigatorController)
{
	AOHSMPlayerController* OHSMController = Cast<AOHSMPlayerController>(InstigatorController);
	if (IsValid(OHSMController))
	{
		OHSMController->OpenCraftPanel();
	}
}
