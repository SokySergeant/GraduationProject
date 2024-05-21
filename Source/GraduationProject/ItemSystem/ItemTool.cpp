#include "ItemTool.h"
#include "GraduationProject/Components/HealthComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"

AItemTool::AItemTool()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create niagara component
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
}

void AItemTool::BeginPlay()
{
	Super::BeginPlay();

	CurrentDurability = MaxDurability;
}

void AItemTool::UseItem()
{
	if(IsInCooldown()) return;
	
	Super::UseItem();

	//Get all hit actors
	const FCollisionShape Shape = FCollisionShape::MakeSphere(Range);
	TArray<FHitResult> Hits;
	FCollisionQueryParams Params = {};
	Params.AddIgnoredActor(User); //Ignore user of tool
	const FVector SweepLoc = UKismetMathLibrary::TransformLocation(User->GetActorTransform(), Offset);
	GetWorld()->SweepMultiByChannel(Hits, SweepLoc, SweepLoc, FQuat::Identity, ECC_WorldDynamic, Shape, Params);

	//Check if any hit actor has a health component
	for(int i = 0; i < Hits.Num(); i++)
	{
		const TObjectPtr<AActor> GottenActor = Hits[i].GetActor();

		if(!GottenActor) continue; //No actor

		//Damage actor's health component
		if(const TObjectPtr<UHealthComponent> HealthComp = GottenActor->GetComponentByClass<UHealthComponent>()) //Get health component
		{
			//Deal damage
			if(HealthComp->RequiredToolType == None || HealthComp->RequiredToolType == ToolType) //Check if tool is of correct type against health component
			{
				//Deal correct tool type amount of dmg
				HealthComp->UpdateHealthBy(-CorrectToolTypeDmg);

			}else //Incorrect tool being used, deal incorrect amount of dmg
			{
				HealthComp->UpdateHealthBy(-IncorrectToolTypeDmg, true);
			}

			//Lower durability
			CurrentDurability--;

			//Check if tool broke
			if(CurrentDurability == 0)
			{
				Destroy();
			}

			break; //Only allow one actor to be damaged per tool use
		}
	}
	
	//Play vfx
	const FVector VfxLocation = UKismetMathLibrary::TransformLocation(User->GetActorTransform(), NiagaraLocationOffset);
	NiagaraComponent->SetWorldLocation(VfxLocation);
	NiagaraComponent->SetWorldRotation(User->GetActorRotation());
	NiagaraComponent->Activate(true);

	StartCooldown();
}

float AItemTool::GetDurabilityPercentage() const
{
	return (float)CurrentDurability / (float)MaxDurability;
}
