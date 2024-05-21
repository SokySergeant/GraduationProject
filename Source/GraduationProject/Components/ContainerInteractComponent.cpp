#include "ContainerInteractComponent.h"
#include "InventoryComponent.h"
#include "ItemMoverComponent.h"
#include "Components/SphereComponent.h"
#include "GraduationProject/GraduationProjectCharacter.h"
#include "GraduationProject/ItemSystem/ItemContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UContainerInteractComponent::UContainerInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UContainerInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AGraduationProjectCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void UContainerInteractComponent::ToggleInteract()
{
	if(IsInteracting) //End interaction
	{
		if(Player->ItemMoverComponent->HeldItem) return; //Don't allow while holding item
		
		//Stop interaction
		InteractingContainer->SetCanBeUsed(false);
		InteractingContainer = nullptr;

		//Close inventory
		Player->InventoryComponent->CloseInventory();
		
		IsInteracting = false;
		
	}else //Start interaction
	{
		if(Player->InventoryComponent->IsInventoryOpen) return; //Don't allow while inventory is open
		
		//Get overlapping container
		TArray<TObjectPtr<AActor>> OverlappingActors;
		Player->SphereTrigger->GetOverlappingActors(OverlappingActors, AItemContainer::StaticClass());
		for(int i = 0; i < OverlappingActors.Num(); i++)
		{
			const TObjectPtr<AItemContainer> GottenContainer = Cast<AItemContainer>(OverlappingActors[i]);
			
			if(!GottenContainer->Interactable) continue; //Ignore containers that aren't interactable

			InteractingContainer = GottenContainer;
			break;
		}

		//No overlapping container was found
		if(!InteractingContainer) return;

		//Overlapping container was found, interact with it
		IsInteracting = true;

		//Move player to container
		const FVector NewPlayerLoc = UKismetMathLibrary::TransformLocation(InteractingContainer->GetActorTransform(), InteractingContainer->PlayerOffsetToContainer);
		Player->SetActorLocation(NewPlayerLoc);
		Player->SetActorRotation(InteractingContainer->GetActorRotation());

		//Allow container to be used
		InteractingContainer->SetCanBeUsed(true);
		
		//Open inventory
		const FVector NewCamTarget = UKismetMathLibrary::InverseTransformLocation(Player->GetActorTransform(), UKismetMathLibrary::TransformLocation(InteractingContainer->GetActorTransform(), InteractingContainer->CamOffsetToContainer));
		Player->InventoryComponent->OpenInventory(NewCamTarget, true);
		
	}
}
