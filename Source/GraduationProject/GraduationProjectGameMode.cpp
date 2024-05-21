#include "GraduationProjectGameMode.h"
#include "UObject/ConstructorHelpers.h"

AGraduationProjectGameMode::AGraduationProjectGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
