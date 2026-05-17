#pragma once

#include "entities/Entity.hpp"
#include "maths/Vec2.hpp"
#include <vector>

// World.hpp
class World {
    std::vector<std::unique_ptr<Entity>> entities;
    float viscosity = 0.001;
    float density = 1000; // Water
    float timeStep = 0.016f;       // ~60fps
    Vec2 gravity = Vec2(0.f, 0.3f);

public:
    void addEntity(std::unique_ptr<Entity> e);
    void removeEntity(int id);
    void update(float dt);

    // accès en lecture seule pour le renderer
    const std::vector<std::unique_ptr<Entity>>& getEntities() const;
    ~World();
};