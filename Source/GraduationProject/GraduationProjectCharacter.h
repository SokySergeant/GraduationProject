#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GraduationProjectCharacter.generated.h"

class UHealthComponent;
class USphereComponent;
class UContainerInteractComponent;
class UItemMoverComponent;
class USpringArmComponent;
class UCameraComponent;
class UInventoryComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(config=Game)
class AGraduationProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInventoryComponent> InventoryComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UItemMoverComponent> ItemMoverComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UContainerInteractComponent> ContainerInteractComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereTrigger;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BackpackMeshComponent;

private:
	//Input
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> InWorldMappingContext;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> InContainerMappingContext;

	//Input actions
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> ToggleInventoryAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> PickupOrDropAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> RotateInventoryAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> RotateItemXAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> RotateItemYAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> RotateItemZAction;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> ContainerInteractAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> EquipItemAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> UseItemAction;

	//Other
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;
	
public:
	AGraduationProjectCharacter();
	virtual void BeginPlay() override;

protected:
	//Input functions
	void SetMappingContext(TObjectPtr<UInputMappingContext> NewContext) const;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void ToggleInventory(const FInputActionValue& Value);
	void PickupOrDrop(const FInputActionValue& Value);
	void StartRotatingInventory(const FInputActionValue& Value);
	void EndRotatingInventory(const FInputActionValue& Value);
	void RotateItemX(const FInputActionValue& Value);
	void RotateItemY(const FInputActionValue& Value);
	void RotateItemZ(const FInputActionValue& Value);
	
	void ContainerInteract(const FInputActionValue& Value);

	void EquipItem(const FInputActionValue& Value);
	void UseItem(const FInputActionValue& Value);

public:
	bool IsTryingToRotateInventory = false;

	//Other functions
	void ShowPlayer() const;
	void HidePlayer() const;

	void SwitchToWorldInput() const;
	void SwitchToContainerInput() const;
	void SwitchToUIInput() const;
};