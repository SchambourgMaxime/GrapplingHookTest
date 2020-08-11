// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrapplingHookTestProjectile.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AGrapplingHookTestProjectile::AGrapplingHookTestProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->OnComponentHit.AddDynamic(this, &AGrapplingHookTestProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	Rope = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rope"));
	Rope->SetupAttachment(CollisionComp);

	Rope->SetWorldScale3D(FVector(0.f, 0.f, 0.f));
	
	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	SetProjectileState(ProjectileState::DOCKED);
}

void AGrapplingHookTestProjectile::Init(USceneComponent* dockPosition)
{
	DockPosition = dockPosition;

	UStaticMesh* staticMesh = Rope->GetStaticMesh();
	if (staticMesh)
	{
		float meshDiameterX = staticMesh->GetBoundingBox().GetExtent().X;
		float meshDiameterY = staticMesh->GetBoundingBox().GetExtent().Y;
		Rope->SetWorldScale3D(FVector(RopeDiameter / meshDiameterX, RopeDiameter / meshDiameterY, 0.f));
	}
}

void AGrapplingHookTestProjectile::Fire()
{
	if(ProjectileStateVar == ProjectileState::DOCKED)
		SetProjectileState(ProjectileState::LAUNCHING);
}

void AGrapplingHookTestProjectile::Retract()
{
	if (ProjectileStateVar == ProjectileState::LAUNCHING || ProjectileStateVar == ProjectileState::HOOKED)
		SetProjectileState(ProjectileState::RETRACTING);
}

void AGrapplingHookTestProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ProjectileStateVar == ProjectileState::LAUNCHING)
		SetProjectileState(ProjectileState::HOOKED);
}

void AGrapplingHookTestProjectile::Tick(float DeltaTime)
{
	Update(DeltaTime);
}

void AGrapplingHookTestProjectile::Update(float DeltaTime)
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
			Launching_Enter();
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			Launching_Update();
		}
	}

	if (ProjectileStateVar == ProjectileState::RETRACTING)
	{
		if (StateStepVar == StateStep::ON_ENTER) {
			Retracting_Enter();
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			Retracting_Update(DeltaTime);
		}
	}

	if (ProjectileStateVar == ProjectileState::HOOKED)
	{
		if (StateStepVar == StateStep::ON_ENTER) {
			Hooked_Enter();
		}
		if (StateStepVar == StateStep::ON_UPDATE) {
			Hooked_Update();
		}
	}
}

void AGrapplingHookTestProjectile::UpdateRope()
{
	float newRopeHeight = (GetActorLocation() - DockPosition->GetComponentLocation()).Size();
	FVector previousScale = Rope->GetComponentScale();
	Rope->SetWorldScale3D(FVector(previousScale.X, previousScale.Y, newRopeHeight / Rope->GetStaticMesh()->GetBoundingBox().GetExtent().Z));
	FVector newLocation = DockPosition->GetComponentLocation();
	DrawDebugSphere(GetWorld(), CollisionComp->GetComponentLocation(), 10.f, 20, FColor::Blue);
	DrawDebugSphere(GetWorld(), DockPosition->GetComponentLocation(), 10.f, 20, FColor::Red);
	Rope->SetWorldLocation(newLocation);
	
	//Rope->SetWorldRotation();
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
	CollisionComp->SetEnableGravity(false);
	AttachToComponent(DockPosition, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;

	SetActorLocation(DockPosition->GetComponentLocation());

	Rope->SetVisibility(false);
	
	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestProjectile::Launching_Enter()
{
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->SetEnableGravity(true);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;

	ProjectileMovement->Velocity = GetActorRightVector() * ProjectileSpeed;

	Rope->SetVisibility(true);
	
	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestProjectile::Launching_Update()
{
	UpdateRope();
}

void AGrapplingHookTestProjectile::Retracting_Enter()
{
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComp->SetEnableGravity(false);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;
	
	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestProjectile::Retracting_Update(float DeltaTime)
{
	FVector newPosition = GetActorLocation();
	FVector direction = (DockPosition->GetComponentLocation() - GetActorLocation()).GetSafeNormal();

	newPosition += direction * (retractingSpeedinCMPerSec * DeltaTime);
	SetActorLocation(newPosition);
	
	float distanceToDockingSquared = FVector::DistSquared(DockPosition->GetComponentLocation(), GetActorLocation());
	
	if (distanceToDockingSquared <= (RetractingToDockingDistance * RetractingToDockingDistance))
	{
		SetProjectileState(ProjectileState::DOCKED);
		return;
	}
	
	UpdateRope();
}

void AGrapplingHookTestProjectile::Hooked_Enter()
{
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComp->SetEnableGravity(false);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;

	StateStepVar = StateStep::ON_UPDATE;
}

void AGrapplingHookTestProjectile::Hooked_Update()
{
	UpdateRope();
}
