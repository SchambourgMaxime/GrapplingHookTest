// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class GRAPPLINGHOOKTEST_API Pendulum
{
	FVector position;    // position of pendulum ball
	FVector origin;      // position of arm origin
	float r;             // Length of arm
	float angle;         // Pendulum arm angle
	float aVelocity;     // Angle velocity
	float aAcceleration; // Angle acceleration

	float ballr;         // Ball radius
	float damping;       // Arbitary damping amount

	float gravity;

	float x, y;
	
public:
	// This constructor could be improved to allow a greater variety of pendulums
	Pendulum();
	Pendulum(FVector origin_, float velocity_, float angle_, float r_, float gravity_, float x_, float y_);
	~Pendulum();

	void update(float deltaTime);

	const FVector& GetPosition() { return position; }
};
