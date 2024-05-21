#include "EnemyBase.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GraduationProject/GraduationProjectCharacter.h"
#include "GraduationProject/Components/HealthComponent.h"
#include "GraduationProject/Components/LootDropComponent.h"
#include "GraduationProject/ItemSystem/ItemTool.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->RequiredToolType = DamageDealer;
	
	LootDropComponent = CreateDefaultSubobject<ULootDropComponent>(TEXT("LootDropComponent"));

	SphereTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("AttackTrigger"));
	SphereTrigger->SetupAttachment(RootComponent);

	OnAttackNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OnAttackNiagaraComponent"));
	OnAttackNiagaraComponent->SetupAttachment(RootComponent);

	//Set auto possess
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::Die);
}

void AEnemyBase::Attack() const
{
	//Get overlapping player(s)
	TArray<TObjectPtr<AActor>> OverlappingPlayers;
	SphereTrigger->GetOverlappingActors(OverlappingPlayers, AGraduationProjectCharacter::StaticClass());

	//Damage all players found
	for(int i = 0; i < OverlappingPlayers.Num(); i++)
	{
		OverlappingPlayers[i]->GetComponentByClass<UHealthComponent>()->UpdateHealthBy(-Damage);
	}

	//Play vfx
	OnAttackNiagaraComponent->Activate(true);
}

void AEnemyBase::Die(bool WrongToolUsed)
{
	Destroy();
}
