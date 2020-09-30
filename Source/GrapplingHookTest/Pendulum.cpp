// Fill out your copyright notice in the Description page of Project Settings.


#include "Pendulum.h"

Pendulum::Pendulum()
{
    origin = FVector::ZeroVector;
    position = FVector::ZeroVector;
    r = 0.f;
    angle = 0.f;

    aVelocity = 0.f;
    aAcceleration = 0.f;
    damping = 0.f;

    gravity = 0.f;

    x = y = 0.f;
}

// This constructor could be improved to allow a greater variety of pendulums
Pendulum::Pendulum(FVector origin_, float velocity_, float angle_, float r_, float gravity_, float _x, float _y) {
    // Fill all variables
    origin = origin_;
    position = FVector::ZeroVector;
    r = r_;
    angle = angle_;

    aVelocity = velocity_;
    aAcceleration = 0.f;
    damping = 0.995f;   // Arbitrary damping

    gravity = -0.05f;

    x = _x;
    y = _y;
}

Pendulum::~Pendulum()
{
}

// Function to update position
void Pendulum::update(float deltaTime) {
    aAcceleration = ((gravity) / r) * FMath::Sin(angle);  // Calculate acceleration (see: http://www.myphysicslab.com/pendulum1.html)
    aVelocity += aAcceleration;                 // Increment velocity
    //aVelocity *= deltaTime;
    //aVelocity *= damping;                       // Arbitrary damping
    angle += aVelocity;                         // Increment angle

    position = FVector((x * r * FMath::Sin(angle)) / FMath::Sqrt(FMath::Square(x) + FMath::Square(y)), 
    (y * r * FMath::Sin(angle)) / FMath::Sqrt(FMath::Square(x) + FMath::Square(y)),
    -r * FMath::Cos(angle)) + origin;         // Polar to cartesian conversion
}
