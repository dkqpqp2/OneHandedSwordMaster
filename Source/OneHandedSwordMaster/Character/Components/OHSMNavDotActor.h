#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OHSMNavDotActor.generated.h"

/**
 * 퀘스트 네비게이션 점선 경로를 ISM 으로 렌더링하는 내부 액터.
 * UOHSMQuestNavigationComponent 가 런타임에 스폰/소멸 관리한다.
 * 월드 원점(0,0,0) 에 배치되므로 인스턴스 위치 = 월드 좌표.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class ONEHANDEDSWORDMASTER_API AOHSMNavDotActor : public AActor
{
	GENERATED_BODY()

public:
	AOHSMNavDotActor();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UInstancedStaticMeshComponent> DotISM;
};
