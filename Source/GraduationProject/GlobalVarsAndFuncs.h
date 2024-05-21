#pragma once

#include "CoreMinimal.h"
#include "GlobalVarsAndFuncs.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoParamDelegate);

UENUM(BlueprintType)
enum EConstrainedDirection
{
	PointingPositiveX,
	PointingNegativeX,
	PointingPositiveY,
	PointingNegativeY,
	PointingPositiveZ,
	PointingNegativeZ
};

UENUM(BlueprintType)
enum ERotationAxis
{
	X,
	Y,
	Z
};

USTRUCT()
struct FConstrainedDirectionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EConstrainedDirection> CounterClockwiseXDir = PointingPositiveX;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EConstrainedDirection> ClockwiseXDir = PointingPositiveX;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EConstrainedDirection> CounterClockwiseYDir = PointingPositiveX;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EConstrainedDirection> ClockwiseYDir = PointingPositiveX;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EConstrainedDirection> CounterClockwiseZDir = PointingPositiveX;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EConstrainedDirection> ClockwiseZDir = PointingPositiveX;
};

static class GlobalVarsAndFuncs
{
public:
	static constexpr float NodeSize = 25.f;

	inline static FIntVector CraftingContainerSize = FIntVector{5, 5, 5};

	//Constrained direction functions
	static EConstrainedDirection GetConstrainedDirection(const FVector& InVector, const FVector& CoordinateSystemForwardVector, bool TestUpDown = false);
	static TMap<EConstrainedDirection, float> GetConstrainedDirectionDotProducts(const FVector& InVector, const FVector& CoordinateSystemForwardVector, bool TestUpDown = false);
	static FVector GetConstrainedVector(const FVector& InVector, const FVector& CoordinateSystemForwardVector, bool TestUpDown = false);
	static EConstrainedDirection GetRotatedConstrainedDirection(const EConstrainedDirection& InDirection, const ERotationAxis& Axis, const bool& Clockwise);
	static FVector GetWorldVectorOfConstrainedDirection(const EConstrainedDirection& InDirection, const FVector& CoordinateSystemForwardVector);
	static EConstrainedDirection GetOppositeConstrainedDirection(const EConstrainedDirection& InDirection);

	//Other helper functions
	static int FIntVectorMagnitude(const FIntVector& InVector);
	static int GetIncrementedArrayIndex(int CurrentIndex, const int& ArrayLength, const bool& Forwards);

private:
	inline static TArray<FAssetData> AssetDatas;
public:
	static TArray<FString> GetAllItemsNames();
	static void RefreshAllItems();
	static UClass* GetItemsAssetClass(FString AssetName);
	
private:
	inline static TMap<TEnumAsByte<EConstrainedDirection>, FConstrainedDirectionInfo> DirectionInfo =
	{
		{PointingPositiveX, FConstrainedDirectionInfo{PointingPositiveX, PointingPositiveX, PointingNegativeZ, PointingPositiveZ, PointingNegativeY, PointingPositiveY}},
		{PointingNegativeX, FConstrainedDirectionInfo{PointingNegativeX, PointingNegativeX, PointingPositiveZ, PointingNegativeZ, PointingPositiveY, PointingNegativeY}},
		{PointingPositiveY, FConstrainedDirectionInfo{PointingPositiveZ, PointingNegativeZ, PointingPositiveY, PointingPositiveY, PointingPositiveX, PointingNegativeX}},
		{PointingNegativeY, FConstrainedDirectionInfo{PointingNegativeZ, PointingPositiveZ, PointingNegativeY, PointingNegativeY, PointingNegativeX, PointingPositiveX}},
		{PointingPositiveZ, FConstrainedDirectionInfo{PointingNegativeY, PointingPositiveY, PointingPositiveX, PointingNegativeX, PointingPositiveZ, PointingPositiveZ}},
		{PointingNegativeZ, FConstrainedDirectionInfo{PointingPositiveY, PointingNegativeY, PointingNegativeX, PointingPositiveX, PointingNegativeZ, PointingNegativeZ}},
	};

	inline static TMap<EConstrainedDirection, FVector> ConstrainedDirectionVectors
	{
        {PointingPositiveX, FVector{1.f, 0.f, 0.f}},
		{PointingNegativeX, FVector{-1.f, 0.f, 0.f}},
		{PointingPositiveY, FVector{0.f, 1.f, 0.f}},
		{PointingNegativeY, FVector{0.f, -1.f, 0.f}},
		{PointingPositiveZ, FVector{0.f, 0.f, 1.f}},
		{PointingNegativeZ, FVector{0.f, 0.f, -1.f}},
	};
};
