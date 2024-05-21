#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RecipesDataAsset.generated.h"

class AItemContainer;
enum EConstrainedDirection : int;
class AItem;

USTRUCT()
struct FRecipePiece
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AItem> ItemType;
	UPROPERTY(EditAnywhere)
	FIntVector Coords = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere)
	bool Root = false;
	UPROPERTY(EditAnywhere)
	FRotator Rotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FRecipePiece> RecipePieces;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AItem> ProductItem; //Item that is created from this recipe
	UPROPERTY(EditAnywhere)
	FIntVector ProductCoords = FIntVector::ZeroValue; //Location in crafting container that product item will be placed at
};

UCLASS()
class GRADUATIONPROJECT_API URecipesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FRecipe> Recipes;
	
	void TryCraftItem(TObjectPtr<AItemContainer> Container);

	void SortRecipes();
};
