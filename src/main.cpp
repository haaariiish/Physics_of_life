#include <iostream>
#include "maths/Vec2.hpp"
#include "physics/World.hpp"
#include "entities/Particule.hpp"
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Physics Engine");

    window.setFramerateLimit(60);  //  limit up to 60fps

    sf::Clock clock;


    World world;
    world.addEntity(std::make_unique<Particule>(Vec2{300.f, 400.f}, 5.f));

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // 2. Mettre à jour la physique
        
        world.update(dt);
        // 3. Dessiner
        window.clear(sf::Color::Black);
        for (const auto& entity : world.getEntities())
            entity->draw(window);
        window.display();
    }

    return 0;
}