#include "BallCells.hpp"
#include <cmath>
#include <iostream>

static const float PI = 2.0f * acos(0.0f);

// ── Constructeur manuel ──────────────────────────────────────────────────────
BallCells::BallCells(
    std::vector<std::unique_ptr<Particule>> particules,
    std::vector<Spring>                     springs,
    float omega,
    float amplitude
)
    : Entity("BallCells", nullptr)
    , particules(std::move(particules))
    , springs(std::move(springs))
    , omega(omega)
    , amplitude(amplitude)
{
    for (auto& s : this->springs) {
        if (s.restLength <= 0.f) {
            Vec2  a    = this->particules[s.indexA]->getBody()->getPosition();
            Vec2  b    = this->particules[s.indexB]->getBody()->getPosition();
            Vec2  diff = {b.x - a.x, b.y - a.y};
            s.restLength = sqrt(diff.x*diff.x + diff.y*diff.y);
        }
    }
    center = computeCenter();
}

// ── Constructeur cercle ──────────────────────────────────────────────────────
BallCells BallCells::makeCircle(
    Vec2 center, int numCells, float radius,
    float stiffness, float omega, float amplitude
) {
    std::vector<std::unique_ptr<Particule>> particules;
    std::vector<Spring>                     springs;

    for (int i = 0; i < numCells; i++) {
        float angle = (2.0f * PI * i) / numCells;
        Vec2  pos   = {
            center.x + radius * cos(angle),
            center.y + radius * sin(angle)
        };
        particules.push_back(std::make_unique<Particule>(pos, 985.f, -1.f));
    }

    for (int i = 0; i < numCells; i++) {
        int j = (i + 1) % numCells;
        springs.push_back({i, j, 0.f, stiffness});
    }

    return BallCells(std::move(particules), std::move(springs), omega, amplitude);
}

// ── Ajoute un ressort ────────────────────────────────────────────────────────
void BallCells::addSpring(int indexA, int indexB, float stiffness, float restLength) {
    if (restLength <= 0.f) {
        Vec2 a    = particules[indexA]->getBody()->getPosition();
        Vec2 b    = particules[indexB]->getBody()->getPosition();
        Vec2 diff = {b.x - a.x, b.y - a.y};
        restLength = sqrt(diff.x*diff.x + diff.y*diff.y);
    }
    springs.push_back({indexA, indexB, restLength, stiffness});
}

// ── Ressorts ─────────────────────────────────────────────────────────────────
void BallCells::applySprings() {
    for (const auto& s : springs) {
        Vec2  a    = particules[s.indexA]->getBody()->getPosition();
        Vec2  b    = particules[s.indexB]->getBody()->getPosition();
        Vec2  diff = {b.x - a.x, b.y - a.y};
        float dist = sqrt(diff.x*diff.x + diff.y*diff.y);
        if (dist < 1e-8f) continue;

        Vec2  dir   = {diff.x / dist, diff.y / dist};
        float force = s.stiffness * (dist - s.restLength);

        particules[s.indexA]->getBody()->applyForce(dir *  force);
        particules[s.indexB]->getBody()->applyForce(dir * -force);
    }
}

// ── Horloge → force ──────────────────────────────────────────────────────────
void BallCells::applyClockForce(float dt) {
    Vec2 c = computeCenter();

    for (auto& p : particules) {
        Vec2  pos    = p->getBody()->getPosition();
        Vec2  radial = {pos.x - c.x, pos.y - c.y};
        float dist   = sqrt(radial.x*radial.x + radial.y*radial.y);
        if (dist < 1e-8f) continue;

        radial       = {radial.x / dist, radial.y / dist};
        Vec2 tangent = {-radial.y, radial.x};

        Vec2 force = radial  * (amplitude * sin(theta))
                   + tangent * (amplitude * cos(theta));

        p->getBody()->applyForce(force);
    }

    theta += omega * dt;
    if (theta >= 2.0f * PI) {
        theta -= 2.0f * PI;

        energyHistory.push_back({totalTime, energyCycle});
        if (energyHistory.size() > 20) energyHistory.pop_front();

        std::cout << "Cycle " << ++cycleCount
                  << " | t=" << totalTime  << "s"
                  << " | T=" << cycleTime  << "s"
                  << " | E=" << energyCycle << "\n";

        energyCycle = 0.f;
        cycleTime   = 0.f;
    }
}

// ── Énergie ───────────────────────────────────────────────────────────────────
void BallCells::measureEnergy(float dt) {
    for (auto& p : particules) {
        Vec2  vel   = p->getBody()->getVelocity();
        float speed = sqrt(vel.x*vel.x + vel.y*vel.y);
        energyCycle += speed * dt;
    }
}

// ── Centre de masse ───────────────────────────────────────────────────────────
Vec2 BallCells::computeCenter() const {
    Vec2 c = {0.f, 0.f};
    for (const auto& p : particules) {
        Vec2 pos = p->getBody()->getPosition();
        c.x += pos.x;
        c.y += pos.y;
    }
    if (!particules.empty()) {
        c.x /= particules.size();
        c.y /= particules.size();
    }
    return c;
}

// ── Update ────────────────────────────────────────────────────────────────────
void BallCells::update(float dt) {
    totalTime += dt;
    cycleTime += dt;

    for (auto& p : particules) {
        float r     = p->getBody()->getDimension();
        float gamma = 6.0f * 10000.f * PI * r;
        p->getBody()->setGamma(gamma);
    }

    applySprings();
    applyClockForce(dt);
    measureEnergy(dt);

    for (auto& p : particules)
        p->update(dt);

    center = computeCenter();
}

// ── Draw ──────────────────────────────────────────────────────────────────────
void BallCells::draw(sf::RenderWindow& window) const {
    float scale = 100.f;

    for (const auto& s : springs) {
        Vec2 a = particules[s.indexA]->getBody()->getPosition();
        Vec2 b = particules[s.indexB]->getBody()->getPosition();

        sf::Vertex line[2];
        line[0].position = {a.x * scale, a.y * scale};
        line[0].color    = sf::Color::Green;
        line[1].position = {b.x * scale, b.y * scale};
        line[1].color    = sf::Color::Green;
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    for (const auto& p : particules)
        p->draw(window);

    sf::CircleShape dot(3.f);
    dot.setFillColor(sf::Color::Red);
    dot.setPosition({center.x * scale - 3.f, center.y * scale - 3.f});
    window.draw(dot);
}

BallCells::~BallCells() {}