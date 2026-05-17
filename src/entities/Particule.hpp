#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

// Particle.hpp — special
class Particule : public Entity {
    float lifetime;
public:
    Particule(Vec2 pos, float lifetime);
    void update(float dt) override;    // override = redefine
    void draw(sf::RenderWindow& window)  const override ;
    ~Particule() override;  
    
};

