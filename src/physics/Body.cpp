// Body.cpp
#include "Body.hpp"
#include "maths/Vec2.hpp"

// Syntaxe : NomClasse::nomMethode
Body::Body(Vec2 pos, float mass)
    : position(pos), velocity{0,0}, mass(mass)  // ← liste d'initialisation
{}

Vec2  Body::getPosition() const { return position; }
Vec2  Body::getVelocity() const { return velocity; }
float Body::getMass()     const { return mass; }

void Body::applyForce(Vec2 force) {
    // F = ma  →  a = F/m
    Vec2 acceleration = force * (1.0f /mass) ;
    velocity = velocity + acceleration;
}

void Body::update(float dt) {
    
    position = position + velocity * dt;
}