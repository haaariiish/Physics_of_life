#include "Entity.hpp"
#include "string"

Entity::Entity(std::string name, Body* body)
    :  name(name), body(body)  // ← liste d'initialisation
{}


Body* Entity::getBody() const{
    return body;
}

std::string Entity::getName() const{
    return name;
}

Entity::~Entity() {
    delete body;   
    body = nullptr;
}


