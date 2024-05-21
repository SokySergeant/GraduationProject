#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GraduationProject/GlobalVarsAndFuncs.h"
#include "Item.generated.h"

class AItem;
class AItemContainer;

UENUM(BlueprintType)
enum ERequirementType
{
	SizeRequirement,
	GravityRequirement
};

USTRUCT(BlueprintType)
struct FItemShapePieceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FIntVector Coords = {};
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItem> Item = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TMap<TEnumAsByte<EConstrainedDirection>, bool> GlueSpots =
	{
		{PointingPositiveX, false},
		{PointingNegativeX, false},
		{PointingPositiveY, false},
		{PointingNegativeY, false},
		{PointingPositiveZ, false},
		{PointingNegativeZ, false},
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotateItem, ERotationAxis, Axis);

UCLASS()
class GRADUATIONPROJECT_API AItem : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USceneComponent> ItemRootComponent;
	void CalculatePoints();

	//Rotation
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> MeshRotationAlphaCurve;
	UPROPERTY(EditAnywhere)
	float RotationTime = 0.1f;
	float CurrentRotationTime = 0.f;
	FRotator PreviousRotation;
	FRotator TargetRotation;
	
	UFUNCTION()
	void RotateItemMesh();
	FTimerHandle RotateItemMeshTimerHandle;

public:
	void RotateItem(ERotationAxis Axis, const FVector& LookForwardVector, const FVector& CoordinateSystemForwardVector, bool DoRotationAnimation = true);
	void RotateItemShapeData(ERotationAxis Axis, bool XClockwise, bool YClockwise);
	UPROPERTY(BlueprintAssignable)
	FOnRotateItem OnRotateItem;

	//Setup
	AItem();
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	//Mesh
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ItemMesh;
	
	//Functionality
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> User;
	virtual void UseItem() {};
	virtual void EndUseItem() {};
	
	UPROPERTY(EditAnywhere)
	bool OverrideDrop = false;
	virtual void OverridenDropAction() {};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItemContainer> InsideThisContainer;

private:
	//Cooldown
	UPROPERTY(EditAnywhere)
	float CooldownTime = 0.f;
	FTimerHandle CooldownTimerHandle;
	void FinishCooldown() {};
public:
	void StartCooldown();
	bool IsInCooldown() const;
	
	//Item shape
	UPROPERTY(VisibleAnywhere)
	TArray<FItemShapePieceData> ItemShapeData;
	void CalculateItemShape();
private:
	FVector ShapeCoordsToWorldLocation(FIntVector ShapeCoords) const;

public:
	FItemShapePieceData* GetNearestItemShapePiece(const FVector& WorldLocation);
	FItemShapePieceData* GetShapePieceAtCoords(FIntVector ShapeCoords);
	FVector GetShapePieceFaceWorldLocation(const FItemShapePieceData* ShapePieceData, EConstrainedDirection Direction) const;
	
	//Attached items
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AItem>> AttachedItems;
	void GlueOtherItemToThisItem(TObjectPtr<AItem> NewItem, FIntVector ShapeCoords);
	void GlueItemsTogether(TArray<TObjectPtr<AItem>> ItemsToGlueTogether, FIntVector MyNodeCoords);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItem> RootItem;
	TObjectPtr<AItem> GetLowestRootItem();
	
	//Delegates
	FNoParamDelegate OnRequestedDrop;
	FNoParamDelegate OnItemAddedToContainer;
	FNoParamDelegate OnItemRemovedFromContainer;

	//Points
	UPROPERTY(VisibleAnywhere)
	TArray<int> BottomPointsIndexes;
	UPROPERTY(VisibleAnywhere)
	TArray<int> TopPointsIndexes;
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AItem>> SupportItemsInContainer;
	UPROPERTY(VisibleAnywhere)
	FIntVector CoordsInContainer;

private:
	UPROPERTY(VisibleAnywhere)
	TMap<TEnumAsByte<EConstrainedDirection>, int> ExtremityPoints =
	{
		{PointingPositiveX, 0},
		{PointingNegativeX, 0},
		{PointingPositiveY, 0},
		{PointingNegativeY, 0},
		{PointingPositiveZ, 0},
		{PointingNegativeZ, 0}
	};

public:
	int GetDistanceFromRootInDirection(EConstrainedDirection Direction);

	//Placement in container requirements
	bool CheckAllRequirements(TObjectPtr<AItemContainer> Container, FIntVector NodeCoords, const TArray<TEnumAsByte<ERequirementType>>& RequirementsToIgnore = {});
private:
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<ERequirementType>> Requirements = {SizeRequirement, GravityRequirement};

	bool CheckSizeRequirement(TObjectPtr<AItemContainer> Container, FIntVector NodeCoords);
	bool CheckGravityRequirement(TObjectPtr<AItemContainer> Container, FIntVector NodeCoords);

public:
	//Glue
	UPROPERTY(EditAnywhere)
	bool CanBeGlued = true;
	UPROPERTY()
	TArray<TObjectPtr<AActor>> GlueActors;

	//Other
	void HideItem();
	void ShowItem();

	void TurnOnItemCollision();
	void TurnOffItemCollision();
};
