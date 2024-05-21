#include "ItemContainer.h"
#include "Item.h"
#include "Kismet/KismetMathLibrary.h"

AItemContainer::AItemContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create container root
	ContainerRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ContainerRoot"));
	RootComponent = ContainerRootComponent;
	
	//Create base plate
	ContainerBasePlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerBasePlateMesh"));
	ContainerBasePlateMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerBasePlateMesh->SetupAttachment(ContainerRootComponent);

	//Create walls
	ContainerWallMeshNorth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerWallNorth"));
	ContainerWallMeshNorth->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerWallMeshNorth->SetupAttachment(ContainerBasePlateMesh);
	ContainerWallMeshSouth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerWallSouth"));
	ContainerWallMeshSouth->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerWallMeshSouth->SetupAttachment(ContainerBasePlateMesh);
	ContainerWallMeshWest = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerWallWest"));
	ContainerWallMeshWest->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerWallMeshWest->SetupAttachment(ContainerBasePlateMesh);
	ContainerWallMeshEast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerWallEast"));
	ContainerWallMeshEast->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerWallMeshEast->SetupAttachment(ContainerBasePlateMesh);
}

void AItemContainer::BeginPlay()
{
	Super::BeginPlay();

	//Get all meshes
	AllMeshes = {ContainerBasePlateMesh, ContainerWallMeshNorth, ContainerWallMeshSouth, ContainerWallMeshWest, ContainerWallMeshEast};
	
	AllWallMeshes = AllMeshes;
	AllWallMeshes.Remove(ContainerBasePlateMesh);

	//Create container
	for(int i = 0; i < ContainerSize.X; i++)
	{
		//Create rows
		FContainerRow Row;
		for(int j = 0; j < ContainerSize.Y; j++)
		{
			//Create depths
			FContainerDepth Depth;
			for(int k = 0; k < ContainerSize.Z; k++)
			{
				FContainerNode NewNode = {false, nullptr};
				Depth.Add(NewNode);
			}
			
			Row.Add(Depth);
		}
		
		Container.Add(Row);
	}
}

bool AItemContainer::AddItem(const TObjectPtr<AItem> Item, const FVector& WorldCoords, const bool Forced)
{
	return AddItem(Item, WorldToNodeCoords(WorldCoords), Forced);
}

bool AItemContainer::AddItem(TObjectPtr<AItem> Item, FIntVector NodeCoords, const bool Forced)
{
	if(!Forced && !CanBeUsed) return false;
	
	//Check if item can be placed at these coords
	if(!Forced && !CanItemBePlacedHere(Item, NodeCoords)) return false; //Item can't be placed here. Return

	//Item can be placed here, check if item can be glued to another item
	const TArray<TObjectPtr<AItem>> ItemsToGlueTogether = GetItemsToGlueTogether(Item, NodeCoords);
	if(ItemsToGlueTogether.Num() != 0) //Can be glued, glue items together
	{
		Item->GlueItemsTogether(ItemsToGlueTogether, NodeCoords);

		//Set glued item's coords in container
		Item->CoordsInContainer = NodeCoords;
		
		//Since this item was glued to another, use that one's info instead as the added item
		Item = Item->GetLowestRootItem();
		NodeCoords = Item->CoordsInContainer;
		
	}else //Update item's support points if it's not getting glued to anything
	{
		Item->SupportItemsInContainer = GetItemsSupports(Item, NodeCoords);
	}
	
	//Add item to container
	for(int i = 0; i < Item->ItemShapeData.Num(); i++)
	{
		const FIntVector BranchCoords = NodeCoords + Item->ItemShapeData[i].Coords;
		SetNodeInfo(BranchCoords, Item->ItemShapeData[i].Item);
	}

	//Set item's new coords in container
	Item->CoordsInContainer = NodeCoords;

	//Update support point coords of all items above
	TArray<TObjectPtr<AItem>> AboveItems = GetItemsItemIsSupporting(Item);
	for(int i = 0; i < AboveItems.Num(); i++)
	{
		AboveItems[i]->SupportItemsInContainer = GetItemsSupports(AboveItems[i]);
	}
	
	//Move item actor to new location and attach it to container
	Item->SetActorLocation(NodeToWorldLocation(NodeCoords));
	Item->SetActorRotation(GetActorRotation());
	Item->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	//Set item's container
	Item->InsideThisContainer = this;

	//Finish
	ItemsWithinContainer.AddUnique(Item);

	Item->OnItemAddedToContainer.Broadcast();
	
	return true;
}

