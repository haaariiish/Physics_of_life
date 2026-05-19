#include <iostream>
#include "maths/Vec2.hpp"
#include "physics/World.hpp"
#include "entities/BallCells.hpp"
#include "entities/Particule.hpp"
#include <SFML/Graphics.hpp>

// 🌟 CLASSE SPÉCIALE : Permet le double-partage sans modifier World ou BallCells
class SharedParticule : public Particule {
    int updateCalls = 0; 
public:
    using Particule::Particule;

    // 🛠️ RÈGLE LE PROBLÈME DU DOUBLE UPDATE
    void update(float dt) override {
        updateCalls++;
        // Le World l'appelle en premier (on ignore), puis BallCells l'appelle en deuxième.
        // On n'exécute l'intégration qu'au 2ème appel, une fois que TOUTES les forces 
        // (Gravité + Archimède + Ressorts + Horloge) ont été accumulées dans le Body.
        if (updateCalls == 2) {
            Particule::update(dt);
            updateCalls = 0; // Reset pour la frame suivante
        }
    }

    // 🛠️ RÈGLE LE PROBLÈME DE LA DOUBLE DESTRUCTION (Double Free)
    // On court-circuite le delete automatique des unique_ptr de World et BallCells.
    void operator delete(void*) {
        // On ne fait rien ici. La mémoire sera libérée proprement à la fin du main.
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Physics Engine");
    window.setFramerateLimit(600);
    sf::Clock clock;

    World world;

    // Conteneur maître pour gérer la vraie durée de vie de nos particules partagées
    std::vector<SharedParticule*> masterStorage;

    // Lambda utilitaire pour instancier nos particules spéciales
    auto createSharedParticle = [&](const std::string& name, Vec2 pos, float rho, float lifetime) {
        auto* p = new SharedParticule(name, pos, rho, lifetime);
        masterStorage.push_back(p);
        return p;
    };

    // Initialisation des 4 particules de la cellule
    SharedParticule* p1 = createSharedParticle("Particule 1", Vec2{5.0f, 0.5f}, 985.f, -1.f);
    SharedParticule* p2 = createSharedParticle("Particule 2", Vec2{2.0f, 3.0f}, 985.f, -1.f);
    SharedParticule* p3 = createSharedParticle("Particule 3", Vec2{2.0f, 0.5f}, 985.f, -1.f);
    SharedParticule* p4 = createSharedParticle("Particule 4", Vec2{5.0f, 3.0f}, 985.f, -1.f);

    // ── 1. On donne les particules à BallCells ───────────────────────────────
    std::vector<std::unique_ptr<Particule>> cells;
    cells.push_back(std::unique_ptr<Particule>(p1));
    cells.push_back(std::unique_ptr<Particule>(p2));
    cells.push_back(std::unique_ptr<Particule>(p3));
    //cells.push_back(std::unique_ptr<Particule>(p4));

    std::vector<Spring> springs = {
        {0, 1, 1.f, 100000.f}, 
        //{3, 1, 1.f, 100000.f}, 
        {0, 2, 1.f, 100000.f}
    };

    auto ball = std::make_unique<BallCells>(std::move(cells), std::move(springs), 1.f, 100000.f);
    ball->buildWaveSequencer();
    
    world.addLiving(std::move(ball));   // Le World prend possession de l'entité BallCells

    // ── 2. On donne AUSSI les mêmes particules individuellement au World ─────
    // Ainsi, World::update possède un pointeur direct dessus et leur applique la physique globale !
    world.addEntity(std::unique_ptr<Entity>(p1));
    world.addEntity(std::unique_ptr<Entity>(p2));
    world.addEntity(std::unique_ptr<Entity>(p3));
    world.addEntity(std::unique_ptr<Entity>(p4));

    // ── Particule seule ──────────────────────────────────────────────────────
    world.addEntity(std::make_unique<Particule>("Particule 5", Vec2{3.0f, 4.0f}, 985.f, 99.f));

    // ── Boucle SFML du jeu ───────────────────────────────────────────────────
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        };

        world.update(dt);

        window.clear(sf::Color::Black);
        for (const auto& entity : world.getEntities())
            entity->draw(window);
        window.display();
    }

    // ── NETTOYAGE MANUEL (Évite les fuites de mémoire à la fermeture) ────────
    for (auto* p : masterStorage) {
        p->~SharedParticule();      // Appel explicite du destructeur virtuel
        ::operator delete(p);       // Libération via l'opérateur global
    }

    return 0;
}