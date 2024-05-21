#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GraduationProject/GlobalVarsAndFuncs.h"
#include "GraduationProject/CraftingSystem/RecipesDataAsset.h"
#include "RecipeEditorUtilities.generated.h"

enum ERotationAxis : int;
class URecipesDataAsset;
enum EConstrainedDirection : int;
class AItem;
class AItemContainer;
class ARecipeEditorCapture;

USTRUCT(BlueprintType)
struct FSpawnedRecipePieceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AItem> Item;
	FIntVector Coords;
};

UCLASS()
class GRADUATIONPROJECT_API ARecipeEditorUtilities : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<URecipesDataAsset> RecipesDataAsset;

public:
	ARecipeEditorUtilities();

	//Editor setup
	UFUNCTION(BlueprintCallable)
	void StartEditor(bool ResetIndex = true);
	UFUNCTION(BlueprintCallable)
	void EndEditor(bool SortRecipes = true);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ARecipeEditorCapture> EditorCapturer;
	UPROPERTY(EditAnywhere)
	FVector EditorWorldMiddle;

public:
	//Camera
	UFUNCTION(BlueprintCallable)
	void RotateCamera(bool Forwards);

private:
	TArray<FTransform> CameraTransforms;
	int CurrentCameraTransformIndex = 0;

	UPROPERTY(EditAnywhere)
	int CamPointAmount = 10;
	UPROPERTY(EditAnywhere)
	FVector CamTargetOffset = {0.f, 0.f, 50.f};
	UPROPERTY(EditAnywhere)
	float CamDistance = 100.f;
	UPROPERTY(EditAnywhere)
	float CamHeight = 25.f;

	//Recipe product
	UPROPERTY(EditAnywhere)
	FVector EditorProductMiddle;
	UPROPERTY(EditAnywhere)
	TObjectPtr<ARecipeEditorCapture> ProductCapturer;
	UPROPERTY()
	TObjectPtr<AItem> Product;

	void DestroyProduct() const;
public:
	UFUNCTION(BlueprintCallable)
	void UpdateProduct(FIntVector NewCoords, FString NewProductName = "");
	
	//Item manipulation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int CurrentItemIndex = 0;
	UFUNCTION(BlueprintCallable)
	void GoToOtherItem(bool Forwards);
	
	UFUNCTION(BlueprintCallable)
	void RotateItem(ERotationAxis Axis);
	
	UFUNCTION(BlueprintCallable)
	void AddNewItem(FString ItemName);
	UFUNCTION(BlueprintCallable)
	void MoveItem(EConstrainedDirection Direction);
	UFUNCTION(BlueprintCallable)
	void DeleteItem();
	
	UPROPERTY(VisibleAnywhere)
	TArray<FSpawnedRecipePieceData> SpawnedItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> AllItemsNames;
	
private:
	//Current item indicator
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> CurrentItemIndicatorActor;
	UPROPERTY(EditAnywhere)
	float CurrentItemIndicatorDistFromCam = 10.f;
	void UpdateCurrentItemIndicator(bool Hide = false);

	//Private helper functions
	FRecipe CreateRecipeFromCurrentItems(const FString& ProductName, FIntVector ProductCoords);

	FVector GetCenteredCoordsLocation(FIntVector Coords, bool ProductCoords = false) const;
	bool IsCoordsWithinCraftingContainerSize(FIntVector Coords) const;

	//Recipe data asset
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int CurrentRecipeIndex = 0;

	UFUNCTION(BlueprintCallable)
	void ShowRecipe();
	void HideRecipe();

	UFUNCTION(BlueprintCallable)
	void GoToOtherRecipe(bool Forwards);
	
	UFUNCTION(BlueprintCallable)
	void NewRecipe();
	UFUNCTION(BlueprintCallable)
	void AddRecipe(FString ProductName, FIntVector ProductCoords);
	UFUNCTION(BlueprintCallable)
	void EditRecipe(FString ProductName, FIntVector ProductCoords);
	UFUNCTION(BlueprintCallable)
	void DeleteRecipe();

	//Helper functions
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetCurrentRecipeProductName() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FIntVector GetCurrentRecipeProductCoords() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetCurrentRecipeName() const;

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnFinishedStarting;
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnRecipeShownOrHidden;
};
