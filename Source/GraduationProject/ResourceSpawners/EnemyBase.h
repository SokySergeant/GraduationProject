#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UNiagaraComponent;
class USphereComponent;
class ULootDropComponent;
class UHealthComponent;

UCLASS()
class GRADUATIONPROJECT_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

	//Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULootDropComponent> LootDropComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereTrigger;

	//Death
	UFUNCTION()
	void Die(bool WrongToolUsed);

	//Attack
	UPROPERTY(EditAnywhere)
	float Damage = 20;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> OnAttackNiagaraComponent;

public:
	AEnemyBase();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Attack() const;
};
