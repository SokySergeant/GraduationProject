#include "Item.h"
#include "ItemContainer.h"
#include "GraduationProject/GlobalVarsAndFuncs.h"
#include "Kismet/KismetMathLibrary.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create item root
	ItemRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ItemRoot"));
	RootComponent = ItemRootComponent;

	//Create item mesh
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ItemMesh->SetupAttachment(ItemRootComponent);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	CalculateItemShape();
	CalculatePoints();
}

void AItem::Destroyed()
{
	Super::Destroyed();

	//Destroy attached items
	for(int i = AttachedItems.Num() - 1; i > -1; i--)
	{
		AttachedItems[i]->Destroy();
	}

	//Destroy glue actors
	for(int i = GlueActors.Num() - 1; i > -1; i--)
	{
		GlueActors[i]->Destroy();
	}
}

void AItem::CalculateItemShape()
{
	TArray<FHitResult> Hits;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.bTraceComplex = false;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GlobalVarsAndFuncs::NodeSize / 4.f);

	//Get item shape
	for(int X = -5; X <= 5; X++) //5 is an arbitrary number that could be bigger if needed. Individual items are typically not going to be bigger than 5x5x5
	{
		for(int Y = -5; Y <= 5; Y++)
		{
			for(int Z = -5; Z <= 5; Z++)
			{
				//Get coordinates
				FIntVector CurrentCoords = {X, Y, Z};
				FVector LocationToCheck = ShapeCoordsToWorldLocation(CurrentCoords);
				
				//Check if item mesh exists at this point
				GetWorld()->SweepMultiByChannel(Hits, LocationToCheck, LocationToCheck, FQuat::Identity, ECC_Visibility, Sphere, CollisionQueryParams);
				for (auto Hit : Hits)
				{
					if(Hit.GetActor() == this) //Item mesh exists at this point, add coordinates to item's shape and create face data for it
					{
						ItemShapeData.Add(FItemShapePieceData{CurrentCoords, this});
						break;
					}
				}
			}
		}
	}

	//If no shape is found, add a shape piece at root coords
	if(ItemShapeData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ITEM SHAPE NOT FOUND. MAKE SURE ITEM HAS CORRECT COLLISION SET UP. ERROR AT ITEM NAMED %s"), *GetName());
		ItemShapeData.Add(FItemShapePieceData{{0, 0, 0}, this});
	}
}

void AItem::RotateItem(ERotationAxis Axis, const FVector& LookForwardVector, const FVector& CoordinateSystemForwardVector, const bool DoRotationAnimation)
{
	//Don't allow item to rotate if still busy rotating
	if(GetWorld()->GetTimerManager().IsTimerActive(RotateItemMeshTimerHandle)) return;

	//Constrain vectors
	const EConstrainedDirection LookDir = GlobalVarsAndFuncs::GetConstrainedDirection(LookForwardVector, CoordinateSystemForwardVector);

	bool XClockwise = true;
	bool YClockwise = true;
	//No Z clockwise because Z can never be counterclockwise since the player can't roll their camera

	//Make rotation axis and rotation direction relative to player's view
	switch (LookDir) //No PointingPositiveX case because that will be true for X and Y clockwise (which are already the default values above)
	{
	case PointingNegativeX:
		XClockwise = false;
		YClockwise = false;
		
		break;
		
	case PointingPositiveY:
		if(Axis == X)
		{
			Axis = Y;
		} else if(Axis == Y)
		{
			Axis = X;
			XClockwise = false;
		}
		
		break;

	case PointingNegativeY:
		if(Axis == X)
		{
			Axis = Y;
			YClockwise = false;
		}else if(Axis == Y)
		{
			Axis = X;
		}
		
		break;
		
	default:
		break;
	}

	//Rotate item shape
	RotateItemShapeData(Axis, XClockwise, YClockwise);
	
	//Recalculate item's points
	CalculatePoints();
	
	//Find target rotation for item mesh
	switch (Axis)
	{
	case X:
		TargetRotation = UKismetMathLibrary::ComposeRotators(ItemMesh->GetRelativeRotation(), FRotator{0.f, 0.f, 90.f * (XClockwise ? 1.f : -1.f)});
		break;
		
	case Y:
		TargetRotation = UKismetMathLibrary::ComposeRotators(ItemMesh->GetRelativeRotation(), FRotator{90.f * (YClockwise ? 1.f : -1.f), 0.f, 0.f});
		break;
		
	case Z:
		TargetRotation = UKismetMathLibrary::ComposeRotators(ItemMesh->GetRelativeRotation(), FRotator{0.f, 90.f, 0.f});
		break;
	}

	//Rotation animation
	if(DoRotationAnimation)
	{
		//Start item mesh rotating animation
		CurrentRotationTime = 0.f;
		PreviousRotation = ItemMesh->GetRelativeRotation();
		GetWorld()->GetTimerManager().SetTimer(RotateItemMeshTimerHandle, this, &AItem::RotateItemMesh, GetWorld()->GetDeltaSeconds(), true);
	}else
	{
		//Set target item mesh rotation immediately
		ItemMesh->SetRelativeRotation(TargetRotation);
	}

	//Delegate
	OnRotateItem.Broadcast(Axis);
}

