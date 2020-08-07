// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplingHookTestProjectile.generated.h"

UCLASS(config = Game)
class AGrapplingHookTestProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	float ProjectileSpeed = 3000.f;
	const USceneComponent* DockPosition;

public:
	AGrapplingHookTestProjectile();

	void Init(const USceneComponent* dockPosition);

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void Tick(float DeltaTime) override;

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

private:
	enum class ProjectileState { DOCKED, LAUNCHING, RETRACTING, HOOKED };
	ProjectileState ProjectileStateVar;

	enum StateStep { ON_ENTER, ON_UPDATE };
	StateStep StateStepVar;

	void Update();
	void SetProjectileState(ProjectileState newState);
	
	void Docked_Enter();
	//void Docked_Update();
	//void Docked_Exit();

	void Launching_Enter();
	void Launching_Update();
	void Launching_Exit();

	void Retracting_Enter();
	void Retracting_Update();
	void Retracting_Exit();

	void Hooked_Enter();
	void Hooked_Update();
	void Hooked_Exit();
};

