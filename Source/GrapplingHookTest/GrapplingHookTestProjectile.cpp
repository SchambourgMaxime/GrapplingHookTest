// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "GrapplingHookTestProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AGrapplingHookTestProjectile::AGrapplingHookTestProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AGrapplingHookTestProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->Velocity = FVector::ZeroVector;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	//ProjectileMovement->InitialSpeed = 0.f;
	//ProjectileMovement->MaxSpeed = 0.f;

	// Die after 3 seconds by default
	InitialLifeSpan = -1.0f;
}

void AGrapplingHookTestProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (!IsStuck && (OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		if(OtherComp->IsSimulatingPhysics())
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		this->SetActorLocation(Hit.ImpactPoint);
		ProjectileMovement->StopMovementImmediately();
		this->AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);

		IsStuck = true;
	}
}

void AGrapplingHookTestProjectile::Fire(FVector direction)
{
	ProjectileMovement->Velocity = direction * 3000.f;
	ProjectileMovement->ProjectileGravityScale = 1.f;
}
