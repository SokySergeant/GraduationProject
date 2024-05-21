#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ContainerInteractComponent.generated.h"

class AItemContainer;
class AGraduationProjectCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRADUATIONPROJECT_API UContainerInteractComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AGraduationProjectCharacter> Player;

public:
	UContainerInteractComponent();
	virtual void BeginPlay() override;

	void ToggleInteract();
	bool IsInteracting = false;
	UPROPERTY()
	TObjectPtr<AItemContainer> InteractingContainer;
};
