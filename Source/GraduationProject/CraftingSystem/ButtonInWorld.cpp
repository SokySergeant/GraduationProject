#include "ButtonInWorld.h"

AButtonInWorld::AButtonInWorld()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create button mesh component
	ButtonMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	ButtonMeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = ButtonMeshComponent;
}

void AButtonInWorld::Press()
{
	if(OneTimeUse && HasBeenPressed) return; //Enforce 'one time use' if turned on
	
	OnButtonPressed.Broadcast();
	
	HasBeenPressed = true;
}