bool AItemContainer::RemoveItem(const TObjectPtr<AItem> Item, const bool Forced)
{
	if(!Forced && !CanBeUsed) return false; //If forced, remove item even if player isn't interacting with container
	
	//Check that container has this item inside it
	if(!ItemsWithinContainer.Contains(Item)) return false;

	//Check that item isn't supporting something. Check if this item were to be removed, would the above items still be supported by something else
	bool IsSupporting = false;
	TArray<TObjectPtr<AItem>> AboveItems = GetItemsItemIsSupporting(Item);
    for(int i = 0; i < AboveItems.Num(); i++)
    {
        TArray<TObjectPtr<AItem>> AboveItemsSupports = AboveItems[i]->SupportItemsInContainer;
    	AboveItemsSupports.Remove(Item);
		if(AboveItemsSupports.Num() == 0) //Above item is solely relying on this item to be supported
		{
			IsSupporting = true;
			break;
		}
    }
	if(!Forced && IsSupporting) return false; //This item is supporting something, don't allow it to be removed. If forced, allow item to be removed anyway

	//Above item isn't solely relying on this item for support (or removal is forced), remove this item from above item's supports
	for(int i = 0; i < AboveItems.Num(); i++)
	{
		AboveItems[i]->SupportItemsInContainer.Remove(Item);
	}

	//Remove item from container
	for(int i = 0; i < Container.Num(); i++)
	{
		for(int j = 0; j < Container.GetRow(i)->Num(); j++)
		{
			for(int k = 0; k < Container.GetRow(i)->GetDepth(j)->Num(); k++)
			{
				if(GetNodeItem({i, j, k}) == Item)
				{
					SetNodeInfo({i, j, k}, nullptr);
				}
			}
		}
	}

	//Detach item from container
	Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	//Set item's container info
	Item->InsideThisContainer = nullptr;

	//Finish
	ItemsWithinContainer.Remove(Item);

	Item->OnItemRemovedFromContainer.Broadcast();
	
	return true;
}

bool AItemContainer::CanItemBePlacedHere(const TObjectPtr<AItem> Item, const FVector& WorldCoords)
{
	return CanItemBePlacedHere(Item, WorldToNodeCoords(WorldCoords));
}

bool AItemContainer::CanItemBePlacedHere(const TObjectPtr<AItem> Item, const FIntVector NodeCoords)
{
	if(!CanBeUsed) return false;

	//Check if at max items allowed in container
	if(MaxItemsInContainer > -1)
	{
		if(ItemsWithinContainer.Num() + 1 > MaxItemsInContainer) return false;
	}

	//Check item's requirements
	return Item->CheckAllRequirements(this, NodeCoords, RequirementsToIgnore);
}

bool AItemContainer::GetIsNodeBusy(const FIntVector NodeCoords)
{
	//Return true if coordinates given aren't within container
	if(!AreNodeCoordsWithinContainer(NodeCoords)) return true;
	//Return node's value of IsBusy
	return Container[NodeCoords]->IsBusy;
}

TObjectPtr<AItem> AItemContainer::GetNodeItem(const FIntVector NodeCoords, const bool GetItemRoot)
{
	//Return nullptr if coordinates given aren't within container
	if(!AreNodeCoordsWithinContainer(NodeCoords)) return nullptr;
	
	//Get item at node coords
	TObjectPtr<AItem> Item = Container[NodeCoords]->Item;

	if(!Item) return nullptr;
	
	//If item is part of another item, use it's root instead if asked to
	if(GetItemRoot)
	{
		Item = Item->GetLowestRootItem();
	}
	
	return Item;
}

void AItemContainer::SetNodeInfo(const FIntVector NodeCoords, const TObjectPtr<AItem> NewItem)
{
	//Check that coords are within container
	if(!AreNodeCoordsWithinContainer(NodeCoords)) return;

	//Set node info
	Container[NodeCoords]->Item = NewItem;
	Container[NodeCoords]->IsBusy = NewItem != nullptr;
}

FVector AItemContainer::NodeToWorldLocation(const FIntVector NodeCoords) const
{
	//Calculate offset to center node coords around actor location
	const FVector CenteredOffset =
	{
		((float)NodeCoords.X - ((float)ContainerSize.X / 2.f)) * GlobalVarsAndFuncs::NodeSize,
		((float)NodeCoords.Y - ((float)ContainerSize.Y / 2.f)) * GlobalVarsAndFuncs::NodeSize,
		(float)NodeCoords.Z * GlobalVarsAndFuncs::NodeSize
	};

	return UKismetMathLibrary::TransformLocation(GetActorTransform(), CenteredOffset + FVector{GlobalVarsAndFuncs::NodeSize / 2.f}); //Slight offset so node coords are centered in the node's size
}

