#include "World.hpp"
#include "entities/BallCells.hpp"
#include "entities/Entity.hpp"
#include <iostream>
#include <cmath>



void World::removeEntity(int id) {
    if (id >= 0 && id < (int)entities.size())
        entities.erase(entities.begin() + id);
}

void World::update(float dt) {
    const float pi = 3.14159f;

    

    for (auto& entity : entities) {
        //  BallCells n'a pas de Body propre — on skip

        if (entity->getBody() == nullptr) {
            entity->update(dt);
            continue;
        }

        /*std::cout << "x,y " << entity->getBody()->getPosition()<< " | name : " << entity->getName()
                  << " | vx,vy=" << entity->getBody()->getVelocity()  << "\n";*/

        float r      = entity->getBody()->getDimension();
        float volume = (4.0f/3.0f) * pi * r*r*r;
        float m      = entity->getBody()->getMass();
        float gamma  = 6.0f * viscosity * pi * r;

        Vec2 gravityForce = gravity * m;
        Vec2 archimede    = gravity * (density * volume);
        Vec2 F_ext        = gravityForce - archimede;

        entity->getBody()->setGamma(gamma);
        entity->getBody()->applyForce(F_ext);
        entity->update(dt);
    }
}

const std::vector<Entity*>& World::getEntities() const {
    return entities;
}

void World::addEntity(std::unique_ptr<Entity> e) {
    entities.push_back(e.get());
    ownedEntities.push_back(std::move(e));
}

void World::addLiving(std::unique_ptr<BallCells> b) {
    ballcells.push_back(b.get());
    entities.push_back(b.get());       // pour le draw
    ownedBallcells.push_back(std::move(b));
}

//World::~World() {}