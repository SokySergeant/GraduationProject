#include "RecipeEditorUtilities.h"
#include "RecipeEditorCapture.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GraduationProject/CraftingSystem/RecipesDataAsset.h"
#include "GraduationProject/ItemSystem/Item.h"
#include "Kismet/KismetMathLibrary.h"

ARecipeEditorUtilities::ARecipeEditorUtilities()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARecipeEditorUtilities::StartEditor(const bool ResetIndex)
{
	//Set default values
	CurrentCameraTransformIndex = 0;
	CurrentItemIndex = 0;
	if(ResetIndex)
	{
		CurrentRecipeIndex = 0;
	}

	//Calculate camera points
	CameraTransforms.Empty();
	const float StepAngle = 360.f / (float)CamPointAmount;
	for(int i = 0; i < CamPointAmount; i++)
	{
		const float CurrentAngle = StepAngle * i;

		//Find cam location
		FVector CamLoc = FVector::BackwardVector.RotateAngleAxis(CurrentAngle, FVector::UpVector);
		CamLoc *= CamDistance;
		CamLoc += FVector::UpVector * CamHeight;
		CamLoc += EditorWorldMiddle;

		//Find cam rotation
		const FRotator CamRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, EditorWorldMiddle + CamTargetOffset);

		//Add to camera transforms
		CameraTransforms.Add({CamRot, CamLoc, FVector::OneVector});
	}

	//Set default camera transform
	EditorCapturer->SetActorTransform(CameraTransforms[0]);
	
	//Start capturing
	EditorCapturer->ShouldBeCapturing = true;

	//Refresh all items (for the item dropdown menus)
	GlobalVarsAndFuncs::RefreshAllItems();

	//Add blueprints' name to array
	AllItemsNames = GlobalVarsAndFuncs::GetAllItemsNames();

	//Finished starting
	OnFinishedStarting.Broadcast();

	//Show recipe
	ShowRecipe();
}

void ARecipeEditorUtilities::EndEditor(const bool SortRecipes)
{
	EditorCapturer->ShouldBeCapturing = false;

	//Sort recipes in data asset
	if(SortRecipes)
	{
		RecipesDataAsset->SortRecipes();
		RecipesDataAsset->MarkPackageDirty();
	}
	
	HideRecipe();
}

void ARecipeEditorUtilities::RotateCamera(const bool Forwards)
{
	//Increment which camera transform we use
	CurrentCameraTransformIndex = GlobalVarsAndFuncs::GetIncrementedArrayIndex(CurrentCameraTransformIndex, CameraTransforms.Num(), Forwards);

	//Set new camera transform
	EditorCapturer->SetActorTransform(CameraTransforms[CurrentCameraTransformIndex]);

	//Update indicator
	UpdateCurrentItemIndicator();
}

void ARecipeEditorUtilities::GoToOtherItem(const bool Forwards)
{
	//Increment current item index
	CurrentItemIndex = GlobalVarsAndFuncs::GetIncrementedArrayIndex(CurrentItemIndex, SpawnedItems.Num(), Forwards);
	
	//Update indicator
	UpdateCurrentItemIndicator();
}

void ARecipeEditorUtilities::RotateItem(const ERotationAxis Axis)
{
	if(SpawnedItems.Num() == 0) return;

	//Rotate item
	SpawnedItems[CurrentItemIndex].Item->RotateItem(Axis, EditorCapturer->GetActorForwardVector(), FVector::ForwardVector, false);
}

void ARecipeEditorUtilities::AddNewItem(const FString ItemName)
{
	//Spawn item
	const TObjectPtr<AItem> SpawnedItem = GetWorld()->SpawnActor<AItem>(GlobalVarsAndFuncs::GetItemsAssetClass(ItemName), GetCenteredCoordsLocation(FIntVector::ZeroValue), FRotator::ZeroRotator);
	SpawnedItems.Add({SpawnedItem, FIntVector::ZeroValue});

	//Set current index to spawned item's index
	CurrentItemIndex = SpawnedItems.Num() - 1;
	
	//Update indicator
	UpdateCurrentItemIndicator();
}