FIntVector AItemContainer::WorldToNodeCoords(const FVector& WorldCoords) const
{
	//Find node coordinates from world coordinates
	const FVector OffsetToUncenter = FVector{((float)ContainerSize.X / 2.f), ((float)ContainerSize.Y / 2.f), 0.f} * GlobalVarsAndFuncs::NodeSize + FVector{-GlobalVarsAndFuncs::NodeSize / 2.f};
	const FVector RelativeLocation = UKismetMathLibrary::InverseTransformLocation(GetActorTransform(), WorldCoords) + OffsetToUncenter;
	const FIntVector NodeCoords = FIntVector{FMath::RoundToInt32(RelativeLocation.X / GlobalVarsAndFuncs::NodeSize), FMath::RoundToInt32(RelativeLocation.Y / GlobalVarsAndFuncs::NodeSize), FMath::RoundToInt32(RelativeLocation.Z / GlobalVarsAndFuncs::NodeSize)};

	return NodeCoords;
}

TArray<TObjectPtr<AItem>> AItemContainer::GetItemsSupports(const TObjectPtr<AItem> Item, FIntVector NodeCoords)
{
	//If NodeCoords isn't within container, use the item's current location in container instead
	if(NodeCoords.Z < 0)
	{
		NodeCoords = Item->CoordsInContainer;
	}

	//Get support items
	TArray<TObjectPtr<AItem>> SupportItems;
	for(int i = 0; i < Item->BottomPointsIndexes.Num(); i++)
	{
		FIntVector BottomPointCoords = NodeCoords + Item->ItemShapeData[Item->BottomPointsIndexes[i]].Coords; //Coordinates of current bottom point of the item
		//Item is directly on ground of container, doesn't need supports
		if(BottomPointCoords.Z == 0)
		{
			SupportItems.AddUnique(Item);
			break;
		}

		//Item isn't placed directly on ground, check if it has supports
		const FIntVector BelowCoords = BottomPointCoords + FIntVector{0, 0, -1}; //Coordinates of where this bottom point's support should be, if it exists

		//Check if supporting point exists, if yes add it to item's support points 
		if(GetNodeItem(BelowCoords))
		{
			SupportItems.AddUnique(GetNodeItem(BelowCoords));
		}
	}

	return SupportItems;
}

TArray<TObjectPtr<AItem>> AItemContainer::GetItemsItemIsSupporting(const TObjectPtr<AItem> Item)
{
	TArray<TObjectPtr<AItem>> AboveItems;
	for(int i = 0; i < Item->TopPointsIndexes.Num(); i++)
	{
		FIntVector TopPointCoords = Item->CoordsInContainer + Item->ItemShapeData[Item->TopPointsIndexes[i]].Coords; //Coordinates of current top point of the item
		if(TopPointCoords.Z == ContainerSize.Z - 1) continue; //Point is at the very top of container, meaning it can't possibly be supporting something

		const FIntVector AboveTopPointCoords = TopPointCoords + FIntVector{0, 0, 1}; //Coordinates of what this point could possibly be supporting

		//Found an item above this one
		if(GetNodeItem(AboveTopPointCoords))
		{
			//Add to above items
			AboveItems.AddUnique(GetNodeItem(AboveTopPointCoords));
		}
	}

	return AboveItems;
}

TArray<TObjectPtr<AItem>> AItemContainer::GetItemsToGlueTogether(const TObjectPtr<AItem> Item, const FIntVector NodeCoords)
{
	TArray<TObjectPtr<AItem>> ItemsToGlueTogether;
	
	if(!Item->CanBeGlued) return ItemsToGlueTogether; //Don't glue item if can't be glued
	
	//Get all items that should be glued together
	for (auto FaceData : Item->ItemShapeData)
	{
		//Get container coords of current face
		FIntVector CurrentFaceCoords = NodeCoords + FaceData.Coords;
		
		//Coordinates of all adjacent nodes
		TMap<EConstrainedDirection, FIntVector> AdjacentCoords =
		{
			{PointingPositiveX, CurrentFaceCoords + FIntVector{1, 0, 0}},
			{PointingNegativeX, CurrentFaceCoords + FIntVector{-1, 0, 0}},
			{PointingPositiveY, CurrentFaceCoords + FIntVector{0, 1, 0}},
			{PointingNegativeY, CurrentFaceCoords + FIntVector{0, -1, 0}},
			{PointingPositiveZ, CurrentFaceCoords + FIntVector{0, 0, 1}},
			{PointingNegativeZ, CurrentFaceCoords + FIntVector{0, 0, -1}},
		};
		
		//Check if there's any adjacent items with glue between them and current item
		for(auto AdjacentCoord : AdjacentCoords)
		{
			TObjectPtr<AItem> GottenItem = GetNodeItem(AdjacentCoord.Value);
			if(!GottenItem) continue; //Found no item at adjacent coords

			if(GottenItem == Item) continue; //Don't add current item

			if(!GottenItem->CanBeGlued) continue; //Don't glue item if can't be glued

			//Check if current item has glue on it's face
			if(FaceData.GlueSpots[AdjacentCoord.Key])
			{
				//Found glue on current item's face, add gotten item to items to attach together
				ItemsToGlueTogether.AddUnique(GottenItem);
				continue;
			}

			//Didn't find glue on current item's face, check if gotten item's face has glue
			//Turn node coords into shape coords
			FIntVector ShapeCoords = AdjacentCoord.Value - GottenItem->CoordsInContainer;
			
			//Check if gotten item's face has glue
			EConstrainedDirection GottenItemFaceDirection = GlobalVarsAndFuncs::GetOppositeConstrainedDirection(AdjacentCoord.Key);
			if(GottenItem->GetShapePieceAtCoords(ShapeCoords)->GlueSpots[GottenItemFaceDirection])
			{
				//Gotten item had glue on it's face. Add to items to attach together
				ItemsToGlueTogether.AddUnique(GottenItem);
			}
		}
	}

	return ItemsToGlueTogether;
}

