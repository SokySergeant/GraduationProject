#include "RecipeEditorCapture.h"
#include "Components/SceneCaptureComponent2D.h"

ARecipeEditorCapture::ARecipeEditorCapture()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARecipeEditorCapture::BeginPlay()
{
	Super::BeginPlay();
}

void ARecipeEditorCapture::Tick(float DeltaTime)
{
	if(!ShouldBeCapturing) return;
	
	Super::Tick(DeltaTime);

	GetCaptureComponent2D()->CaptureScene();
}

bool ARecipeEditorCapture::ShouldTickIfViewportsOnly() const
{
	return ShouldBeCapturing;
}

