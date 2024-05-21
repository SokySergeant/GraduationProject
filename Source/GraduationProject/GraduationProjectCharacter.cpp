#include "GraduationProjectCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/ContainerInteractComponent.h"
#include "Components/HealthComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/ItemMoverComponent.h"
#include "Components/SphereComponent.h"
#include "ItemSystem/Item.h"

AGraduationProjectCharacter::AGraduationProjectCharacter()
{
	//Create camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	//Create follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	//Create inventory component
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	//Create item mover component
	ItemMoverComponent = CreateDefaultSubobject<UItemMoverComponent>(TEXT("ItemMoverComponent"));

	//Create container interact component
	ContainerInteractComponent = CreateDefaultSubobject<UContainerInteractComponent>(TEXT("ContainerInteractComponent"));

	//Create interact trigger
	SphereTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("InteractTrigger"));
	SphereTrigger->SetupAttachment(RootComponent);

	//Create health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	//Create backpack mesh component
	BackpackMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackpackMeshComponent"));
	BackpackMeshComponent->SetupAttachment(GetMesh(), FName("BACKPACKSOCKET"));
}

void AGraduationProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(Controller);

	//Input setup
	SetMappingContext(InWorldMappingContext);
}

void AGraduationProjectCharacter::SetMappingContext(const TObjectPtr<UInputMappingContext> NewContext) const
{
	if (const TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(NewContext, 0);
	}
}

void AGraduationProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (const TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//In world actions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGraduationProjectCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGraduationProjectCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AGraduationProjectCharacter::Look);

		EnhancedInputComponent->BindAction(EquipItemAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::EquipItem);
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::UseItem);

		//In container actions
		EnhancedInputComponent->BindAction(PickupOrDropAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::PickupOrDrop);
		EnhancedInputComponent->BindAction(RotateInventoryAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::StartRotatingInventory);
		EnhancedInputComponent->BindAction(RotateInventoryAction, ETriggerEvent::Completed, this, &AGraduationProjectCharacter::EndRotatingInventory);
		EnhancedInputComponent->BindAction(RotateItemXAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::RotateItemX);
		EnhancedInputComponent->BindAction(RotateItemYAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::RotateItemY);
		EnhancedInputComponent->BindAction(RotateItemZAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::RotateItemZ);
		
		//Both world and container actions
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::ToggleInventory);
		EnhancedInputComponent->BindAction(ContainerInteractAction, ETriggerEvent::Started, this, &AGraduationProjectCharacter::ContainerInteract);
	}
}

void AGraduationProjectCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AGraduationProjectCharacter::Look(const FInputActionValue& Value)
{
	if(InventoryComponent->IsInventoryOpen && !IsTryingToRotateInventory) return; //Don't rotate camera if in the inventory, unless the player is holding right click

	const FVector2D MouseDelta = Value.Get<FVector2D>();
	AddControllerYawInput(MouseDelta.X);
	AddControllerPitchInput(MouseDelta.Y);
}

void AGraduationProjectCharacter::ToggleInventory(const FInputActionValue& Value)
{
	if(InventoryComponent->IsInventoryOpen && IsTryingToRotateInventory) return; //Don't let inventory close if still trying to rotate it

	//If interacting with a container, toggle container interact instead of inventory
	if(ContainerInteractComponent->IsInteracting)
	{
		ContainerInteractComponent->ToggleInteract();
		return;
	}
	
	//Toggle inventory
	InventoryComponent->ToggleInventory();
}

void AGraduationProjectCharacter::PickupOrDrop(const FInputActionValue& Value)
{
	if(InventoryComponent->IsInventoryOpen && IsTryingToRotateInventory) return; //Don't allow picking up or dropping if rotating inventory
	
	ItemMoverComponent->TryPickupOrDrop();
}

void AGraduationProjectCharacter::StartRotatingInventory(const FInputActionValue& Value)
{
	IsTryingToRotateInventory = true;
}

void AGraduationProjectCharacter::EndRotatingInventory(const FInputActionValue& Value)
{
	IsTryingToRotateInventory = false;
}

void AGraduationProjectCharacter::RotateItemX(const FInputActionValue& Value)
{
	ItemMoverComponent->TryRotateItem(X);
}

void AGraduationProjectCharacter::RotateItemY(const FInputActionValue& Value)
{
	ItemMoverComponent->TryRotateItem(Y);
}

void AGraduationProjectCharacter::RotateItemZ(const FInputActionValue& Value)
{
	ItemMoverComponent->TryRotateItem(Z);
}

void AGraduationProjectCharacter::ContainerInteract(const FInputActionValue& Value)
{
	//If only inventory is open, toggle it instead 
	if(InventoryComponent->IsInventoryOpen && !ContainerInteractComponent->IsInteracting)
	{
		InventoryComponent->ToggleInventory();
		return;
	}

	//Toggle interact
	ContainerInteractComponent->ToggleInteract();
}

void AGraduationProjectCharacter::EquipItem(const FInputActionValue& Value)
{
	InventoryComponent->TryEquipItem(Value.Get<float>());
}

void AGraduationProjectCharacter::UseItem(const FInputActionValue& Value)
{
	InventoryComponent->TryUseItem();
}

void AGraduationProjectCharacter::ShowPlayer() const
{
	GetMesh()->SetHiddenInGame(false);
	BackpackMeshComponent->SetHiddenInGame(false);
}

void AGraduationProjectCharacter::HidePlayer() const
{
	GetMesh()->SetHiddenInGame(true);
	BackpackMeshComponent->SetHiddenInGame(true);
}

void AGraduationProjectCharacter::SwitchToWorldInput() const
{
	SetMappingContext(InWorldMappingContext);
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetInputMode(FInputModeGameOnly{});
}

void AGraduationProjectCharacter::SwitchToContainerInput() const
{
	SetMappingContext(InContainerMappingContext);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeGameAndUI{});
}

void AGraduationProjectCharacter::SwitchToUIInput() const
{
	SetMappingContext(nullptr);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeGameAndUI{});
}
