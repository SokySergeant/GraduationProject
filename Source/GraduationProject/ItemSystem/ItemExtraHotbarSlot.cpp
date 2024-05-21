#include "ItemExtraHotbarSlot.h"
#include "GraduationProject/Components/InventoryComponent.h"

AItemExtraHotbarSlot::AItemExtraHotbarSlot()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemExtraHotbarSlot::UseItem()
{
	Super::UseItem();

	for(int i = 0; i < HotbarAmountToAdd; i++)
	{
		User->GetComponentByClass<UInventoryComponent>()->AddHotbarContainer();
	}
	
	Destroy();
}
