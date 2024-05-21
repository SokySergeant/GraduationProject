#pragma once

#include "CoreMinimal.h"
#include "GraduationProject/ItemSystem/ItemContainer.h"
#include "CraftingContainer.generated.h"

class AButtonInWorld;
class AItemGlueBrush;
class URecipesDataAsset;

UCLASS()
class GRADUATIONPROJECT_API ACraftingContainer : public AItemContainer
{
	GENERATED_BODY()

	//Crafting
	UPROPERTY(EditAnywhere)
	TObjectPtr<URecipesDataAsset> RecipesDataAsset;
	UFUNCTION()
	void TryCraftingOnItemUpdated();

	//Crafting button
	UPROPERTY(EditAnywhere)
	TSubclassOf<AButtonInWorld> CraftingButtonTemplate;
	UPROPERTY()
	TObjectPtr<AButtonInWorld> CraftingButton;
	UPROPERTY(EditAnywhere)
	FVector CraftingButtonLocationOffset = {-100.f, 50.f, 0.f};

	//Glue brush
	UPROPERTY()
	TObjectPtr<AItemGlueBrush> GlueBrush;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemGlueBrush> GlueBrushTemplate;

	//Glue brush container
	UPROPERTY()
	TObjectPtr<AItemContainer> GlueBrushContainer;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemContainer> GlueBrushContainerTemplate;
	UPROPERTY(EditAnywhere)
	FVector GlueBrushContainerLocationOffset = {-100.f, 0.f, 0.f};

	UFUNCTION()
	void SetBrushContainerCanBeUsed(bool NewCanBeUsed);

public:
	ACraftingContainer();
	virtual void BeginPlay() override;
};
