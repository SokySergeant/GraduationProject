#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

enum EToolType : int;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, bool, WrongToolUsed);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRADUATIONPROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere)
	float CurrentHealth = 0.f;

public:
	UHealthComponent();
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	bool IsDead = false;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EToolType> RequiredToolType;

	//Functions
	void UpdateHealthBy(float Health, bool WrongToolUsed = false);
	void Revive();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHealthPercentage() const;

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnDeathDelegate OnDeath;
};
