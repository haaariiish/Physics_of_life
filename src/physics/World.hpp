#pragma once
#include "entities/Entity.hpp"
#include "maths/Vec2.hpp"
#include <vector>
#include <memory>

class World {
    std::vector<std::unique_ptr<Entity>> entities;
    float viscosity = 10000.f;   
    float density   = 1100.f;   
    float timeStep  = 0.016f;
    Vec2  gravity   = {0.f, 9.8f};

public:
    void addEntity(std::unique_ptr<Entity> e);
    void removeEntity(int id);
    void update(float dt);
    const std::vector<std::unique_ptr<Entity>>& getEntities() const;
    ~World();
};