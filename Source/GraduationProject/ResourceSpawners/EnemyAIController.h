#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class GRADUATIONPROJECT_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBehaviorTree> BehaviorTree;

public:
	AEnemyAIController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};
