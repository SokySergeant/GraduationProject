#include "LootDropComponent.h"
#include "HealthComponent.h"
#include "GraduationProject/ItemSystem/Item.h"
#include "Kismet/GameplayStatics.h"

ULootDropComponent::ULootDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULootDropComponent::BeginPlay()
{
	Super::BeginPlay();

	//Get player
	PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	//Get owner's health component
	HealthComponent = GetOwner()->GetComponentByClass<UHealthComponent>();

	//If no health component on owner, destroy self
	if(!HealthComponent)
	{
		DestroyComponent();
		return;
	}

	//Found health component, bind delegates
	HealthComponent->OnDeath.AddDynamic(this, &ULootDropComponent::DropLoot);
}

void ULootDropComponent::DropLoot(const bool WrongToolUsed)
{
	if(!DropLootIfWrongToolUsed && WrongToolUsed) return; //Enforce 'drop loot if wrong tool used'
	
	//Get all row names
	TArray<FName> AllRowNames = LootTable->GetRowNames();

	TArray<TSubclassOf<AItem>> ItemsToDrop;

	//Get items to spawn
	for(int i = 0; i < AllRowNames.Num(); i++)
	{
		//Get current entry
		FLootTableEntry* CurrentEntry = LootTable->FindRow<FLootTableEntry>(AllRowNames[i], "");

		if(CurrentEntry->ItemsToSpawn.Num() == 0) continue; //Entry has no items to spawn

		//Make sure chance of drop is within range
		float ChanceOfDrop = FMath::Clamp(CurrentEntry->ChanceOfDrop, 0.f, 1.f);

		//Apply multiplier if wrong tool used
		if(WrongToolUsed)
		{
			ChanceOfDrop *= WrongToolDropRateMultiplier;
		}
		
		//Roll for chance of drop
		if(FMath::RandRange(0.f, 1.f) > ChanceOfDrop) continue; //If failed roll, continue

		//Roll for amount of items that should drop
		const int ItemAmount = FMath::RandRange(CurrentEntry->AmountMin, CurrentEntry->AmountMax);
		
		//Get ItemAmount amount of random items to spawn
		for(int j = 0; j < ItemAmount; j++)
		{
			ItemsToDrop.Add(CurrentEntry->ItemsToSpawn[FMath::RandRange(0, CurrentEntry->ItemsToSpawn.Num() - 1)]);
		}
	}

	//Spawn items
	const float StepSize = 360.f / (float)ItemsToDrop.Num();
	const float StepRandomOffset = FMath::RandRange(0.f, 359.f);
	for(int i = 0; i < ItemsToDrop.Num(); i++)
	{
		const float CurrentStep = StepSize * i + StepRandomOffset;

		//Calculate spawn location
		FVector SpawnOffset = FVector::ForwardVector.RotateAngleAxis(CurrentStep, FVector::UpVector) * DropRange;
		FVector SpawnLoc = GetOwner()->GetActorLocation() + SpawnOffset; //Without correct Z

		//Find floor location
		TArray<FHitResult> Hits;
		GetWorld()->LineTraceMultiByChannel(Hits, SpawnLoc, SpawnLoc + FVector::DownVector * 500.f, ECC_Visibility);
		
		for(int j = 0; j < Hits.Num(); j++)
		{
			TObjectPtr<AActor> GottenActor = Hits[j].GetActor();
			if(!GottenActor) continue; //Didn't hit an actor

			if(GottenActor->IsA(APawn::StaticClass())) continue; //Hit a player or enemy or etc

			//Hit the floor, set correct floor Z location
			SpawnLoc.Z = Hits[j].Location.Z;
			break;
		}

		//Add slight offset so item is placed on the ground instead of halfway through it
		SpawnLoc += FVector::UpVector * (GlobalVarsAndFuncs::NodeSize / 2.f); 
		
		//Random rotation for spawn
		FRotator SpawnRot = FRotator{0.f, FMath::RandRange(-90.f, 269.f), 0.f};

		//Spawn item
		GetWorld()->SpawnActor(ItemsToDrop[i], &SpawnLoc, &SpawnRot);
	}
}
