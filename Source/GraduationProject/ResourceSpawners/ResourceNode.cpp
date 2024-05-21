#include "ResourceNode.h"
#include "GraduationProject/Components/HealthComponent.h"
#include "GraduationProject/Components/LootDropComponent.h"
#include "GraduationProject/ItemSystem/ItemTool.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->RequiredToolType = Axe;
	
	LootDropComponent = CreateDefaultSubobject<ULootDropComponent>(TEXT("LootDropComponent"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void AResourceNode::BeginPlay()
{
	Super::BeginPlay();

	//Bind to OnDeath delegate
	HealthComponent->OnDeath.AddDynamic(this, &AResourceNode::CheckIfOutOfLoot);
}

void AResourceNode::CheckIfOutOfLoot(bool WrongToolUsed)
{
	DropAmount--;
	
	if(DropAmount == 0) //Destroy resource node if dropped enough loot
	{
		Destroy();
	}else //Didn't run out of loot yet, revive to allow player to destroy node again
	{
		HealthComponent->Revive();
	}
}