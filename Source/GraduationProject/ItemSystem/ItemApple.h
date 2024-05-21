#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemApple.generated.h"

class UHealthComponent;

UCLASS()
class GRADUATIONPROJECT_API AItemApple : public AItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float HealthIncrease = 20.f;

public:
	AItemApple();

	virtual void UseItem() override;
};
