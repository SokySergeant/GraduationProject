#include "ItemApple.h"
#include "GraduationProject/Components/HealthComponent.h"

AItemApple::AItemApple()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemApple::UseItem()
{
	Super::UseItem();

	User->GetComponentByClass<UHealthComponent>()->UpdateHealthBy(HealthIncrease);

	Destroy();
}
