#include "InventoryComponent.h"
#include "ItemMoverComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GraduationProject/GraduationProjectCharacter.h"
#include "GraduationProject/ItemSystem/Item.h"
#include "GraduationProject/ItemSystem/ItemContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Player = Cast<AGraduationProjectCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	
	//Create inventory container
	InventoryContainer = GetWorld()->SpawnActor<AItemContainer>(InventoryContainerTemplate, FVector::ZeroVector, FRotator::ZeroRotator);
	InventoryContainer->SetHidden(true);

	//Create hotbar containers
	for(int i = 0; i < HotbarSlotAmount; i++)
	{
		AddHotbarContainer();
	}
}

void UInventoryComponent::ToggleInventory()
{
	if(IsInventoryOpen)
	{
		CloseInventory();
	}else
	{
		OpenInventory();
	}
}

void UInventoryComponent::OpenInventory(const FVector& NewCamTarget, const bool UseNewCamTarget)
{
	if(!Player || !InventoryContainer) return;
	if(IsInventoryOpen) return; //Already open
	
	//Move inventory container to player
	const FVector NewInventoryLoc = UKismetMathLibrary::TransformLocation(Player->GetActorTransform(), InventoryContainerOffset);
	InventoryContainer->SetActorLocation(NewInventoryLoc);
	InventoryContainer->SetActorRotation(Player->GetActorRotation());

	//Show item container
	InventoryContainer->ShowContainer();
	InventoryContainer->SetCanBeUsed(true);

	//Remove equipped item mesh
	RemoveEquippedItemMesh();
	
	//Move hotbar containers to player and show them
	for(int i = 0; i < HotbarContainers.Num(); i++)
	{
		//Move hotbar container to player
		FVector NewHotbarLoc = UKismetMathLibrary::TransformLocation(InventoryContainer->GetActorTransform(), HotbarOffset);
		const float HotbarSlotOffsetDistance = (i * DistBetweenHotbarContainers) - ((float)(HotbarContainers.Num() - 1) / 2.f) * DistBetweenHotbarContainers; //This distance is centered

		FVector HotbarSlotOffset; //Offset to place hotbar container in a horizontal or vertical line
		if(HotbarHorizontalPlacement)
		{
			HotbarSlotOffset = InventoryContainer->GetActorRightVector() * HotbarSlotOffsetDistance;
		}else
		{
			HotbarSlotOffset = InventoryContainer->GetActorForwardVector() * HotbarSlotOffsetDistance;
		}
		
		HotbarContainers[i]->SetActorLocation(NewHotbarLoc + HotbarSlotOffset);
		HotbarContainers[i]->SetActorRotation(InventoryContainer->GetActorRotation());

		//Show hotbar container
		HotbarContainers[i]->ShowContainer();
		HotbarContainers[i]->SetCanBeUsed(true);
	}

	//Update input
	Player->SwitchToContainerInput();

	//Hide player mesh
	Player->HidePlayer();

	//Move camera in
	PreviousCamLoc = Player->CameraBoom->GetRelativeLocation();
	if(UseNewCamTarget)
	{
		TargetCamLoc = NewCamTarget;
	}else
	{
		TargetCamLoc = InventoryContainerOffset;
	}
	CurrentCamTransitionTime = 0.f;
	GetWorld()->GetTimerManager().SetTimer(CamTransitionTimerHandle, this, &UInventoryComponent::TransitionCamera, GetWorld()->GetDeltaSeconds(), true);

	//Start hiding wall that's obstructing view
	ObstructingWallMesh = nullptr;
	GetWorld()->GetTimerManager().SetTimer(HideWallTimerHandle, this, &UInventoryComponent::HideObstructingWallMesh, GetWorld()->GetDeltaSeconds(), true);

	//Finished
	OnInventoryToggle.Broadcast(true);
	IsInventoryOpen = true;
}

void UInventoryComponent::CloseInventory()
{
	if(!Player || !InventoryContainer) return;
	if(!IsInventoryOpen) return; //Already closed
	if(Player->ItemMoverComponent->HeldItem || Player->IsTryingToRotateInventory) return; //Don't allow inventory to close if still holding an item or rotating inventory
	
	//Hide inventory container
	InventoryContainer->HideContainer();
	InventoryContainer->SetCanBeUsed(false);

	//Hide hotbar containers
	for(int i = 0; i < HotbarContainers.Num(); i++)
	{
		HotbarContainers[i]->HideContainer();
		HotbarContainers[i]->SetCanBeUsed(false);
	}

	//Update input
	Player->SwitchToWorldInput();

	//Show player mesh
	Player->ShowPlayer();

	//Move camera back
	PreviousCamLoc = Player->CameraBoom->GetRelativeLocation();
	TargetCamLoc = FVector::ZeroVector;
	CurrentCamTransitionTime = 0.f;
	GetWorld()->GetTimerManager().SetTimer(CamTransitionTimerHandle, this, &UInventoryComponent::TransitionCamera, GetWorld()->GetDeltaSeconds(), true);

	//Stop hiding wall that's obstructing view
	GetWorld()->GetTimerManager().ClearTimer(HideWallTimerHandle);

	//Update equipped item mesh
	UpdateEquippedItemMesh();

	//Finish
	OnInventoryToggle.Broadcast(false);
	IsInventoryOpen = false;
}

