// Body.hpp
#pragma once
#include "maths/Vec2.hpp"

class Body {
// ── Private data ──────────────────────
private:
    Vec2  position;
    Vec2  velocity;
    float mass;

// ── Public Interfacee ──────────────────────────────
public:
    // Constructor
    Body(Vec2 position, float mass);

    // Getters  (const = ne modifie pas l'objet)
    Vec2  getPosition() const;
    Vec2  getVelocity() const;
    float getMass()     const;

    // behavior
    void applyForce(Vec2 force);
    void update(float dt);          // dt = delta time
};