// Copyright Epic Games, Inc. All Rights Reserved.

#include "LMS_TeamProjectCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "LMSAttributeSet.h"
#include "LMS_TeamProjectPlayerState.h"
#include "LMSGameplayAbility.h"
#include "GameplayEffect.h"
#include "Weapons/LMSWeaponComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ALMS_TeamProjectCharacter

ALMS_TeamProjectCharacter::ALMS_TeamProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	WeaponComponent = CreateDefaultSubobject<ULMSWeaponComponent>(TEXT("WeaponComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)




}

UAbilitySystemComponent* ALMS_TeamProjectCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ALMS_TeamProjectCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

		InitAbilityActorInfo();
		GiveDefaultAbilities();
		ApplyDefaultEffects();
}

void ALMS_TeamProjectCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라: PlayerState가 복제돼서 도착한 시점
	InitAbilityActorInfo();
}

void ALMS_TeamProjectCharacter::InitAbilityActorInfo()
{
	// PlayerState 가져오기
	ALMS_TeamProjectPlayerState* PS = GetPlayerState<ALMS_TeamProjectPlayerState>();
	if (!PS)
	{
		return;   // 아직 준비 안 됨
	}

	// PlayerState의 ASC/AttributeSet을 캐릭터에 캐싱
	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	AttributeSet = PS->GetAttributeSet();

	// ★ 핵심: Owner=PlayerState, Avatar=이 캐릭터
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);

	if (WeaponComponent)
	{
		WeaponComponent->RefreshGrantedAbilities();
	}

	// ★ MoveSpeed 변경 구독
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AttributeSet->GetSpeedAttribute())
		.AddUObject(this, &ALMS_TeamProjectCharacter::OnSpeedChanged);

	// 초기값 즉시 반영 (구독 전 이미 세팅된 값 대비)
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetSpeed();
	}

}

void ALMS_TeamProjectCharacter::GiveDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		// ★ 어빌리티의 CDO에서 InputID 꺼내기
		const ULMSGameplayAbility* AbilityCDO =
			Cast<ULMSGameplayAbility>(AbilityClass->GetDefaultObject());

		const int32 InputID = AbilityCDO
			? (int32)AbilityCDO->AbilityInputID
			: INDEX_NONE;

		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(AbilityClass, 1, InputID, this));
	}
}

void ALMS_TeamProjectCharacter::ApplyDefaultEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}

	}
}

void ALMS_TeamProjectCharacter::OnSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ALMS_TeamProjectCharacter::OnAbilityInputPressed(ELMSAbilityInputID InputID)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputPressed((int32)InputID);
	}
}

void ALMS_TeamProjectCharacter::OnAbilityInputReleased(ELMSAbilityInputID InputID)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased((int32)InputID);
	}
}

void ALMS_TeamProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ALMS_TeamProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALMS_TeamProjectCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALMS_TeamProjectCharacter::Look);

		// Sprint (홀드)
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ALMS_TeamProjectCharacter::OnAbilityInputPressed, ELMSAbilityInputID::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALMS_TeamProjectCharacter::OnAbilityInputReleased, ELMSAbilityInputID::Sprint);

		// Dash (단발)
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ALMS_TeamProjectCharacter::OnAbilityInputPressed, ELMSAbilityInputID::Dash);

		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Started, this, &ALMS_TeamProjectCharacter::OnAbilityInputPressed, ELMSAbilityInputID::PrimaryAttack);
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Completed, this, &ALMS_TeamProjectCharacter::OnAbilityInputReleased, ELMSAbilityInputID::PrimaryAttack);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ALMS_TeamProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ALMS_TeamProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
