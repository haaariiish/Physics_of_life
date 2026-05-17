#include "Particule.hpp"
#include "maths/Vec2.hpp"
#include <SFML/Graphics.hpp>

Particule::Particule(Vec2 pos, float lifetime)
    : Entity("Particule", new Body(pos, 0.001f)), lifetime(lifetime)
{}

void Particule::update(float dt) {   
    lifetime -= dt;
    body->update(dt);
}

void Particule::draw(sf::RenderWindow& window) const {
    sf::CircleShape circle(5.f);
    circle.setFillColor(sf::Color::Cyan);
    Vec2 pos = body->getPosition();
    circle.setPosition({pos.x, pos.y});
    window.draw(circle);
}

Particule::~Particule() {}            