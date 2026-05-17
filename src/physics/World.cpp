#include "World.hpp"

// Constructeur — rien à faire, vector s'initialise tout seul


void World::addEntity(std::unique_ptr<Entity> e) {
    entities.push_back(std::move(e));  // move obligatoire avec unique_ptr
}

void World::removeEntity(int id) {
    if (id >= 0 && id < entities.size())
        entities.erase(entities.begin() + id);
}

void World::update(float dt) {
    for (auto& entity : entities){
        entity->getBody()->applyForce(gravity * entity->getBody()->getMass());
        entity->update(dt);
    }
}

const std::vector<std::unique_ptr<Entity>>& World::getEntities() const {
    return entities;
}



World::~World() {}  // unique_ptr détruit tout automatiquement