#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemGlueBrush.generated.h"

UCLASS()
class GRADUATIONPROJECT_API AItemGlueBrush : public AItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItemContainer> HomeContainer;

	UFUNCTION()
	void GetHomeContainer();

	UPROPERTY(VisibleAnywhere)
	bool IsBrushWet = false;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> GlueMeshTemplate;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> BrushUngluedMesh;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> BrushGluedMesh;

public:
	AItemGlueBrush();
	virtual void BeginPlay() override;

	virtual void OverridenDropAction() override;
};