void AItem::RotateItemShapeData(const ERotationAxis Axis, const bool XClockwise, const bool YClockwise)
{
	for(int i = 0; i < ItemShapeData.Num(); i++)
	{
		const FIntVector PreviousCoords = ItemShapeData[i].Coords;
		FIntVector NewCoords;
		
		//Rotate shape coords
		switch (Axis)
		{
		case X:
			NewCoords = XClockwise ? FIntVector{PreviousCoords.X, PreviousCoords.Z, -PreviousCoords.Y} : FIntVector{PreviousCoords.X, -PreviousCoords.Z, PreviousCoords.Y};
			break;
		case Y:
			NewCoords = YClockwise ? FIntVector{-PreviousCoords.Z, PreviousCoords.Y, PreviousCoords.X} : FIntVector{PreviousCoords.Z, PreviousCoords.Y, -PreviousCoords.X};
			break;
		case Z:
			NewCoords = {-PreviousCoords.Y, PreviousCoords.X, PreviousCoords.Z};
			break;
		}
		
		ItemShapeData[i].Coords = NewCoords;
		
		//Find new glue spots
		TMap<TEnumAsByte<EConstrainedDirection>, bool> NewGlueSpots =
		{
			{PointingPositiveX, false},
			{PointingNegativeX, false},
			{PointingPositiveY, false},
			{PointingNegativeY, false},
			{PointingPositiveZ, false},
			{PointingNegativeZ, false},
		};
		
		for (auto GlueSpot : ItemShapeData[i].GlueSpots)
		{
			if(!GlueSpot.Value) continue; //No glue here

			//Add glue to rotated face spot
			EConstrainedDirection NewKey = GlueSpot.Key;
			switch (Axis)
			{
			case X:
				NewKey = GlobalVarsAndFuncs::GetRotatedConstrainedDirection(GlueSpot.Key, Axis, XClockwise);
				break;
			case Y:
				NewKey = GlobalVarsAndFuncs::GetRotatedConstrainedDirection(GlueSpot.Key, Axis, YClockwise);
				break;
			case Z:
				NewKey = GlobalVarsAndFuncs::GetRotatedConstrainedDirection(GlueSpot.Key, Axis, true);
				break;
			}
			NewGlueSpots[NewKey] = true;
		}

		//Set new glue spots
		ItemShapeData[i].GlueSpots = NewGlueSpots;
	}
}

