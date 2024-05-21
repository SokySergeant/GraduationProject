#include "ItemRecipeBook.h"
#include "Blueprint/UserWidget.h"
#include "GraduationProject/GraduationProjectCharacter.h"
#include "GraduationProject/RecipeEditor/RecipeEditorUtilities.h"
#include "GraduationProject/Widgets/RecipeBookWidget.h"
#include "Kismet/GameplayStatics.h"

AItemRecipeBook::AItemRecipeBook()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemRecipeBook::BeginPlay()
{
	Super::BeginPlay();

	//Get player
	Player = Cast<AGraduationProjectCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	//Get editor utilities
	EditorUtilities = Cast<ARecipeEditorUtilities>(UGameplayStatics::GetActorOfClass(GetWorld(), ARecipeEditorUtilities::StaticClass()));

	//Create widget
	RecipeBookWidget = Cast<URecipeBookWidget>(CreateWidget(GetWorld(), RecipeBookWidgetTemplate));
	RecipeBookWidget->AddToViewport();
	RecipeBookWidget->MyRecipeBook = this;
	RecipeBookWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AItemRecipeBook::UseItem()
{
	Super::UseItem();

	//Show widget
	EditorUtilities->StartEditor(false);
	RecipeBookWidget->SetVisibility(ESlateVisibility::Visible);

	//Switch player input
	Player->SwitchToUIInput();
}

void AItemRecipeBook::EndUseItem()
{
	Super::EndUseItem();
	
	//Hide widget
	RecipeBookWidget->SetVisibility(ESlateVisibility::Hidden);
	EditorUtilities->EndEditor(false);

	//Switch player input
	Player->SwitchToWorldInput();
}
