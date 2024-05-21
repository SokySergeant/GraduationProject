#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LootDropComponent.generated.h"

class AItem;
class UHealthComponent;

USTRUCT()
struct FLootTableEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ChanceOfDrop = 0.f;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AItem>> ItemsToSpawn = {};
	UPROPERTY(EditAnywhere)
	int AmountMin = 0;
	UPROPERTY(EditAnywhere)
	int AmountMax = 0;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRADUATIONPROJECT_API ULootDropComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;
	
	UPROPERTY()
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> LootTable;

	//Drop
	UFUNCTION()
	void DropLoot(bool WrongToolUsed = false);
	UPROPERTY(EditAnywhere)
	float DropRange = 100.f;
	UPROPERTY(EditAnywhere)
	float WrongToolDropRateMultiplier = 0.5f;
	UPROPERTY(EditAnywhere)
	bool DropLootIfWrongToolUsed = true;

public:
	ULootDropComponent();
	virtual void BeginPlay() override;
};