void AItem::RotateItemMesh()
{
	//Update current time
	CurrentRotationTime += GetWorld()->GetTimerManager().GetTimerElapsed(RotateItemMeshTimerHandle);

	//Get new rotation (using curve data if given)
	float Alpha = 1.f;
	if(MeshRotationAlphaCurve)
	{
		Alpha = MeshRotationAlphaCurve->GetFloatValue(CurrentRotationTime / RotationTime);
	}
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	const FRotator NewRotation = FQuat::Slerp(PreviousRotation.Quaternion(), TargetRotation.Quaternion(), Alpha).Rotator();
	
	//Set new rotation
	ItemMesh->SetRelativeRotation(NewRotation);

	//Turn timer off if it reaches end
	if(CurrentRotationTime > RotationTime)
	{
		ItemMesh->SetRelativeRotation(TargetRotation);
		GetWorld()->GetTimerManager().ClearTimer(RotateItemMeshTimerHandle);
	} 
}

void AItem::CalculatePoints()
{
	//Reset point values
	for (auto& ExtremityPoint : ExtremityPoints)
	{
		ExtremityPoint.Value = 0;
	}
	
	BottomPointsIndexes.Empty();
	TopPointsIndexes.Empty();

	//Calculate new point values
	for(int i = 0; i < ItemShapeData.Num(); i++)
	{
		//Get lowest point
		if(ItemShapeData[i].Coords.Z < ExtremityPoints[PointingNegativeZ])
		{
			ExtremityPoints[PointingNegativeZ] = ItemShapeData[i].Coords.Z;
		}
		//Get highest point
		if(ItemShapeData[i].Coords.Z > ExtremityPoints[PointingPositiveZ])
		{
			ExtremityPoints[PointingPositiveZ] = ItemShapeData[i].Coords.Z;
		}
		//Get leftmost point
		if(ItemShapeData[i].Coords.Y < ExtremityPoints[PointingNegativeY])
		{
			ExtremityPoints[PointingNegativeY] = ItemShapeData[i].Coords.Y;
		}
		//Get rightmost point
		if(ItemShapeData[i].Coords.Y > ExtremityPoints[PointingPositiveY])
		{
			ExtremityPoints[PointingPositiveY] = ItemShapeData[i].Coords.Y;
		}
		//Get backmost point
		if(ItemShapeData[i].Coords.X < ExtremityPoints[PointingNegativeX])
		{
			ExtremityPoints[PointingNegativeX] = ItemShapeData[i].Coords.X;
		}
		//Get forwardmost point
		if(ItemShapeData[i].Coords.X > ExtremityPoints[PointingPositiveX])
		{
			ExtremityPoints[PointingPositiveX] = ItemShapeData[i].Coords.X;
		}
		
		//Find top points. Check if there's a point above this one. If not, add it to top points (this gets any point that doesn't directly have another point above it)
		if(!GetShapePieceAtCoords(ItemShapeData[i].Coords + FIntVector{0, 0, 1}))
		{
			TopPointsIndexes.Add(i);
		}
		
		//Find bottom points. Check if there's any points below this one (this gets any point that is the lowest in it's column. Imagine a camera placed under the item looking straight up. The points visible from that angle are the bottom points calculated here)
		bool FoundPointBelowThisOne = false;
		for(int j = 0; j < ItemShapeData.Num(); j++)
		{
			if(i == j) continue; //Don't need to check point against itself

			if(ItemShapeData[i].Coords.X == ItemShapeData[j].Coords.X && ItemShapeData[i].Coords.Y == ItemShapeData[j].Coords.Y) //Check if point is at the same X,Y coords
			{
				//Check if point is below this one
				if(ItemShapeData[i].Coords.Z > ItemShapeData[j].Coords.Z)
				{
					FoundPointBelowThisOne = true;
				}
			}
		}
		//If didn't find a point below this one, add it to bottom points
		if(!FoundPointBelowThisOne)
		{
			BottomPointsIndexes.Add(i);
		}
	}
}

int AItem::GetDistanceFromRootInDirection(const EConstrainedDirection Direction)
{
	return FMath::Abs(ExtremityPoints[Direction]);
}

