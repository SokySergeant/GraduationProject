#include "ItemGlueBrush.h"
#include "ItemContainer.h"
#include "ItemGlue.h"
#include "Kismet/GameplayStatics.h"

AItemGlueBrush::AItemGlueBrush()
{
	PrimaryActorTick.bCanEverTick = false;
	
	OverrideDrop = true;

	CanBeGlued = false;
}

void AItemGlueBrush::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	OnItemAddedToContainer.AddDynamic(this, &AItemGlueBrush::GetHomeContainer);
}

void AItemGlueBrush::GetHomeContainer()
{
	HomeContainer = InsideThisContainer;
	OnItemAddedToContainer.RemoveDynamic(this, &AItemGlueBrush::GetHomeContainer);
}

void AItemGlueBrush::OverridenDropAction()
{
	if(!HomeContainer) return;
	
	if(HomeContainer->CanItemBePlacedHere(this, GetActorLocation())) //If hovering over it's home container, drop item in it
	{
		OnRequestedDrop.Broadcast();
		OnRequestedDrop.Clear();
		
	}else //Trying to grab or place glue
	{
		//Sweep at brush tip location
		TArray<FHitResult> SphereHits;
		const FVector SweepLoc = GetActorLocation() + ItemMesh->GetForwardVector() * (GlobalVarsAndFuncs::NodeSize / 2.f);
		const FCollisionShape Sphere = FCollisionShape::MakeSphere(GlobalVarsAndFuncs::NodeSize / 4.f);
		GetWorld()->SweepMultiByChannel(SphereHits, SweepLoc, SweepLoc, FQuat::Identity, ECC_Visibility, Sphere);

		//get item at brush tip (if it exists)
		FHitResult Hit;
		for (const auto SphereHit : SphereHits)
		{
			if(SphereHit.GetActor() == this) continue; //Ignore self
			if(!SphereHit.GetActor()->IsA(AItem::StaticClass())) continue; //Hit actor isn't an item
			
			Hit = SphereHit;
			break;
		}

		if(!Hit.GetActor()) return; //No actor found

		//If actor is a glue item, try to wet the brush with it
		if(Hit.GetActor()->IsA(AItemGlue::StaticClass()))
		{
			if(IsBrushWet) return; //Brush is already wet, no need to wet it again
			
			//Try taking one glue from item. If successful, wet the brush
			const TObjectPtr<AItemGlue> GottenItemGlue = Cast<AItemGlue>(Hit.GetActor());
			if(GottenItemGlue->TakeOneGlue()) 
			{
				IsBrushWet = true;
				ItemMesh->SetStaticMesh(BrushGluedMesh);
			}

			return; //Finished wetting brush
		}

		//If actor isn't a glue item, it must be another item. Try to apply glue to it
		
		if(!IsBrushWet) return; //Don't have glue, return
		
		//Get item
		TObjectPtr<AItem> GottenItem = Cast<AItem>(Hit.GetActor());
		GottenItem = GottenItem->GetLowestRootItem();
		
		if(!GottenItem->CanBeGlued) return; //Don't glue item if can't be glued

		//Get shape piece data
		EConstrainedDirection NormalDir = GlobalVarsAndFuncs::GetConstrainedDirection(-ItemMesh->GetForwardVector(), GottenItem->GetActorForwardVector(), true);
		FItemShapePieceData* ShapePieceData = GottenItem->GetNearestItemShapePiece(SweepLoc);
		
		if(ShapePieceData->GlueSpots[NormalDir]) return; //Face already has glue on it

		//Apply glue to item's face
		ShapePieceData->GlueSpots[NormalDir] = true;

		//Remove glue from brush
		IsBrushWet = false;
		ItemMesh->SetStaticMesh(BrushUngluedMesh);
		
		//Spawn glue mesh at face location
		FVector SpawnLoc = GottenItem->GetShapePieceFaceWorldLocation(ShapePieceData, NormalDir);
		FRotator SpawnRot = GlobalVarsAndFuncs::GetWorldVectorOfConstrainedDirection(NormalDir, GottenItem->GetActorForwardVector()).Rotation();
		TObjectPtr<AActor> GlueMesh = GetWorld()->SpawnActor(GlueMeshTemplate, &SpawnLoc, &SpawnRot);

		//Attach glue mesh to item mesh
		GlueMesh->AttachToComponent(GottenItem->ItemMesh, FAttachmentTransformRules::KeepWorldTransform);
		GottenItem->GlueActors.Add(GlueMesh);
	}
}