void ARecipeEditorUtilities::MoveItem(const EConstrainedDirection Direction)
{
	if(SpawnedItems.Num() == 0) return;

	//Calculate needed vectors
	const FVector ConstrainedCamForward = GlobalVarsAndFuncs::GetConstrainedVector(EditorCapturer->GetActorForwardVector(), FVector::ForwardVector);
	const FVector MoveDir = GlobalVarsAndFuncs::GetWorldVectorOfConstrainedDirection(Direction, ConstrainedCamForward);
	const EConstrainedDirection MoveDirConstrained = GlobalVarsAndFuncs::GetConstrainedDirection(MoveDir, FVector::ForwardVector, true);

	//Get new coords
	FIntVector NewCoords = SpawnedItems[CurrentItemIndex].Coords;
	switch (MoveDirConstrained)
	{
	case PointingPositiveX:
		NewCoords += FIntVector{1, 0, 0};
		break;

	case PointingNegativeX:
		NewCoords += FIntVector{-1, 0, 0};
		break;

	case PointingPositiveY:
		NewCoords += FIntVector{0, 1, 0};
		break;

	case PointingNegativeY:
		NewCoords += FIntVector{0, -1, 0};
		break;

	case PointingPositiveZ:
		NewCoords += FIntVector{0, 0, 1};
		break;

	case PointingNegativeZ:
		NewCoords += FIntVector{0, 0, -1};
		break;
	}

	//Check if item is outside of crafting container size at those coords
	if(!IsCoordsWithinCraftingContainerSize(NewCoords)) return;

	//Item isn't outside crafting container size, move it to new coords
	SpawnedItems[CurrentItemIndex].Item->SetActorLocation(GetCenteredCoordsLocation(NewCoords));
	SpawnedItems[CurrentItemIndex].Coords = NewCoords;

	//Update indicator
	UpdateCurrentItemIndicator();
}

void ARecipeEditorUtilities::DeleteItem()
{
	if(SpawnedItems.Num() == 0) return;
	
	//Delete current item
	const TObjectPtr<AItem> ItemToDestroy = SpawnedItems[CurrentItemIndex].Item;
	SpawnedItems.RemoveAt(CurrentItemIndex);
	ItemToDestroy->Destroy();
	
	//Set new current index since old one was deleted
	if(SpawnedItems.Num() <= 1)
	{
		CurrentItemIndex = 0;
	}else
	{
		CurrentItemIndex = CurrentItemIndex == 0 ? 1 : CurrentItemIndex - 1;
	}
	
	//Update indicator
	UpdateCurrentItemIndicator();
}

void ARecipeEditorUtilities::DestroyProduct() const
{
	if(Product)
	{
		Product->Destroy();
		ProductCapturer->GetCaptureComponent2D()->CaptureScene();
	}
}

void ARecipeEditorUtilities::UpdateProduct(const FIntVector NewCoords, const FString NewProductName)
{
	//Destroy old product
	DestroyProduct();

	//If given coords are within crafting container size, use those. Otherwise, use product coords from current recipe
	FVector SpawnLoc;
	if(IsCoordsWithinCraftingContainerSize(NewCoords))
	{
		SpawnLoc = GetCenteredCoordsLocation(NewCoords, true);
	}else
	{
		SpawnLoc = GetCenteredCoordsLocation(RecipesDataAsset->Recipes[CurrentRecipeIndex].ProductCoords, true);
	}

	//If given a product name, find asset of that name. Otherwise, use product from current recipe
	TObjectPtr<UClass> ProductClass;
	if(NewProductName != "")
	{
		ProductClass = GlobalVarsAndFuncs::GetItemsAssetClass(NewProductName);
	}else
	{
		ProductClass = RecipesDataAsset->Recipes[CurrentRecipeIndex].ProductItem;
	}

	//Spawn new product
	Product = GetWorld()->SpawnActor<AItem>(ProductClass, SpawnLoc, FRotator::ZeroRotator);

	//Update view
	ProductCapturer->GetCaptureComponent2D()->CaptureScene();
}

void ARecipeEditorUtilities::ShowRecipe()
{
	HideRecipe();
	
	const FRecipe* RecipeToShow = &RecipesDataAsset->Recipes[CurrentRecipeIndex];
	
	//Spawn in new recipe pieces
	for(int i = 0; i < RecipeToShow->RecipePieces.Num(); i++)
	{
		if(!RecipeToShow->RecipePieces[i].Root) continue; //This piece isn't the root, don't spawn item for it

		//Spawn recipe piece's item
		const FVector SpawnLoc = GetCenteredCoordsLocation(RecipeToShow->RecipePieces[i].Coords);
		const TObjectPtr<AItem> SpawnedItem = GetWorld()->SpawnActor<AItem>(RecipeToShow->RecipePieces[i].ItemType, SpawnLoc, FRotator::ZeroRotator);
		SpawnedItem->ItemMesh->SetRelativeRotation(RecipeToShow->RecipePieces[i].Rotation);
		
		SpawnedItems.Add({SpawnedItem, RecipeToShow->RecipePieces[i].Coords});
	}
	
	//Update indicator
	UpdateCurrentItemIndicator();

	//Update product
	UpdateProduct(FIntVector{-1,-1,-1});
	
	//Delegate
	OnRecipeShownOrHidden.Broadcast();
}