bool AItem::CheckAllRequirements(const TObjectPtr<AItemContainer> Container, const FIntVector NodeCoords, const TArray<TEnumAsByte<ERequirementType>>& RequirementsToIgnore)
{
	//Check all item's requirements
	for (auto Requirement : Requirements)
	{
		if(RequirementsToIgnore.Contains(Requirement)) continue; //Ignore requirements that should be ignored
		
		switch (Requirement)
		{
		case SizeRequirement:
			if(!CheckSizeRequirement(Container, NodeCoords)) return false;
			break;
			
		case GravityRequirement:
			if(!CheckGravityRequirement(Container, NodeCoords)) return false;
			break;

		default:
			return false;
		}
	}
	
	return true;
}

bool AItem::CheckSizeRequirement(const TObjectPtr<AItemContainer> Container, const FIntVector NodeCoords)
{
	//Check if item can fit at it's location
	for(int i = 0; i < ItemShapeData.Num(); i++)
	{
		FIntVector BranchCoords = NodeCoords + ItemShapeData[i].Coords;
		if(Container->GetIsNodeBusy(BranchCoords)) return false;
	}

	return true;
}

bool AItem::CheckGravityRequirement(const TObjectPtr<AItemContainer> Container, const FIntVector NodeCoords)
{
	//Check if item can be glued to another item (Because glue will circumvent the need for supports)
	if(CanBeGlued)
	{
		if(Container->GetItemsToGlueTogether(this, NodeCoords).Num() != 0) return true;
	}
	
	//Found no items that should be glued together. Check that item isn't floating
	if(Container->GetItemsSupports(this, NodeCoords).Num() == 0) return false;

	return true;
}

FVector AItem::ShapeCoordsToWorldLocation(const FIntVector ShapeCoords) const
{
	const FVector ShapeCoordsRelativeLoc = FVector{(float)ShapeCoords.X, (float)ShapeCoords.Y, (float)ShapeCoords.Z} * GlobalVarsAndFuncs::NodeSize;
	return UKismetMathLibrary::TransformLocation(GetActorTransform(), ShapeCoordsRelativeLoc);
}

FItemShapePieceData* AItem::GetNearestItemShapePiece(const FVector& WorldLocation)
{
	FItemShapePieceData* ClosestItemShapePieceData = &ItemShapeData[0];
	float ClosestDistance = 100000.f;
	
	for(int i = 0; i < ItemShapeData.Num(); i++)
	{
		FVector FaceWorldLoc = ShapeCoordsToWorldLocation(ItemShapeData[i].Coords);
		const float Distance = FVector::Distance(WorldLocation, FaceWorldLoc);
		if(Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestItemShapePieceData = &ItemShapeData[i];
		}
	}

	return ClosestItemShapePieceData;
}

FItemShapePieceData* AItem::GetShapePieceAtCoords(FIntVector ShapeCoords)
{
	return ItemShapeData.FindByPredicate([ShapeCoords](const FItemShapePieceData& InData){ return InData.Coords == ShapeCoords; });
}

FVector AItem::GetShapePieceFaceWorldLocation(const FItemShapePieceData* ShapePieceData, const EConstrainedDirection Direction) const
{
	const FVector FaceForwardVector = GlobalVarsAndFuncs::GetWorldVectorOfConstrainedDirection(Direction, GetActorForwardVector());
	
	return ShapeCoordsToWorldLocation(ShapePieceData->Coords) + FaceForwardVector * (GlobalVarsAndFuncs::NodeSize / 2.f);
}

void AItem::StartCooldown()
{
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &AItem::FinishCooldown, CooldownTime);
}

bool AItem::IsInCooldown() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(CooldownTimerHandle);
}