void UInventoryComponent::AddHotbarContainer()
{
	const TObjectPtr<AItemContainer> HotbarContainer = GetWorld()->SpawnActor<AItemContainer>(HotbarContainerTemplate, FVector::ZeroVector, FRotator::ZeroRotator);
	HotbarContainers.Add(HotbarContainer);
}

void UInventoryComponent::TryEquipItem(const int ItemIndex)
{
	//Remove old equipped item mesh
	RemoveEquippedItemMesh();
	
	//Update equipped item index, clamp within possible range
	EquippedItemIndex = FMath::Clamp(ItemIndex, 0, HotbarContainers.Num() - 1);

	//Update equipped mesh
	UpdateEquippedItemMesh();
}

void UInventoryComponent::TryUseItem() const
{
	if(!EquippedItem) return; //No item equipped

	//Use item
	EquippedItem->UseItem();
}

void UInventoryComponent::RemoveEquippedItemMesh()
{
	if(!EquippedItem) return;

	//Detach equipped item and place it back in it's hotbar container
	EquippedItem->DetachAllSceneComponents(Player->GetMesh(), FDetachmentTransformRules::KeepRelativeTransform);
	EquippedItem->HideItem();
	EquippedItem->ItemMesh->SetRelativeRotation(RecordedEquippedItemMeshRotation); //Set item mesh rotation to old rotation
	HotbarContainers[EquippedItemIndex]->AddItem(EquippedItem, RecordedEquippedItemCoordsInContainer, true); //Put equipped item back in it's hotbar slot

	//Unbind from ondestroyed
    EquippedItem->OnDestroyed.RemoveDynamic(this, &UInventoryComponent::SetEquippedItemToNull);
	
	SetEquippedItemToNull();
}

void UInventoryComponent::UpdateEquippedItemMesh()
{
	if(HotbarContainers.Num() == 0) return; //No hotbar slot exists
	if(HotbarContainers[EquippedItemIndex]->ItemsWithinContainer.Num() == 0) return; //No item in hotbar slot
	
	//Get new equipped item and remove it from it's hotbar container
	EquippedItem = HotbarContainers[EquippedItemIndex]->ItemsWithinContainer[0];
	RecordedEquippedItemCoordsInContainer = EquippedItem->CoordsInContainer; //Record old coords in container
	HotbarContainers[EquippedItemIndex]->RemoveItem(EquippedItem, true);

	//Attach equipped item to player's hand
	EquippedItem->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("EQUIPPEDITEMSOCKET"));
	RecordedEquippedItemMeshRotation = EquippedItem->ItemMesh->GetRelativeRotation(); //Record item mesh rotation to set it back when item is placed in hotbar
	EquippedItem->ItemMesh->SetRelativeRotation(EquippedItemInHandRotation); //Set item mesh rotation so it points forward when held in hand
	EquippedItem->ShowItem();

	//If the item gets destroyed, remove reference to it
	EquippedItem->OnDestroyed.AddUniqueDynamic(this, &UInventoryComponent::SetEquippedItemToNull);
	
	OnEquippedItemUpdated.Broadcast(EquippedItem);
}

void UInventoryComponent::SetEquippedItemToNull(AActor* DestroyedActor)
{
	EquippedItem = nullptr;
	
	OnEquippedItemUpdated.Broadcast(nullptr);
}

void UInventoryComponent::TransitionCamera()
{
	//Update current time
	CurrentCamTransitionTime += GetWorld()->GetTimerManager().GetTimerElapsed(CamTransitionTimerHandle);

	//Calculate alpha (using curve data)
	float Alpha = CamTransitionAlphaCurve->GetFloatValue(CurrentCamTransitionTime / CamTransitionTime);
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	
	//Set camera values
	Player->CameraBoom->SetRelativeLocation(UKismetMathLibrary::VLerp(PreviousCamLoc, TargetCamLoc, Alpha));
	
	//Turn timer off if it reaches end
	if(CurrentCamTransitionTime > CamTransitionTime)
	{
		GetWorld()->GetTimerManager().ClearTimer(CamTransitionTimerHandle);
	} 
}

void UInventoryComponent::HideObstructingWallMesh() const
{
	//Hide wall that obstructs view
	InventoryContainer->HideObstructingMeshes(Player->Camera->GetComponentLocation());
}