void ARecipeEditorUtilities::HideRecipe()
{
	if(SpawnedItems.Num() == 0) return;
	
	//Destroy old recipe pieces
	for(int i = SpawnedItems.Num() - 1; i > -1; i--)
	{
		const TObjectPtr<AItem> ItemToDestroy = SpawnedItems[i].Item;
		SpawnedItems.RemoveAt(i);
		if(ItemToDestroy)
		{
			ItemToDestroy->Destroy();
		}
	}

	//Destroy old product
	DestroyProduct();
}

void ARecipeEditorUtilities::GoToOtherRecipe(const bool Forwards)
{
	CurrentRecipeIndex = GlobalVarsAndFuncs::GetIncrementedArrayIndex(CurrentRecipeIndex, RecipesDataAsset->Recipes.Num(), Forwards);
	CurrentItemIndex = 0;

	ShowRecipe();
}

void ARecipeEditorUtilities::NewRecipe()
{
	CurrentRecipeIndex = -1;
	
	HideRecipe();
	
	OnRecipeShownOrHidden.Broadcast();

	//Hide item indicator
	UpdateCurrentItemIndicator(true);
}

void ARecipeEditorUtilities::AddRecipe(const FString ProductName, const FIntVector ProductCoords)
{
	//Add recipe to recipe data asset
	const int NewIndex = RecipesDataAsset->Recipes.Add(CreateRecipeFromCurrentItems(ProductName, ProductCoords));
	RecipesDataAsset->MarkPackageDirty(); //This is so you have to save the data asset (otherwise it won't save between sessions)

	//Go to new created recipe
	CurrentRecipeIndex = NewIndex;
	ShowRecipe();
}

void ARecipeEditorUtilities::EditRecipe(const FString ProductName, const FIntVector ProductCoords)
{
	if(CurrentRecipeIndex == -1) //If recipe index is -1, we're editing a new recipe that isn't in the data asset. Therefore, add it as a new recipe
	{
		AddRecipe(ProductName, ProductCoords);
	}else //Else, update recipe at current index
	{
		RecipesDataAsset->Recipes[CurrentRecipeIndex] = CreateRecipeFromCurrentItems(ProductName, ProductCoords);
		RecipesDataAsset->MarkPackageDirty(); //This is so you have to save the data asset (otherwise it won't save between sessions)
		
		ShowRecipe();
	}
}

void ARecipeEditorUtilities::DeleteRecipe()
{
	if(RecipesDataAsset->Recipes.Num() == 0) return;
	
	if(CurrentRecipeIndex == -1) //If recipe index is -1, it means we're editing a new recipe not in the data asset. Therefore, just go to the last recipe in the data asset and forego any deleting
	{
		CurrentRecipeIndex = RecipesDataAsset->Recipes.Num() - 1;
	}else //Recipe index is valid, delete recipe at that index
	{
		//Delete recipe
		RecipesDataAsset->Recipes.RemoveAt(CurrentRecipeIndex);

		//Set recipe index to one lower than the recipe that was deleted
		if(RecipesDataAsset->Recipes.Num() <= 1)
		{
			CurrentRecipeIndex = 0;
		}else
		{
			CurrentRecipeIndex = CurrentRecipeIndex == 0 ? 1 : CurrentRecipeIndex - 1;
		}
	}
	
	ShowRecipe();
}

void ARecipeEditorUtilities::UpdateCurrentItemIndicator(bool Hide)
{
	//Only show indicator if in editor
	if(GetWorld()->WorldType != EWorldType::Editor)
	{
		Hide = true;
	}
	
	//If should hide or no spawned items exist, hide indicator by placing it behind the camera
	if(SpawnedItems.Num() == 0 || Hide)
	{
		CurrentItemIndicatorActor->SetActorLocation(EditorCapturer->GetActorLocation() - EditorCapturer->GetActorForwardVector() * CurrentItemIndicatorDistFromCam);
		return;
	}
	
	//Find direction to item
	const FVector ItemLoc = SpawnedItems[CurrentItemIndex].Item->GetActorLocation();
	const FVector DirToItem = (ItemLoc - EditorCapturer->GetActorLocation()).GetSafeNormal();

	//Set indicator location
	CurrentItemIndicatorActor->SetActorLocation(EditorCapturer->GetActorLocation() + DirToItem * CurrentItemIndicatorDistFromCam);
}

