#include "World.hpp"
#include <iostream>
#include <cmath>

void World::addEntity(std::unique_ptr<Entity> e) {
    entities.push_back(std::move(e));
}

void World::removeEntity(int id) {
    if (id >= 0 && id < (int)entities.size())
        entities.erase(entities.begin() + id);
}

void World::update(float dt) {
    const float pi = 3.14159f;

    for (auto& entity : entities) {
        float r      = entity->getBody()->getDimension();
        float volume = (4.0f/3.0f) * pi * r*r*r;
        float m      = entity->getBody()->getMass();

        float gamma  = 6.0f * viscosity * pi * r;  // Stockes

        Vec2 gravityForce = gravity * m;
        Vec2 archimede    = gravity * (density * volume);  // Archimede
        Vec2 F_ext        = gravityForce - archimede;      // without visquous

        std::cout << "Gravity   : " << gravityForce << "\n";
        std::cout << "Archimede : " << archimede    << "\n";
        std::cout << "F_net     : " << F_ext        << "\n";
        std::cout << "v_lim     : " << F_ext * (1.f/gamma) << "\n";

        entity->getBody()->setGamma(gamma);     
        entity->getBody()->applyForce(F_ext);   
        entity->update(dt);

        std::cout << *entity << "\n\n";
    }
}

const std::vector<std::unique_ptr<Entity>>& World::getEntities() const {
    return entities;
}

World::~World() {}