
#pragma once

#include "maths/Vec2.hpp"
#include "physics/Body.hpp"
#include <SFML/Graphics.hpp>
#include "string"
#include <iostream>

// Entity.hpp — classe de base
class Entity {
    protected:                       // accessible aux enfants, pas à l'extérieur
        Body* body;
        std::string name;
    
    public:
        Entity(std::string name, Body* body);
        Body* getBody() const;
        virtual void update(float dt) = 0;     // virtual = peut être redéfini
        virtual void draw(sf::RenderWindow& window) const = 0;         // virtual = peut être redéfini
        virtual ~Entity() ;       // destructeur virtual OBLIGATOIRE
        friend std::ostream& operator<<(std::ostream& os, const Entity& p) {
            os << "(Position x and y : " << p.getBody()->getPosition()<< ") - (Velocity x and y : "<< p.getBody()->getVelocity()<< ") - name : "<< p.name;
            return os; // Return the stream to allow chaining
        }
    };


