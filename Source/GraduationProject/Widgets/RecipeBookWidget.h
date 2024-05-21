#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RecipeBookWidget.generated.h"

class AItemRecipeBook;

UCLASS()
class GRADUATIONPROJECT_API URecipeBookWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AItemRecipeBook> MyRecipeBook;
};
