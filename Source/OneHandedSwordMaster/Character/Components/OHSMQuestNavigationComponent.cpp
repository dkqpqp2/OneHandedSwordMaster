#include "OHSMQuestNavigationComponent.h"

#include "OHSMNavDotActor.h"
#include "OHSMQuestComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "EngineUtils.h"
#include "OneHandedSwordMaster/Data/OHSMQuestData.h"
#include "OneHandedSwordMaster/NPC/OHSMQuestNPC.h"
#include "OneHandedSwordMaster/World/OHSMQuestObjectiveMarker.h"

UOHSMQuestNavigationComponent::UOHSMQuestNavigationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ─── 생명주기 ─────────────────────────────────────────────────────────────────

void UOHSMQuestNavigationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UOHSMQuestComponent* QuestComp = GetOwner()->FindComponentByClass<UOHSMQuestComponent>())
	{
		InitializeNavigation(QuestComp);
	}
}

void UOHSMQuestNavigationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);

	if (IsValid(NavDotActor))
	{
		NavDotActor->Destroy();
		NavDotActor = nullptr;
	}
}

// ─── 초기화 ──────────────────────────────────────────────────────────────────

void UOHSMQuestNavigationComponent::InitializeNavigation(UOHSMQuestComponent* InQuestComp)
{
	QuestComponent = InQuestComp;
	if (!QuestComponent)
	{
		return;
	}

	QuestComponent->OnQuestTrackingChanged.AddUObject(this, &UOHSMQuestNavigationComponent::OnTrackingChanged);
	QuestComponent->OnQuestProgressUpdated.AddUObject(this, &UOHSMQuestNavigationComponent::OnQuestProgressUpdated);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	NavDotActor = GetWorld()->SpawnActor<AOHSMNavDotActor>(FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (NavDotActor && DotMesh)
	{
		NavDotActor->DotISM->SetStaticMesh(DotMesh);
		if (DotMaterial)
		{
			NavDotActor->DotISM->SetMaterial(0, DotMaterial);
		}
	}
}

// ─── 명시적 타겟 설정 ─────────────────────────────────────────────────────────

void UOHSMQuestNavigationComponent::SetManualNavTarget(FName QuestID)
{
	ManualNavQuestID = QuestID;
	TickUpdate();
}

// ─── 델리게이트 콜백 ─────────────────────────────────────────────────────────

void UOHSMQuestNavigationComponent::OnTrackingChanged()
{
	ManualNavQuestID = NAME_None;
	StartUpdateTimer();
	TickUpdate();
}

void UOHSMQuestNavigationComponent::OnQuestProgressUpdated(FName /*QuestID*/)
{
	TickUpdate();
}

// ─── 업데이트 ─────────────────────────────────────────────────────────────────

void UOHSMQuestNavigationComponent::StartUpdateTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle, this,
		&UOHSMQuestNavigationComponent::TickUpdate,
		UpdateInterval, /*bLoop=*/true);
}

void UOHSMQuestNavigationComponent::TickUpdate()
{
	FVector Target;
	if (FindCurrentTarget(Target))
	{
		RebuildDots(Target);
	}
	else
	{
		ClearDots();
	}
}

// ─── 목표 탐색 ───────────────────────────────────────────────────────────────

bool UOHSMQuestNavigationComponent::FindQuestTargetLocation(FName QuestID, FVector& OutLocation) const
{
	if (!QuestComponent || QuestID.IsNone())
	{
		return false;
	}

	const FOHSMQuestData* Data = QuestComponent->GetQuestData(QuestID);
	if (!Data)
	{
		return false;
	}

	// ── Completable 상태: 모든 목표 달성 → 퀘스트를 가진 NPC에게 귀환 ──────
	if (QuestComponent->GetQuestState(QuestID) == EQuestState::Completable)
	{
		for (TActorIterator<AOHSMQuestNPC> It(GetWorld()); It; ++It)
		{
			if (It->AvailableQuestIDs.Contains(QuestID))
			{
				OutLocation = It->GetActorLocation();
				return true;
			}
		}
		return false;
	}

	// ── 진행 중인 목표 탐색 ───────────────────────────────────────────────────
	for (int32 i = 0; i < Data->Objectives.Num(); ++i)
	{
		const int32 Current = QuestComponent->GetObjectiveProgress(QuestID, i);
		if (Current >= Data->Objectives[i].RequiredCount)
		{
			continue;
		}

		const FQuestObjectiveData& Obj = Data->Objectives[i];

		if (Obj.ObjectiveType == EQuestObjectiveType::Talk)
		{
			for (TActorIterator<AOHSMQuestNPC> It(GetWorld()); It; ++It)
			{
				if (It->NPC_ID == Obj.TargetID)
				{
					OutLocation = It->GetActorLocation();
					return true;
				}
			}
		}
		else
		{
			for (TActorIterator<AOHSMQuestObjectiveMarker> It(GetWorld()); It; ++It)
			{
				if (It->QuestID == QuestID && It->ObjectiveIndex == i)
				{
					OutLocation = It->GetActorLocation();
					return true;
				}
			}
		}
	}

	return false;
}

bool UOHSMQuestNavigationComponent::FindCurrentTarget(FVector& OutLocation) const
{
	if (!QuestComponent)
	{
		return false;
	}

	if (!ManualNavQuestID.IsNone())
	{
		return FindQuestTargetLocation(ManualNavQuestID, OutLocation);
	}

	const TArray<FName>& TrackedIDs = QuestComponent->GetTrackedQuestIDs();
	if (TrackedIDs.IsEmpty())
	{
		return false;
	}

	return FindQuestTargetLocation(TrackedIDs[0], OutLocation);
}

// ─── 점 경로 빌드 ────────────────────────────────────────────────────────────

void UOHSMQuestNavigationComponent::RebuildDots(const FVector& TargetLocation)
{
	if (!IsValid(NavDotActor))
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		ClearDots();
		return;
	}

	const FVector StartLoc = Owner->GetActorLocation();

	if (FVector::Dist(StartLoc, TargetLocation) < ArrivalDistance)
	{
		ClearDots();
		return;
	}

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		GetWorld(), StartLoc, TargetLocation);

	if (!NavPath || !NavPath->IsValid() || NavPath->PathPoints.Num() < 2)
	{
		ClearDots();
		return;
	}

	NavDotActor->DotISM->ClearInstances();

	const TArray<FVector>& PathPts = NavPath->PathPoints;
	float Accumulated = DotSpacing;

	for (int32 i = 0; i < PathPts.Num() - 1; ++i)
	{
		const FVector SegStart = PathPts[i];
		const FVector SegEnd   = PathPts[i + 1];
		const float   SegLen   = FVector::Dist(SegStart, SegEnd);
		const FVector SegDir   = (SegEnd - SegStart).GetSafeNormal();

		while (Accumulated < SegLen)
		{
			FVector DotPos = SegStart + SegDir * Accumulated;
			DotPos.Z += 5.f;

			FRotator DotRot(0.f, SegDir.Rotation().Yaw, 0.f);

			NavDotActor->DotISM->AddInstance(FTransform(DotRot, DotPos, DotScale));
			Accumulated += DotSpacing;
		}
		Accumulated -= SegLen;
	}
}

void UOHSMQuestNavigationComponent::ClearDots()
{
	if (IsValid(NavDotActor))
	{
		NavDotActor->DotISM->ClearInstances();
	}
}
