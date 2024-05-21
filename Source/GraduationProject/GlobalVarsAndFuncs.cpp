#include "GlobalVarsAndFuncs.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ItemSystem/Item.h"
#include "Kismet/KismetMathLibrary.h"

EConstrainedDirection GlobalVarsAndFuncs::GetConstrainedDirection(const FVector& InVector, const FVector& CoordinateSystemForwardVector, const bool TestUpDown)
{
	float BestDot = -1.f;
	EConstrainedDirection ConstrainedDir = PointingPositiveX;

	const FVector RightVector = FVector::CrossProduct(FVector::UpVector, CoordinateSystemForwardVector);
	
	//Test forward
	float Dot = FVector::DotProduct(InVector, CoordinateSystemForwardVector);
	if(Dot > BestDot)
	{
		BestDot = Dot;
		ConstrainedDir = PointingPositiveX;
	}

	//Test backwards
	Dot = FVector::DotProduct(InVector, -CoordinateSystemForwardVector);
	if(Dot > BestDot)
	{
		BestDot = Dot;
		ConstrainedDir = PointingNegativeX;
	}

	//Test right
	Dot = FVector::DotProduct(InVector, RightVector);
	if(Dot > BestDot)
	{
		BestDot = Dot;
		ConstrainedDir = PointingPositiveY;
	}

	//Test left
	Dot = FVector::DotProduct(InVector, -RightVector);
	if(Dot > BestDot)
	{
		BestDot = Dot;
		ConstrainedDir = PointingNegativeY;
	}

	if(TestUpDown)
	{
		//Test up
		Dot = FVector::DotProduct(InVector, FVector::UpVector);
		if(Dot > BestDot)
		{
			BestDot = Dot;
			ConstrainedDir = PointingPositiveZ;
		}

		//Test down
		Dot = FVector::DotProduct(InVector, -FVector::UpVector);
		if(Dot > BestDot)
		{
			ConstrainedDir = PointingNegativeZ;
		}
	}

	return ConstrainedDir;
}

TMap<EConstrainedDirection, float> GlobalVarsAndFuncs::GetConstrainedDirectionDotProducts(const FVector& InVector, const FVector& CoordinateSystemForwardVector, const bool TestUpDown)
{
	TMap<EConstrainedDirection, float> ConstrainedDirArray;
	const FVector RightVector = FVector::CrossProduct(FVector::UpVector, CoordinateSystemForwardVector);
	
	//Get forward dot
	float Dot = FVector::DotProduct(InVector, CoordinateSystemForwardVector);
	ConstrainedDirArray.Add(PointingPositiveX, Dot);

	//Get backwards dot
	Dot = FVector::DotProduct(InVector, -CoordinateSystemForwardVector);
	ConstrainedDirArray.Add(PointingNegativeX, Dot);

	//Get right dot
	Dot = FVector::DotProduct(InVector, RightVector);
	ConstrainedDirArray.Add(PointingPositiveY, Dot);

	//Get left dot
	Dot = FVector::DotProduct(InVector, -RightVector);
	ConstrainedDirArray.Add(PointingNegativeY, Dot);

	if(TestUpDown)
	{
		//Get up dot
		Dot = FVector::DotProduct(InVector, FVector::UpVector);
		ConstrainedDirArray.Add(PointingPositiveZ, Dot);

		//Get down dot
		Dot = FVector::DotProduct(InVector, -FVector::UpVector);
		ConstrainedDirArray.Add(PointingNegativeZ, Dot);
	}

	return ConstrainedDirArray;
}

FVector GlobalVarsAndFuncs::GetConstrainedVector(const FVector& InVector, const FVector& CoordinateSystemForwardVector, const bool TestUpDown)
{
	const EConstrainedDirection Dir = GetConstrainedDirection(InVector, CoordinateSystemForwardVector, TestUpDown);
	return GetWorldVectorOfConstrainedDirection(Dir, CoordinateSystemForwardVector);
}

EConstrainedDirection GlobalVarsAndFuncs::GetRotatedConstrainedDirection(const EConstrainedDirection& InDirection, const ERotationAxis& Axis, const bool& Clockwise)
{
	switch (Axis)
	{
	case X:
		if(Clockwise)
		{
			return DirectionInfo[InDirection].ClockwiseXDir;
		}else
		{
			return DirectionInfo[InDirection].CounterClockwiseXDir;
		}
		
	case Y:
		if(Clockwise)
		{
			return DirectionInfo[InDirection].ClockwiseYDir;
		}else
		{
			return DirectionInfo[InDirection].CounterClockwiseYDir;
		}

	case Z:
		if(Clockwise)
		{
			return DirectionInfo[InDirection].ClockwiseZDir;
		}else
		{
			return DirectionInfo[InDirection].CounterClockwiseZDir;
		}
		
	default:
		return DirectionInfo[InDirection].ClockwiseXDir;
	}
}

