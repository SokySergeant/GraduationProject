#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemExtraHotbarSlot.generated.h"

UCLASS()
class GRADUATIONPROJECT_API AItemExtraHotbarSlot : public AItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int HotbarAmountToAdd = 1;

public:
	AItemExtraHotbarSlot();

	virtual void UseItem() override;
};
