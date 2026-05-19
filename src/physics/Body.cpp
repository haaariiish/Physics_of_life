#include "Body.hpp"
#include "maths/Vec2.hpp"
#include <cmath>

Body::Body(Vec2 pos, float rho)
    : position(pos), velocity{0.f, 0.f}, rho(rho), dimension(0.3f),
      mass(rho * (4.0f/3.0f) * 3.14159f * dimension*dimension*dimension)
      
{}

Vec2  Body::getPosition()  const { return position; }
Vec2  Body::getVelocity()  const { return velocity; }
float Body::getRho()       const { return rho; }
float Body::getMass()      const { return mass; }
float Body::getDimension() const { return dimension; }
float Body::getGamma() const { return gammaAccum; }

void Body::setGamma(float gamma) {
    gammaAccum = gamma;             
}

void Body::applyForce(Vec2 force) {
    forceAccum = forceAccum + force; 
}



void Body::update(float dt) {
    Vec2  F_ext = forceAccum;
    Vec2  v_lim = F_ext * (1.0f / gammaAccum);         
    float decay = exp(-gammaAccum / mass * dt);          
    velocity    = v_lim + (velocity - v_lim) * decay;   //  exact solutoin
    position    = position + velocity * dt;

    forceAccum  = {0.f, 0.f};  // reset
    gammaAccum  = 0.f;          // reset
}