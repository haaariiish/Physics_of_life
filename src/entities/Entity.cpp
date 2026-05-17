#include "Entity.hpp"
#include "string"

Entity::Entity(std::string name, Body* body)
    :  name(name), body(body)  // ← liste d'initialisation
{}


Body* Entity::getBody() const{
    return body;
}

Entity::~Entity() {
    delete body;   
    body = nullptr;
}


