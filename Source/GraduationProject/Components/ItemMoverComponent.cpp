#include "ItemMoverComponent.h"
#include "ContainerInteractComponent.h"
#include "InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "GraduationProject/GlobalVarsAndFuncs.h"
#include "GraduationProject/GraduationProjectCharacter.h"
#include "GraduationProject/CraftingSystem/ButtonInWorld.h"
#include "GraduationProject/ItemSystem/Item.h"
#include "GraduationProject/ItemSystem/ItemContainer.h"
#include "Kismet/GameplayStatics.h"

UItemMoverComponent::UItemMoverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UItemMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AGraduationProjectCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

void UItemMoverComponent::TryPickupOrDrop()
{
	//Get actor under cursor
	CalculateHitUnderMouse();
	
	if(!HitUnderMouse.GetActor()) return; //Didn't hit anything

	const TObjectPtr<AActor> HitActor = HitUnderMouse.GetActor();
	
	if(HeldItem) //Trying to drop item
	{
		//Drop item
		if(!HeldItem->OverrideDrop)
		{
			DropItem();
		}else //Item wants to decide itself when it can be dropped. Bind to its delegate who will be called when it wants to be dropped
		{
			if(!HeldItem->OnRequestedDrop.Contains(this, FName("DropItem")))
			{
				HeldItem->OnRequestedDrop.AddDynamic(this, &UItemMoverComponent::DropItem);
			}
			//Do item's overriden drop action
			HeldItem->OverridenDropAction();
		}
		
	}else //Trying to pickup item
	{
		if(HitActor->IsA(AItem::StaticClass())) //Collided with item, pick it up
		{
			PickupItem(Cast<AItem>(HitActor));
			
		}else if(HitActor->IsA(AButtonInWorld::StaticClass())) //Collided with a button, press it
		{
			Cast<AButtonInWorld>(HitActor)->Press();
		}
	}
}

void UItemMoverComponent::PickupItem(AItem* Item)
{
	if(HeldItem) return; //Already holding item

	//If item is glued to something, use its root instead
	Item = Item->GetLowestRootItem();

	//Remove item from container if it is in one
	if(Item->InsideThisContainer)
	{
		if(!Item->InsideThisContainer->RemoveItem(Item)) return; //Try to remove item from container. If failed, return
	}

	//Set held item
	HeldItem = Item;
	HeldItem->User = Player;
	
	//Attach item to mouse position
	Item->TurnOffItemCollision();
	Item->SetActorRotation(Player->InventoryComponent->InventoryContainer->GetActorRotation());
	GetWorld()->GetTimerManager().SetTimer(MoveHeldItemTimerHandle, this, &UItemMoverComponent::MoveHeldItemToMousePos, GetWorld()->GetDeltaSeconds(), true);
}

void UItemMoverComponent::DropItem()
{
	if(!HeldItem) return;

	//Trying to place item in a container
	if(HoveredContainer)
	{
		if(!HoveredContainer->AddItem(HeldItem, HeldItem->GetActorLocation())) return; //Try to place item in container at given coords. If failed, return
	}else
	{
		if(!HitUnderMouse.bBlockingHit) return; //Trying to place item in the air
		if(FVector::DotProduct(HitUnderMouse.ImpactNormal, FVector::UpVector) < FlatGroundThreshold) return; //Trying to place item on the side of a wall

		//Remove held item's user
		HeldItem->User = nullptr;
	}
	
	//Release held item
	GetWorld()->GetTimerManager().ClearTimer(MoveHeldItemTimerHandle);
	HeldItem->TurnOnItemCollision();
	
	HeldItem = nullptr;
}

