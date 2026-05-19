#pragma once
#include "Particule.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <deque>

// Un ressort = lien entre deux particules
struct Spring {
    int   indexA;       // indice dans le vector de particules
    int   indexB;
    float restLength;   // longueur au repos (0 = calculée automatiquement)
    float stiffness;    // rigidité k
};

// Mesure d'énergie par cycle
struct EnergyRecord {
    float time;
    float energy;
};

// Remplace l'ancienne section "Horloge interne"

// Une étape du séquenceur : quelle particule pousse, dans quelle direction
struct ClockStep {
    int   activeIndex;   // indice de la particule motrice
    Vec2  direction;     // direction normalisée de la force active
    float phaseStart;    // début de la fenêtre [0, 2π]
    float phaseEnd;      // fin de la fenêtre
};

class BallCells : public Entity {

    std::vector<std::unique_ptr<Particule>> particules;
    std::vector<Spring>                     springs;
    Vec2                                    center {0.f, 0.f};

    // Horloge interne
    float theta      = 0.f;    // phase [0, 2π]
    float omega      = 1.0f;   // vitesse angulaire rad/s
    float amplitude  = 1e-4f;  // amplitude de la force

    std::vector<ClockStep> clockSteps;
    // Mesure d'énergie
    float energyCycle = 0.f;
    float totalTime   = 0.f;
    float cycleTime   = 0.f;
    int   cycleCount  = 0;
    std::deque<EnergyRecord> energyHistory;

public:

float crossRepulsion = 1000000000.f;
float fluidDensity    = 1000.f;  // eau (kg/m³)
float segmentRadius   = 1e-6f;   // rayon de section du "filament" (m)
    // ── Constructeur manuel ──────────────────────────────────────────────────

    // Tu passes tes particules et tes ressorts directement
    BallCells(
        std::vector<std::unique_ptr<Particule>> particules,
        std::vector<Spring>                     springs,
        float omega     = 1.0f,
        float amplitude = 1e-4f
    );


      // Construit un séquenceur ondulatoire automatique (N étapes sur le cercle)
    void buildWaveSequencer(int steps = -1); // -1 = 1 étape par particule
      // Ou ajoute une étape manuellement
    void addClockStep(int activeIndex, Vec2 direction, float phaseStart, float phaseEnd);

    // Dans BallCells.hpp (dans la section public:)
const std::vector<std::unique_ptr<Particule>>& getParticules() const { return particules; }

    // ── Constructeur automatique (cercle) ────────────────────────────────────
    static BallCells makeCircle(
        Vec2  center,
        int   numCells,
        float radius,
        float stiffness = 0.5f,
        float omega     = 1.0f,
        float amplitude = 1e-4f
    );

    // ── Interface ────────────────────────────────────────────────────────────
    void update(float dt) override;
    void draw(sf::RenderWindow& window) const override;
    ~BallCells() override;

    // Ajoute un ressort après construction
    void addSpring(int indexA, int indexB, float stiffness, float restLength = 0.f);

    // Stats
    float getTotalTime()  const { return totalTime; }
    int   getCycleCount() const { return cycleCount; }
    const std::deque<EnergyRecord>& getEnergyHistory() const { return energyHistory; }

private:
    void applySprings();
    bool wouldCross(int indexA, int indexB) const;  // ← ajouter
    void applyClockForce(float dt);
    void measureEnergy(float dt);
    Vec2 computeCenter() const;
    void applyUncrossForces(float crossRepulsion);
    void applyHydrodynamicForces();
};