FVector GlobalVarsAndFuncs::GetWorldVectorOfConstrainedDirection(const EConstrainedDirection& InDirection, const FVector& CoordinateSystemForwardVector)
{
	const FVector LocalDir = ConstrainedDirectionVectors[InDirection];
	
	float DeltaAngle = UKismetMathLibrary::DegAcos(FVector::DotProduct(FVector::ForwardVector, CoordinateSystemForwardVector));

	//If InDirection is in the 2nd or 3rd quadrant, make angle negative so it goes in the left direction instead of right
	if(FVector::DotProduct(CoordinateSystemForwardVector, FVector::RightVector) < 0.f)
	{
		DeltaAngle = -DeltaAngle;
	}

	const FVector WorldDir = LocalDir.RotateAngleAxis(DeltaAngle, FVector::UpVector);
	
	return WorldDir;
}

EConstrainedDirection GlobalVarsAndFuncs::GetOppositeConstrainedDirection(const EConstrainedDirection& InDirection)
{
	switch (InDirection)
	{
	case PointingPositiveX:
		return PointingNegativeX;

	case PointingNegativeX:
		return PointingPositiveX;

	case PointingPositiveY:
		return PointingNegativeY;

	case PointingNegativeY:
		return PointingPositiveY;

	case PointingPositiveZ:
		return PointingNegativeZ;

	case PointingNegativeZ:
		return PointingPositiveZ;

	default:
		return PointingPositiveX;
	}
}

int GlobalVarsAndFuncs::FIntVectorMagnitude(const FIntVector& InVector)
{
	return FMath::Abs(InVector.X) + FMath::Abs(InVector.Y) + FMath::Abs(InVector.Z);
}

int GlobalVarsAndFuncs::GetIncrementedArrayIndex(int CurrentIndex, const int& ArrayLength, const bool& Forwards)
{
	if(ArrayLength == 0) return 0;

	//Make sure currentIndex is within array length
	CurrentIndex = FMath::Clamp(CurrentIndex, 0, ArrayLength - 1);
	
	if(Forwards) //Increment
	{
		if(CurrentIndex == ArrayLength - 1) //If at the end of array, wrap back to 0
		{
			CurrentIndex = 0;
		}else
		{
			CurrentIndex++;
		}
	}else //Decrement
	{
		if(CurrentIndex == 0) //If at the beginning of array, wrap back to max
		{
			CurrentIndex = ArrayLength - 1;
		}else
		{
			CurrentIndex--;
		}
	}

	return CurrentIndex;
}

TArray<FString> GlobalVarsAndFuncs::GetAllItemsNames()
{
	TArray<FString> ReturnArray;

	//Get all item asset names
	for(int i = 0; i < AssetDatas.Num(); i++)
	{
		ReturnArray.Add(AssetDatas[i].AssetName.ToString());
	}

	//Sort array alphabetically
	ReturnArray.Sort([](const FString& Name1, const FString& Name2)
	{
		return Name1 < Name2;
	});
	
	return ReturnArray;
}

void GlobalVarsAndFuncs::RefreshAllItems()
{
	//Get asset registry
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	//Get all classes tags deriving from AItem
	TMultiMap<FName, FString> TagsValues;
	for(FThreadSafeObjectIterator Iterator = FThreadSafeObjectIterator{AItem::StaticClass()}; Iterator; ++Iterator)
	{
		const UClass* CurrentClass = Iterator->GetClass();
		
		if(CurrentClass->IsNative() && CurrentClass->ClassDefaultObject == *Iterator) //Make sure class is native and derives from AItem
		{
			TagsValues.AddUnique(FBlueprintTags::NativeParentClassPath, FObjectPropertyBase::GetExportPath(CurrentClass));
		}
	}

	//Get all blueprints deriving from AItem or another class that derives from AItem
	AssetDatas.Empty();
	AssetRegistryModule.Get().GetAssetsByTagValues(TagsValues, AssetDatas);
}

UClass* GlobalVarsAndFuncs::GetItemsAssetClass(FString AssetName)
{
	//Find asset data with given name
	const FAssetData* AssetData = AssetDatas.FindByPredicate([AssetName](const FAssetData& InData){ return InData.AssetName.ToString() == AssetName; });

	//Load asset's blueprint
	const FString ItemPath = AssetData->GetObjectPathString().Append("_C");
	return StaticLoadClass(UObject::StaticClass(), nullptr, *ItemPath);
}
