#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	//Get player
	PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	RunBehaviorTree(BehaviorTree);
}

void AEnemyAIController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetBlackboardComponent()->SetValueAsVector("PlayerLoc", PlayerActor->GetActorLocation());
}
