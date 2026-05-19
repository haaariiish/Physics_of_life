#include "Particule.hpp"
#include "maths/Vec2.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

static const float PI = 2.0f * acos(0.0f);

// Constructeur original
Particule::Particule(Vec2 pos, float lifetime)
    : Entity("Particule", new Body(pos, 985.f)), lifetime(lifetime)
{}

//  Constructeur avec densité explicite
Particule::Particule(Vec2 pos, float rho, float lifetime)
    : Entity("Particule", new Body(pos, rho)), lifetime(lifetime)
{}
//  Constructeur avec densité explicite
Particule::Particule(std::string name ,Vec2 pos, float rho, float lifetime)
    : Entity(name, new Body(pos, rho)), lifetime(lifetime)
{}

void Particule::update(float dt) {
    if (lifetime > 0.f) lifetime -= dt;

    //  Calcule et applique gamma si pas déjà fait par World ou BallCells
    if (body->getGamma() <= 0.f) {
        float r     = body->getDimension();
        float gamma = 6.0f * 10000.f * PI * r;  // goudron par défaut
        body->setGamma(gamma);
    }

    body->update(dt);
}

void Particule::draw(sf::RenderWindow& window) const {
    float scale  = 100.f;
    float radius = body->getDimension() * scale;
    if (radius < 3.f) radius = 3.f;  // minimum visible

    sf::CircleShape circle(radius);
    circle.setFillColor(sf::Color::Cyan);

    //CORRECTION : On place l'origine au centre du cercle
    circle.setOrigin({radius, radius});

    Vec2 pos = body->getPosition();
    // Maintenant, setPosition placera le CENTRE du cercle à cet endroit
    circle.setPosition({pos.x * scale, pos.y * scale});
    window.draw(circle);
}

Particule::~Particule() {}