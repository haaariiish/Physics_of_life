#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class Particule : public Entity {
    float lifetime;
public:
    // Constructeur original — lifetime
    Particule(Vec2 pos, float lifetime);

    // Nouveau — densité explicite, lifetime infinie
    Particule(Vec2 pos, float rho, float lifetime);
    Particule(std::string name,Vec2 pos, float rho, float lifetime);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) const override;
    ~Particule() override;
};