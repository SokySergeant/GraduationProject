#include "CraftingContainer.h"
#include "ButtonInWorld.h"
#include "RecipesDataAsset.h"
#include "GraduationProject/ItemSystem/ItemGlueBrush.h"
#include "Kismet/KismetMathLibrary.h"

ACraftingContainer::ACraftingContainer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACraftingContainer::BeginPlay()
{
	Super::BeginPlay();

	//Create crafting button
	const FVector CraftingButtonSpawnLocation = UKismetMathLibrary::TransformLocation(GetActorTransform(), CraftingButtonLocationOffset);
	CraftingButton = GetWorld()->SpawnActor<AButtonInWorld>(CraftingButtonTemplate, CraftingButtonSpawnLocation, GetActorRotation());
	CraftingButton->OnButtonPressed.AddDynamic(this, &ACraftingContainer::TryCraftingOnItemUpdated);

	//Create glue brush container
	const FVector GlueBrushContainerLocation = UKismetMathLibrary::TransformLocation(GetActorTransform(), GlueBrushContainerLocationOffset);
	GlueBrushContainer = GetWorld()->SpawnActor<AItemContainer>(GlueBrushContainerTemplate, GlueBrushContainerLocation, GetActorRotation());
	GlueBrushContainer->Interactable = false;
	
	//Create glue brush
	GlueBrush = GetWorld()->SpawnActor<AItemGlueBrush>(GlueBrushTemplate, GlueBrushContainerLocation, GetActorRotation());
	//Add glue brush to glue brush container
	GlueBrushContainer->AddItem(GlueBrush, FIntVector::ZeroValue, true);

	//Make glue brush container be usable when the crafting container is used
	OnCanBeUsedUpdated.AddDynamic(this, &ACraftingContainer::SetBrushContainerCanBeUsed);
}

void ACraftingContainer::TryCraftingOnItemUpdated()
{
	RecipesDataAsset->TryCraftItem(this);
}

void ACraftingContainer::SetBrushContainerCanBeUsed(const bool NewCanBeUsed)
{
	GlueBrushContainer->SetCanBeUsed(NewCanBeUsed);
}