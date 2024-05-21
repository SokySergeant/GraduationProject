#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AItem;
class AGraduationProjectCharacter;
class AItemContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryToggleDelegate, bool, Opened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedItemUpdatedDelegate, AItem*, NewEquippedItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRADUATIONPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<AGraduationProjectCharacter> Player;

	//Inventory container
	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemContainer> InventoryContainerTemplate;
public:
	UPROPERTY()
	TObjectPtr<AItemContainer> InventoryContainer;
private:
	UPROPERTY(EditAnywhere)
	FVector InventoryContainerOffset = {100.f, 0.f, -91.5f};

	//Hotbar
	UPROPERTY(EditAnywhere)
	int HotbarSlotAmount = 3;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemContainer> HotbarContainerTemplate;
	UPROPERTY()
	TArray<TObjectPtr<AItemContainer>> HotbarContainers;
public:
	void AddHotbarContainer();
	
private:
	UPROPERTY(EditAnywhere)
	FVector HotbarOffset = {-100.f, 0.f, 0.f};
	UPROPERTY(EditAnywhere)
	float DistBetweenHotbarContainers = 100.f;
	UPROPERTY(EditAnywhere)
	bool HotbarHorizontalPlacement = true;

	//Equipped item
	int EquippedItemIndex = 0;

	UPROPERTY()
	TObjectPtr<AItem> EquippedItem;
	UPROPERTY(EditAnywhere)
	FRotator EquippedItemInHandRotation = {0.f, -90.f, 0.f};
	FRotator RecordedEquippedItemMeshRotation;
	FIntVector RecordedEquippedItemCoordsInContainer;
	void RemoveEquippedItemMesh();
	void UpdateEquippedItemMesh();

	UFUNCTION()
	void SetEquippedItemToNull(AActor* DestroyedActor = nullptr);

	//Camera transition
	FVector PreviousCamLoc;
	FVector TargetCamLoc;

	FTimerHandle CamTransitionTimerHandle;
	UFUNCTION()
	void TransitionCamera();
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> CamTransitionAlphaCurve;
	
	UPROPERTY(EditAnywhere)
	float CamTransitionTime = 0.2f;
	float CurrentCamTransitionTime = 0.f;

	//Hiding obstructing wall
	FTimerHandle HideWallTimerHandle;
	UFUNCTION()
	void HideObstructingWallMesh() const;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ObstructingWallMesh;

public:
	//Setup
	UInventoryComponent();
	virtual void BeginPlay() override;

	bool IsInventoryOpen = false;
	void ToggleInventory();
	void OpenInventory(const FVector& NewCamTarget = {}, bool UseNewCamTarget = false);
	void CloseInventory();

	//Item usage
	void TryEquipItem(int ItemIndex);
	void TryUseItem() const;
	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnInventoryToggleDelegate OnInventoryToggle;
	
	UPROPERTY(BlueprintAssignable)
	FOnEquippedItemUpdatedDelegate OnEquippedItemUpdated;
};
