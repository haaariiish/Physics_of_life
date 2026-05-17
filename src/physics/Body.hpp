#pragma once
#include "maths/Vec2.hpp"
#include <cmath>

class Body {
private:
    Vec2  position;
    Vec2  velocity;
    float rho;
    float dimension;
    float mass;
    Vec2  forceAccum  {0.f, 0.f};
    float gammaAccum  {0.f};        // ✅ gamma est un scalaire, pas un Vec2

public:
    Body(Vec2 position, float rho);

    Vec2  getPosition()  const;
    Vec2  getVelocity()  const;
    float getRho()       const;
    float getMass()      const;
    float getDimension() const;

    void setGamma(float gamma);      // ✅ séparé de applyForce
    void applyForce(Vec2 force);     // ✅ signature corrigée
    void update(float dt);
};