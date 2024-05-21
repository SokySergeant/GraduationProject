#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemContainer.generated.h"

enum ERequirementType : int;
class AItem;

USTRUCT(BlueprintType)
struct FContainerNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	bool IsBusy = false;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItem> Item = nullptr; 
};

USTRUCT(BlueprintType)
struct FContainerDepth
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FContainerNode> ZNodes = {};

	FContainerNode* GetNode(const int i)
	{
		return &ZNodes[i];
	}
	
	void Add(const FContainerNode& Node)
	{
		ZNodes.Add(Node);
	}
	
	int Num() const
	{
		return ZNodes.Num();
	}
};

USTRUCT(BlueprintType)
struct FContainerRow
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FContainerDepth> YDepths = {};

	FContainerDepth* GetDepth(const int i)
	{
		return &YDepths[i];
	}
	
	void Add(const FContainerDepth& Depth)
	{
		YDepths.Add(Depth);
	}

	int Num() const
	{
		return YDepths.Num();
	}
};

USTRUCT(BlueprintType)
struct FContainer
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FContainerRow> XRows = {};

	FContainerNode* operator[](const FIntVector Coords)
	{
		return GetRow(Coords.X)->GetDepth(Coords.Y)->GetNode(Coords.Z);
	}

	FContainerRow* GetRow(const int i)
	{
		return &XRows[i];
	}
	
	void Add(const FContainerRow& Row)
	{
		XRows.Add(Row);
	}
	
	int Num() const
	{
		return XRows.Num();
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCanBeUsedUpdatedDelegate, bool, Bool);

UCLASS()
class GRADUATIONPROJECT_API AItemContainer : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USceneComponent> ContainerRootComponent;

	//Container
	UPROPERTY(EditAnywhere)
	FIntVector ContainerSize = {5, 5, 5};
	UPROPERTY(VisibleAnywhere)
	FContainer Container;

	//Node
	void SetNodeInfo(FIntVector NodeCoords, TObjectPtr<AItem> NewItem = nullptr);
public:
	TObjectPtr<AItem> GetNodeItem(FIntVector NodeCoords, bool GetItemRoot = true);
	bool GetIsNodeBusy(FIntVector NodeCoords);

	FVector NodeToWorldLocation(FIntVector NodeCoords) const;
	FIntVector WorldToNodeCoords(const FVector& WorldCoords) const;
	
	TArray<TObjectPtr<AItem>> GetItemsSupports(TObjectPtr<AItem> Item, FIntVector NodeCoords = {0, 0, -1});
	TArray<TObjectPtr<AItem>> GetItemsItemIsSupporting(TObjectPtr<AItem> Item);
	
	TArray<TObjectPtr<AItem>> GetItemsToGlueTogether(const TObjectPtr<AItem> Item, const FIntVector NodeCoords);

private:
	bool AreNodeCoordsWithinContainer(FIntVector Coords) const;
	
	//Container meshes
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ContainerBasePlateMesh;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ContainerWallMeshNorth;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ContainerWallMeshSouth;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ContainerWallMeshWest;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ContainerWallMeshEast;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> AllWallMeshes;

public:
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> AllMeshes;

	AItemContainer();
	virtual void BeginPlay() override;

	//Container functionality
	bool AddItem(TObjectPtr<AItem> Item, const FVector& WorldCoords, bool Forced = false);
	bool AddItem(TObjectPtr<AItem> Item, FIntVector NodeCoords, bool Forced = false);
	bool RemoveItem(TObjectPtr<AItem> Item, bool Forced = false);
	
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AItem>> ItemsWithinContainer;

	bool CanItemBePlacedHere(TObjectPtr<AItem> Item, const FVector& WorldCoords);
	bool CanItemBePlacedHere(TObjectPtr<AItem> Item, FIntVector NodeCoords);
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<ERequirementType>> RequirementsToIgnore;

	UPROPERTY(EditAnywhere)
	bool Interactable = true;
	UPROPERTY(EditAnywhere)
	FVector PlayerOffsetToContainer = {-200.f, -200.f, 92.f};
	UPROPERTY(EditAnywhere)
	FVector CamOffsetToContainer = {0.f, 0.f, 50.f};
	
private:
	UPROPERTY(VisibleAnywhere)
	bool CanBeUsed = false;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "A value of -1 means infinite items."))
	int MaxItemsInContainer = -1; 
	
public:
	void SetCanBeUsed(bool InCanBeUsed);

	void HideObstructingMeshes(const FVector& CamLocation);

	void ShowContainer();
	void HideContainer();
	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FCanBeUsedUpdatedDelegate OnCanBeUsedUpdated;
};
