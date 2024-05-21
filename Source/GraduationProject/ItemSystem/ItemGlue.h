#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemGlue.generated.h"

UCLASS()
class GRADUATIONPROJECT_API AItemGlue : public AItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int MaxUses = 4;
	UPROPERTY(VisibleAnywhere)
	int CurrentUses = 0;

public:
	AItemGlue();
	virtual void BeginPlay() override;

	bool TakeOneGlue();
};
