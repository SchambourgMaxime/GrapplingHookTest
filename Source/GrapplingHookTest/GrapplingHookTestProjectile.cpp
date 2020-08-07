// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrapplingHookTestProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AGrapplingHookTestProjectile::AGrapplingHookTestProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->OnComponentHit.AddDynamic(this, &AGrapplingHookTestProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AGrapplingHookTestProjectile::Init(const USceneComponent* dockPosition)
{
	DockPosition = dockPosition;
}

void AGrapplingHookTestProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AGrapplingHookTestProjectile::Tick(float DeltaTime)
{
	Update();
}

void AGrapplingHookTestProjectile::Update()
{
	if (ProjectileStateVar == ProjectileState::DOCKED)
	{
		if (StateStepVar == StateStep::ON_ENTER) {
			Docked_Enter();
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			//Docked_Update();
		}
	}

	if (ProjectileStateVar == ProjectileState::LAUNCHING)
	{
		if (StateStepVar == StateStep::ON_ENTER) {
			Docked_Enter();
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			//Docked_Update();
		}
	}
}

void AGrapplingHookTestProjectile::SetProjectileState(ProjectileState newState)
{
	// Append any GameStates you add to this example to this switch statement...
	switch (ProjectileStateVar)
	{
	case ProjectileState::DOCKED:
		//Docked_Exit();
		break;
	case ProjectileState::LAUNCHING:
		//Launching_Exit();
		break;
	case ProjectileState::RETRACTING:
		//Retracting_Exit();
		break;
	case ProjectileState::HOOKED:
		//Hooked_Exit();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Unexpected state has not been implemented!"), newState);
		return;
	}

	// Set new GameStates state and begin OnEnter of that state
	ProjectileStateVar = newState;
	StateStepVar = StateStep::ON_ENTER;
}

void AGrapplingHookTestProjectile::Docked_Enter()
{
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->UpdatedComponent = nullptr;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;

	SetActorLocation(DockPosition->GetComponentLocation());
	
	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestProjectile::Launching_Enter()
{
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMovement->AddForce(GetActorForwardVector() * ProjectileSpeed);
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	
	StateStepVar = StateStep::ON_UPDATE;
}
