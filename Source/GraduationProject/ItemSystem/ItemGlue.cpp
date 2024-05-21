#include "ItemGlue.h"
#include "ItemContainer.h"

AItemGlue::AItemGlue()
{
	PrimaryActorTick.bCanEverTick = false;

	CanBeGlued = false;
}

void AItemGlue::BeginPlay()
{
	Super::BeginPlay();

	CurrentUses = MaxUses;
}

bool AItemGlue::TakeOneGlue()
{
	//Destroy self if about to be out of uses
	if(CurrentUses == 1)
	{
		//Remove self from container if in one
		if(InsideThisContainer)
		{
			if(!InsideThisContainer->RemoveItem(this)) return false; //Failed to remove self from container. Don't allow taking glue
		}

		Destroy();
		return true;
	}
	
	//Decrement current uses
	CurrentUses--;
	return true;
}
