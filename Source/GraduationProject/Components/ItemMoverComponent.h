#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemMoverComponent.generated.h"

enum EConstrainedDirection : int;
class AGraduationProjectCharacter;
enum ERotationAxis : int;
class AItemContainer;
class AItem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRADUATIONPROJECT_API UItemMoverComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AGraduationProjectCharacter> Player;
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	//Moving item
	UPROPERTY(EditAnywhere)
	float MovingItemRange = 1000.f;
	void MoveHeldItemToMousePos();
	FTimerHandle MoveHeldItemTimerHandle;
	FHitResult HitUnderMouse;
	void CalculateHitUnderMouse();

	//Pickup and dropping item
	UFUNCTION()
	void PickupItem(AItem* Item);
public:
	UFUNCTION()
	void DropItem();

private:
	UPROPERTY(EditAnywhere, meta = (UIMin = "0", UIMax = "1"))
	float FlatGroundThreshold = 0.8f;

	//Hovered container
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItemContainer> HoveredContainer;
	
public:
	UItemMoverComponent();
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AItem> HeldItem;
	void TryPickupOrDrop();
	void TryRotateItem(ERotationAxis Axis) const;
};
