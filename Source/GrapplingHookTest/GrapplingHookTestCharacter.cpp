// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrapplingHookTestCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AGrapplingHookTestCharacter

AGrapplingHookTestCharacter::AGrapplingHookTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterSkeletalMesh"));
	SkeletalMesh->SetOnlyOwnerSee(true);
	SkeletalMesh->SetupAttachment(FirstPersonCameraComponent);
	SkeletalMesh->bCastDynamicShadow = false;
	SkeletalMesh->CastShadow = false;
	SkeletalMesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	SkeletalMesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Gun->bCastDynamicShadow = false;
	Gun->CastShadow = false;
	// Gun->SetupAttachment(SkeletalMesh, TEXT("GripPoint"));
	Gun->SetupAttachment(RootComponent);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(Gun);
	MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	SetCharacterState(CharacterState::GROUNDED);
}

void AGrapplingHookTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	Gun->AttachToComponent(SkeletalMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	SkeletalMesh->SetHiddenInGame(false, true);

	// Spawn projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((MuzzleLocation != nullptr) ? MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			// spawn the projectile at the muzzle
			Projectile = World->SpawnActor<AGrapplingHookTestProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			Projectile->Init(MuzzleLocation);
		}
	}
}

void AGrapplingHookTestCharacter::Tick(float DeltaTime)
{
	Update(DeltaTime);
}

void AGrapplingHookTestCharacter::Update(float DeltaTime)
{
	if (CharacterStateVar == CharacterState::GROUNDED)
	{
		if (StateStepVar == StateStep::ON_ENTER) {
			Grounded_Enter();
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			Grounded_Update();
		}
	}

	if (CharacterStateVar == CharacterState::SWINGING)
	{
		if (StateStepVar == StateStep::ON_ENTER) {
			Swinging_Enter(DeltaTime);
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			Swinging_Update(DeltaTime);
		}
	}
}

void AGrapplingHookTestCharacter::SetCharacterState(CharacterState newState)
{
	// Append any GameStates you add to this example to this switch statement...
	switch (CharacterStateVar)
	{
	case CharacterState::GROUNDED:
		//Docked_Exit();
		break;
	case CharacterState::SWINGING:
		//Launching_Exit();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Unexpected state has not been implemented!"), newState);
		return;
	}

	// Set new GameStates state and begin OnEnter of that state
	CharacterStateVar = newState;
	StateStepVar = StateStep::ON_ENTER;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGrapplingHookTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGrapplingHookTestCharacter::OnFire);
	PlayerInputComponent->BindAction("Retract", IE_Pressed, this, &AGrapplingHookTestCharacter::OnRetract);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AGrapplingHookTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGrapplingHookTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGrapplingHookTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGrapplingHookTestCharacter::LookUpAtRate);
}

void AGrapplingHookTestCharacter::Grounded_Enter()
{
	GetCharacterMovement()->GravityScale = 1.f;
	
	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestCharacter::Grounded_Update()
{
	if (Projectile->GetProjectileState() == ProjectileState::HOOKED)
		SetCharacterState(CharacterState::SWINGING);
}

void AGrapplingHookTestCharacter::Swinging_Enter()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->GravityScale = 0.f;

	FVector  ropeVector = Projectile->GetRopeVector();
	ropeVector.Normalize();
	float startAngle = -FMath::Acos(ropeVector | GetActorUpVector());
	FVector velocity = GetVelocity() * ;
	FVector forwardVec = GetActorForwardVector();
	float projectedVelToFVec = FVector::DotProduct(velocity, forwardVec);
	FVector bottomTriangle = forwardVec * projectedVelToFVec;
	FVector triangleOpposite = -ropeVector + bottomTriangle;
	float angleWithLength = FVector::DotProduct(ropeVector, triangleOpposite);
	float angleWithoutLength = angleWithLength / (ropeVector.Size() * triangleOpposite.Size());
	float angle = FMath::Acos(angleWithoutLength);
	//float startVelocity = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
	//startVelocity = FMath::Acos(FVector::DotProduct(ropeVector, GetActorForwardVector() * startVelocity)) / (ropeVector.Size() * (GetActorForwardVector() * startVelocity).Size());
	PendulumVar = Pendulum(Projectile->GetCollisionComp()->GetComponentLocation(), angleWithoutLength, startAngle, Projectile->GetRopeLength(), GetWorld()->GetGravityZ(), ropeVector.X, ropeVector.Y);
	
	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestCharacter::Swinging_Update(float deltaTime)
{
	GetCharacterMovement()->StopMovementImmediately();
	PendulumVar.update(deltaTime);
	
	SetActorLocation(PendulumVar.GetPosition() /*+ MuzzleLocation->GetComponentLocation()*/);
}

void AGrapplingHookTestCharacter::OnFire()
{
	// try and fire a projectile
	if (Projectile != nullptr)
	{
		Projectile->Fire();
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AGrapplingHookTestCharacter::OnRetract()
{
	// try and fire the projectile
	if (Projectile != nullptr)
	{
		Projectile->Retract();
	}
}

void AGrapplingHookTestCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGrapplingHookTestCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGrapplingHookTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGrapplingHookTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
