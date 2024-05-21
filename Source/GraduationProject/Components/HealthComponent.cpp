#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
}

void UHealthComponent::UpdateHealthBy(const float Health, const bool WrongToolUsed)
{
	//Update current health
	CurrentHealth += Health;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);

	//Check if died
	if(CurrentHealth == 0.f)
	{
		IsDead = true;
		OnDeath.Broadcast(WrongToolUsed);
	}
}

void UHealthComponent::Revive()
{
	CurrentHealth = MaxHealth;
	IsDead = false;
}

float UHealthComponent::GetHealthPercentage() const
{
	return CurrentHealth / MaxHealth;
}
