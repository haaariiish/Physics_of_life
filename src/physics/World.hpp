#pragma once
#include "entities/BallCells.hpp"
#include "entities/Entity.hpp"
#include "maths/Vec2.hpp"
#include <vector>
#include <memory>

class World {
    std::vector<std::unique_ptr<Entity>>     ownedEntities;  // propriétaire
    std::vector<Entity*>                     entities;       // vues brutes pour itérer
    std::vector<std::unique_ptr<BallCells>>  ownedBallcells;
    std::vector<BallCells*>                  ballcells;

    float viscosity = 10000.f;
    float density   = 1100.f;
    float timeStep  = 10.f;
    Vec2  gravity   = {0.f, 9.8f};

public:
    // World prend possession
    void addEntity(std::unique_ptr<Entity> e);
    void addLiving(std::unique_ptr<BallCells> b);

    void removeEntity(int id);
    void update(float dt);
    const std::vector<Entity*>& getEntities() const;
    ~World() = default;
};