void AItem::HideItem()
{
	//Hide self
	SetActorHiddenInGame(true);

	//Hide glue actors
	for(int i = 0; i < GlueActors.Num(); i++)
	{
		GlueActors[i]->SetActorHiddenInGame(true);
	}

	//Hide attached items
	for(int i = 0; i < AttachedItems.Num(); i++)
	{
		AttachedItems[i]->HideItem();
	}
}

void AItem::ShowItem()
{
	//Show self
	SetActorHiddenInGame(false);

	//Show glue actors
	for(int i = 0; i < GlueActors.Num(); i++)
	{
		GlueActors[i]->SetActorHiddenInGame(false);
	}

	//Show attached items
	for(int i = 0; i < AttachedItems.Num(); i++)
	{
		AttachedItems[i]->ShowItem();
	}
}

void AItem::TurnOnItemCollision()
{
	//Turn on self collision
	ItemMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	//Turn on attached items' collision
	for(int i = 0; i < AttachedItems.Num(); i++)
	{
		AttachedItems[i]->TurnOnItemCollision();
	}
}

void AItem::TurnOffItemCollision()
{
	//Turn off self collision
	ItemMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	//Turn off attached items' collision
	for(int i = 0; i < AttachedItems.Num(); i++)
	{
		AttachedItems[i]->TurnOffItemCollision();
	}
}

void AItem::GlueItemsTogether(TArray<TObjectPtr<AItem>> ItemsToGlueTogether, const FIntVector MyNodeCoords)
{
	//Check if got items that should be attached together
	if(ItemsToGlueTogether.Num() == 0) return;

	//Sort array based on item shape size
	ItemsToGlueTogether.Sort([](const AItem& Item1, const AItem& Item2)
	{
		return Item1.ItemShapeData.Num() > Item2.ItemShapeData.Num();
	});
	
	ItemsToGlueTogether.AddUnique(this); //Add self to array (After the sort because self should never have a chance at being the root)

	const TObjectPtr<AItem> NewRootItem = ItemsToGlueTogether[0];

	//Attach items together. They will get attached to the item with the biggest size, which is the first item in the array because of the previous sort
	for(int i = 1; i < ItemsToGlueTogether.Num(); i++)
	{
		//Find shape coords to attach at
		FIntVector ShapeCoords;
		if(ItemsToGlueTogether[i] == this) //If at current item, use given node coords instead since this item isn't placed in a container yet
		{
			ShapeCoords = MyNodeCoords - NewRootItem->CoordsInContainer;
		}else
		{
			ShapeCoords = ItemsToGlueTogether[i]->CoordsInContainer - NewRootItem->CoordsInContainer;
		}

		//Attach
		NewRootItem->GlueOtherItemToThisItem(ItemsToGlueTogether[i], ShapeCoords);
	}
}

void AItem::GlueOtherItemToThisItem(const TObjectPtr<AItem> NewItem, const FIntVector ShapeCoords)
{
	if(AttachedItems.Contains(NewItem)) return;
	
	//Add to attached items array
	AttachedItems.AddUnique(NewItem);

	//Add new item's shape to this item's shape
	for(int i = 0; i < NewItem->ItemShapeData.Num(); i++)
	{
		NewItem->ItemShapeData[i].Coords += ShapeCoords;
		ItemShapeData.Add(NewItem->ItemShapeData[i]);
	}

	//Recalculate points
	CalculatePoints();

	//Attach new item's mesh to this item's mesh
	NewItem->SetActorLocation(ShapeCoordsToWorldLocation(ShapeCoords));
	NewItem->SetActorRotation(GetActorRotation());
	NewItem->ItemMesh->AttachToComponent(ItemMesh, FAttachmentTransformRules::KeepWorldTransform);

	//Set new item's root item to this item
	NewItem->RootItem = this;
}

TObjectPtr<AItem> AItem::GetLowestRootItem()
{
	TObjectPtr<AItem> ReturnItem = this;
	while(ReturnItem->RootItem) //Go down the tree until we find the lowest root
	{
		ReturnItem = ReturnItem->RootItem;
	}
	return ReturnItem;
}