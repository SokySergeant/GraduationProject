#include "RecipesDataAsset.h"
#include "GraduationProject/ItemSystem/Item.h"
#include "GraduationProject/ItemSystem/ItemContainer.h"

void URecipesDataAsset::TryCraftItem(const TObjectPtr<AItemContainer> Container)
{
	for(int i = 0; i < Recipes.Num(); i++)
	{
		TArray<TObjectPtr<AItem>> CraftingItems;
		
		for(int j = 0; j < Recipes[i].RecipePieces.Num(); j++)
		{
			//Check if item exists at recipe piece's coords
			TObjectPtr<AItem> ItemInContainer = Container->GetNodeItem(Recipes[i].RecipePieces[j].Coords, false);
			if(!ItemInContainer) goto Failed;

			//Check if item is of correct type
			if(!ItemInContainer->IsA(Recipes[i].RecipePieces[j].ItemType)) goto Failed;
			
			//Add item to crafting item array
			CraftingItems.AddUnique(ItemInContainer->GetLowestRootItem());
		}

		//Check that crafting container ONLY contains the crafting items needed
		if(Container->ItemsWithinContainer.Num() != CraftingItems.Num()) goto Failed;
		for(int j = 0; j < CraftingItems.Num(); j++)
		{
			if(!Container->ItemsWithinContainer.Contains(CraftingItems[j])) goto Failed;
		}

		//Got through all checks, craft item
		{ //This is in a block because goto Failed will throw an error otherwise because of Product not being initialized
			
			//Remove crafting items from container
			for(int j = 0; j < CraftingItems.Num(); j++)
			{
				Container->RemoveItem(CraftingItems[j], true); //(When Forcefully is true, RemoveItem can only fail if item doesn't exist in container. Therefore, it's okay to assume this removal doesn't fail in this case, since we've already assured the item exists in the container)
			}
		
			//Destroy all crafting items
			for(int j = 0; j < CraftingItems.Num(); j++)
			{
				CraftingItems[j]->Destroy();
			}

			//Spawn crafting product item
			const TObjectPtr<AItem> Product = Container->GetWorld()->SpawnActor<AItem>(Recipes[i].ProductItem, FVector::ZeroVector, FRotator::ZeroRotator);
		
			//Try adding product item to container
			if(!Container->AddItem(Product, Recipes[i].ProductCoords))
			{
				//If failed to add to container, place product item on top of it instead
				Product->SetActorLocation(Container->GetActorLocation() + FVector{0.f, 0.f, GlobalVarsAndFuncs::NodeSize / 2.f});
			}
		
			return;
		}

		Failed:
		continue; //This has to be here because Failed: otherwise throws an error since it would be pointing directly at the closing bracket
	}
}

void URecipesDataAsset::SortRecipes()
{
	Recipes.Sort([](const FRecipe& Recipe1, const FRecipe& Recipe2)
	{
		return Recipe1.ProductItem->GetName() < Recipe2.ProductItem->GetName();
	});
}