#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GraduationProject/GlobalVarsAndFuncs.h"
#include "ButtonInWorld.generated.h"

UCLASS()
class GRADUATIONPROJECT_API AButtonInWorld : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ButtonMeshComponent;
	
	bool HasBeenPressed = false;

	UPROPERTY(EditAnywhere)
	bool OneTimeUse = false;

public:
	AButtonInWorld();

	void Press();
	
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnButtonPressed;
};