bool AItemContainer::AreNodeCoordsWithinContainer(const FIntVector Coords) const
{
	return Coords.X >= 0 && Coords.X < ContainerSize.X && Coords.Y >= 0 && Coords.Y < ContainerSize.Y && Coords.Z >= 0 && Coords.Z < ContainerSize.Z;
}

void AItemContainer::HideObstructingMeshes(const FVector& CamLocation)
{
	FVector ForwardLookVector = GetActorLocation() - CamLocation; //No need to normalize it here cause it happens on the next line anyway
	ForwardLookVector = FVector{ForwardLookVector.X, ForwardLookVector.Y, 0.f}.GetSafeNormal(); //Projected look forward vector onto XY plane
	TMap<EConstrainedDirection, float> ConstrainedDirectionDots = GlobalVarsAndFuncs::GetConstrainedDirectionDotProducts(ForwardLookVector, GetActorForwardVector());

	//Find obstructing meshes
	TArray<TObjectPtr<UStaticMeshComponent>> ObstructingMeshes;
	for(const auto ConstrainedDirectionDot : ConstrainedDirectionDots)
	{
		if(ConstrainedDirectionDot.Value < 0.3f) continue; //Threshold for what counts as obstructing
		
		switch (ConstrainedDirectionDot.Key)
		{
		case PointingPositiveX:
			ObstructingMeshes.Add(ContainerWallMeshSouth);
			break;

		case PointingNegativeX:
			ObstructingMeshes.Add(ContainerWallMeshNorth);
			break;

		case PointingPositiveY:
			ObstructingMeshes.Add(ContainerWallMeshWest);
			break;

		case PointingNegativeY:
			ObstructingMeshes.Add(ContainerWallMeshEast);
			break;

		default:
			break;
		}
	}

	//Hide obstructing meshes and show unobstructing ones
	for(int i = 0; i < AllWallMeshes.Num(); i++)
	{
		if(ObstructingMeshes.Contains(AllWallMeshes[i]))
		{
			AllWallMeshes[i]->SetHiddenInGame(true);
			AllWallMeshes[i]->SetCollisionProfileName(TEXT("NoCollision"));
		}else
		{
			AllWallMeshes[i]->SetHiddenInGame(false);
			AllWallMeshes[i]->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		}
	}
}

void AItemContainer::SetCanBeUsed(const bool InCanBeUsed)
{
	CanBeUsed = InCanBeUsed;
	OnCanBeUsedUpdated.Broadcast(CanBeUsed);
}

void AItemContainer::ShowContainer()
{
	//Show container
	SetActorHiddenInGame(false);
	for(int i = 0; i < AllMeshes.Num(); i++)
	{
		AllMeshes[i]->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}

	//Show all items within container
	for(int i = 0; i < ItemsWithinContainer.Num(); i++)
	{
		ItemsWithinContainer[i]->ShowItem();
		ItemsWithinContainer[i]->TurnOnItemCollision();
	}
}

void AItemContainer::HideContainer()
{
	//Hide container
	SetActorHiddenInGame(true);
	for(int i = 0; i < AllMeshes.Num(); i++)
	{
		AllMeshes[i]->SetCollisionProfileName(TEXT("NoCollision"));
	}

	//Hide all items within container
	for(int i = 0; i < ItemsWithinContainer.Num(); i++)
	{
		ItemsWithinContainer[i]->HideItem();
		ItemsWithinContainer[i]->TurnOffItemCollision();
	}
}
