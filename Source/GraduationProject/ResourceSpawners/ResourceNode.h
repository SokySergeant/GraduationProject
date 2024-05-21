#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceNode.generated.h"

class ULootDropComponent;
class UHealthComponent;

UCLASS()
class GRADUATIONPROJECT_API AResourceNode : public AActor
{
	GENERATED_BODY()

	//Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULootDropComponent> LootDropComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	//Drop
	UPROPERTY(EditAnywhere)
	int DropAmount = 3;

	UFUNCTION()
	void CheckIfOutOfLoot(bool WrongToolUsed);

public:
	AResourceNode();
	virtual void BeginPlay() override;
};