FString ARecipeEditorUtilities::GetCurrentRecipeProductName() const
{
	if(CurrentRecipeIndex == -1) return "";

	FString Name = RecipesDataAsset->Recipes[CurrentRecipeIndex].ProductItem->GetName();
	Name.RemoveFromEnd("_C");
	
	return Name;
}

FIntVector ARecipeEditorUtilities::GetCurrentRecipeProductCoords() const
{
	if(CurrentRecipeIndex == -1) return FIntVector::ZeroValue;

	return RecipesDataAsset->Recipes[CurrentRecipeIndex].ProductCoords;
}

FString ARecipeEditorUtilities::GetCurrentRecipeName() const
{
	if(CurrentRecipeIndex == -1) return "New recipe";

	FString Name = RecipesDataAsset->Recipes[CurrentRecipeIndex].ProductItem->GetName();

	//Remove suffix and prefix
	Name.RemoveFromEnd("_C");
	Name.RemoveFromStart("BP_");
	
	//Add spaces before all capital letters
	for(int i = Name.Len() - 1; i > 0; i--)
	{
		if(FChar::IsUpper(Name[i]))
		{
			Name.InsertAt(i, " ");
		}
	}
	
	return Name;
}

FRecipe ARecipeEditorUtilities::CreateRecipeFromCurrentItems(const FString& ProductName, FIntVector ProductCoords)
{
	//Make sure given coordinates are within crafting container size
	if(!IsCoordsWithinCraftingContainerSize(ProductCoords))
	{
		ProductCoords.X = FMath::Clamp(ProductCoords.X, 0, GlobalVarsAndFuncs::CraftingContainerSize.X - 1);
		ProductCoords.Y = FMath::Clamp(ProductCoords.Y, 0, GlobalVarsAndFuncs::CraftingContainerSize.Y - 1);
		ProductCoords.Z = FMath::Clamp(ProductCoords.Z, 0, GlobalVarsAndFuncs::CraftingContainerSize.Z - 1);
	}
	
	//Create recipe pieces from each spawned item's shape
	TArray<FRecipePiece> RecipePieces = {};
	for(int i = 0; i < SpawnedItems.Num(); i++)
	{
		SpawnedItems[i].Item->CalculateItemShape(); //Calculate item's shape here, it hasn't run yet since it usually runs in BeginPlay
		
		for(int j = 0; j < SpawnedItems[i].Item->ItemShapeData.Num(); j++)
		{
			const FIntVector NodeCoords = SpawnedItems[i].Coords + SpawnedItems[i].Item->ItemShapeData[j].Coords; //Container node coords

			//If coords are the item's root, set root to true in recipe piece
			if(SpawnedItems[i].Item->ItemShapeData[j].Coords == FIntVector{0, 0, 0}) 
			{
				RecipePieces.Add({SpawnedItems[i].Item.GetClass(), NodeCoords, true, SpawnedItems[i].Item->ItemMesh->GetComponentRotation()});
			}else
			{
				RecipePieces.Add({SpawnedItems[i].Item.GetClass(), NodeCoords, false, SpawnedItems[i].Item->ItemMesh->GetComponentRotation()});
			}
		}
	}

	//Create new recipe
	const FRecipe NewRecipe = {RecipePieces, GlobalVarsAndFuncs::GetItemsAssetClass(ProductName), ProductCoords};

	return NewRecipe;
}

FVector ARecipeEditorUtilities::GetCenteredCoordsLocation(const FIntVector Coords, const bool ProductCoords) const
{
	const FVector CenteredCoords = FVector{(float)Coords.X - ((float)GlobalVarsAndFuncs::CraftingContainerSize.X / 2.f - 0.5f), (float)Coords.Y - ((float)GlobalVarsAndFuncs::CraftingContainerSize.Y / 2.f - 0.5f), (float)Coords.Z} * GlobalVarsAndFuncs::NodeSize; // -0.5f is to center around the middle of the node, and not the corner of it

	if(ProductCoords)
	{
		return EditorProductMiddle + CenteredCoords;
	}
	
	return EditorWorldMiddle + CenteredCoords;
}

bool ARecipeEditorUtilities::IsCoordsWithinCraftingContainerSize(const FIntVector Coords) const
{
	if(Coords.X < 0 || Coords.X >= GlobalVarsAndFuncs::CraftingContainerSize.X) return false;
	if(Coords.Y < 0 || Coords.Y >= GlobalVarsAndFuncs::CraftingContainerSize.Y) return false;
	if(Coords.Z < 0 || Coords.Z >= GlobalVarsAndFuncs::CraftingContainerSize.Z) return false;

	return true;
}