void UItemMoverComponent::MoveHeldItemToMousePos()
{
	if(Player->IsTryingToRotateInventory) return; //Don't move item while rotating inventory
	
	CalculateHitUnderMouse();
	
	//Get held item's potential new location at distance from hit
	//Get constrained direction pointing towards hit (inverse of normal)
	EConstrainedDirection TowardsHitDirection;
	if(Player->ContainerInteractComponent->IsInteracting) //If interacting with a container, use it's forward for a more accurate forward
	{
		TowardsHitDirection = GlobalVarsAndFuncs::GetConstrainedDirection(-HitUnderMouse.ImpactNormal, Player->ContainerInteractComponent->InteractingContainer->GetActorForwardVector(), true);
	}else //Otherwise, use player's forward
	{
		TowardsHitDirection = GlobalVarsAndFuncs::GetConstrainedDirection(-HitUnderMouse.ImpactNormal, Player->GetActorForwardVector(), true);
	}
	//Get distance from hit to hold item at 
	const float Distance = ((float)HeldItem->GetDistanceFromRootInDirection(TowardsHitDirection) * GlobalVarsAndFuncs::NodeSize) + (GlobalVarsAndFuncs::NodeSize / 2.f);

	//Get potential location to hold item at. If hit something, use hit location plus distance to hold item at. Otherwise, hold item at max distance
	const FVector PotentialLoc = HitUnderMouse.bBlockingHit ? HitUnderMouse.Location + HitUnderMouse.ImpactNormal * Distance : HitUnderMouse.TraceEnd;
	
	//If hovering over a container, try to show preview of item's location in it
	if(HoveredContainer)
	{
		//Check if item could be placed in the container at its current location
		const FIntVector PotentialNodeCoords = HoveredContainer->WorldToNodeCoords(PotentialLoc);
		for(int i = 0; i < 5; i++) //Check if item fits. If it doesn't, check if it fits one unit higher up (until the end of the container's height)
		{
			const FIntVector UpwardsShiftedNodeCoords = PotentialNodeCoords + FIntVector{0, 0, i};
			if(HoveredContainer->CanItemBePlacedHere(HeldItem, UpwardsShiftedNodeCoords)) //Item fits
			{
				HeldItem->SetActorLocation(HoveredContainer->NodeToWorldLocation(UpwardsShiftedNodeCoords)); //Set item's preview location in the container
				return;
			}
		}
	}
	
	//If not hovering over a container or item doesn't fit in it, set item's location straight to potential location
	HeldItem->SetActorLocation(PotentialLoc);
}

void UItemMoverComponent::CalculateHitUnderMouse()
{
	//Get position of mouse on screen
	FVector2D ScreenPosition;
	PlayerController->GetLocalPlayer()->ViewportClient->GetMousePosition(ScreenPosition);
	
	//Get direction from camera to where mouse is hovering over
	FVector WorldPosition; //This won't be used ever, it only exists cause it's needed for DeprojectScreenToWorld
	FVector WorldDirection;
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPosition, WorldPosition, WorldDirection);
	
	//Get hit under mouse
	const FVector TraceStart = Player->Camera->GetComponentLocation();
	const FVector TraceEnd = Player->Camera->GetComponentLocation() + WorldDirection * MovingItemRange;
	FCollisionQueryParams Params = {};
	Params.bTraceComplex = false;
	GetWorld()->LineTraceSingleByChannel(HitUnderMouse, TraceStart, TraceEnd, ECC_Visibility, Params);

	//Get actor under mouse to update hovered container
	const TObjectPtr<AActor> HitActor = HitUnderMouse.GetActor();
	
	//Didn't hit anything
	if(!HitActor)
	{
		if(HoveredContainer != nullptr)
		{
			HoveredContainer = nullptr;
		}
		return;
	}

	//Collided with container, update hovered container
	if(HitActor->IsA(AItemContainer::StaticClass()))
	{
		if(HoveredContainer != HitActor)
		{
			HoveredContainer = Cast<AItemContainer>(HitActor);
		}
		return;
	}

	//Collided with item, check if it's inside a container
	if(HitActor->IsA(AItem::StaticClass()))
	{
		//Get item
		TObjectPtr<AItem> GottenItem = Cast<AItem>(HitActor);
		GottenItem = GottenItem->GetLowestRootItem();
		
		//Get item's container
		const TObjectPtr<AItemContainer> ItemsContainer = GottenItem->InsideThisContainer;
		if(HoveredContainer != ItemsContainer)
		{
			HoveredContainer = ItemsContainer;
		}
		return;
	}

	//Collided with some other actor
	if(HoveredContainer != nullptr)
	{
		HoveredContainer = nullptr;
	}
}

void UItemMoverComponent::TryRotateItem(const ERotationAxis Axis) const
{
	if(!HeldItem) return;

	HeldItem->RotateItem(Axis, Player->Camera->GetForwardVector(), Player->GetActorForwardVector());
}
