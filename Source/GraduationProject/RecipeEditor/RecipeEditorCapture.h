#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "RecipeEditorCapture.generated.h"

UCLASS()
class GRADUATIONPROJECT_API ARecipeEditorCapture : public ASceneCapture2D
{
	GENERATED_BODY()

public:
	ARecipeEditorCapture();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual bool ShouldTickIfViewportsOnly() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ShouldBeCapturing = false;
};
