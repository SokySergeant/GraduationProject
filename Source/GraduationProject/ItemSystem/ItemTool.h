#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ItemTool.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UENUM()
enum EToolType
{
	None,
	DamageDealer,
	Axe,
	Shovel,
	Pickaxe
};

UCLASS()
class GRADUATIONPROJECT_API AItemTool : public AItem
{
	GENERATED_BODY()

	//Tool type
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EToolType> ToolType;
	UPROPERTY(EditAnywhere)
	float IncorrectToolTypeDmg = 10.f;
	UPROPERTY(EditAnywhere)
	float CorrectToolTypeDmg = 50.f;

	//Range
	UPROPERTY(EditAnywhere)
	float Range = 100.f;
	UPROPERTY(EditAnywhere)
	FVector Offset = {100.f, 0.f, 0.f};

	//Durability
	UPROPERTY(EditAnywhere)
	int MaxDurability = 25;
	UPROPERTY(VisibleAnywhere)
	int CurrentDurability;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDurabilityPercentage() const;

	//Vfx
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> NiagaraComponent;
	UPROPERTY(EditAnywhere)
	FVector NiagaraLocationOffset = {100.f, 0.f, 0.f};

public:
	AItemTool();
	virtual void BeginPlay() override;

	virtual void UseItem() override;
};
