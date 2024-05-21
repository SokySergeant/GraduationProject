#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemRecipeBook.generated.h"

class URecipeBookWidget;
class AGraduationProjectCharacter;
class ARecipeEditorUtilities;

UCLASS()
class GRADUATIONPROJECT_API AItemRecipeBook : public AItem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<AGraduationProjectCharacter> Player;

	UPROPERTY(EditAnywhere)
	TSubclassOf<URecipeBookWidget> RecipeBookWidgetTemplate;
	UPROPERTY()
	TObjectPtr<URecipeBookWidget> RecipeBookWidget;

	UPROPERTY()
	TObjectPtr<ARecipeEditorUtilities> EditorUtilities;

public:
	AItemRecipeBook();
	virtual void BeginPlay() override;

	virtual void UseItem() override;

	UFUNCTION(BlueprintCallable)
	virtual void EndUseItem() override;
};
