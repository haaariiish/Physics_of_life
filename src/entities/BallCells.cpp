#include "BallCells.hpp"
#include <cmath>
#include <iostream>

static const float PI = 2.0f * acos(0.0f);

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                          CONSTRUCTION                                       ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

BallCells::BallCells(
    std::vector<std::unique_ptr<Particule>> particules,
    std::vector<Spring>                     springs,
    float omega,
    float amplitude
)
    : Entity("BallCells", nullptr)
    , particules(std::move(particules))
    , omega(omega)
    , amplitude(amplitude)
{
    for (auto& s : springs) {
        if (s.restLength <= 0.f) {
            Vec2 a    = this->particules[s.indexA]->getBody()->getPosition();
            Vec2 b    = this->particules[s.indexB]->getBody()->getPosition();
            Vec2 diff = { b.x - a.x, b.y - a.y };
            s.restLength = sqrt(diff.x*diff.x + diff.y*diff.y);
        }
        if (wouldCross(s.indexA, s.indexB)) {
            std::cerr << "[BallCells] Spring (" << s.indexA << "," << s.indexB
                      << ") ignorée : croisement détecté.\n";
            continue;
        }
        this->springs.push_back(s);
    }
    center = computeCenter();
}

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
        springs.push_back({ i, j, 0.f, stiffness });
    }
    return BallCells(std::move(particules), std::move(springs), omega, amplitude);
}

void BallCells::addSpring(int indexA, int indexB, float stiffness, float restLength) {
    if (wouldCross(indexA, indexB)) {
        std::cerr << "[BallCells] Spring (" << indexA << "," << indexB
                  << ") refusée : croisement détecté.\n";
        return;
    }
    if (restLength <= 0.f) {
        Vec2  a    = particules[indexA]->getBody()->getPosition();
        Vec2  b    = particules[indexB]->getBody()->getPosition();
        Vec2  diff = { b.x - a.x, b.y - a.y };
        restLength = sqrt(diff.x*diff.x + diff.y*diff.y);
    }
    springs.push_back({ indexA, indexB, restLength, stiffness });
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                       GÉOMÉTRIE / UTILITAIRES                               ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

// Centre de masse pondéré par les masses (pas moyenne géométrique)
Vec2 BallCells::computeCenter() const {
    Vec2  c         = { 0.f, 0.f };
    float totalMass = 0.f;
    for (const auto& p : particules) {
        float m   = p->getBody()->getMass();
        Vec2  pos = p->getBody()->getPosition();
        c.x      += pos.x * m;
        c.y      += pos.y * m;
        totalMass += m;
    }
    if (totalMass > 1e-8f) {
        c.x /= totalMass;
        c.y /= totalMass;
    }
    return c;
}

// Produit vectoriel 2D signé : (a-p) × (b-p)
static float cross2D(Vec2 p, Vec2 a, Vec2 b) {
    return (a.x - p.x) * (b.y - p.y)
         - (a.y - p.y) * (b.x - p.x);
}

// Intersection stricte de deux segments (exclut les extrémités communes)
static bool segmentsIntersect(Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
    float d1 = cross2D(c, d, a);
    float d2 = cross2D(c, d, b);
    float d3 = cross2D(a, b, c);
    float d4 = cross2D(a, b, d);
    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
        return true;
    return false;
}

// Paramètres t, u de l'intersection des droites portant AB et CD
static bool intersectionParams(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float& t, float& u) {
    float denom = (b.x - a.x) * (d.y - c.y)
                - (b.y - a.y) * (d.x - c.x);
    if (fabsf(denom) < 1e-10f) return false;
    t = ((c.x - a.x) * (d.y - c.y) - (c.y - a.y) * (d.x - c.x)) / denom;
    u = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x)) / denom;
    return true;
}

bool BallCells::wouldCross(int iA, int iB) const {
    Vec2 a = particules[iA]->getBody()->getPosition();
    Vec2 b = particules[iB]->getBody()->getPosition();
    for (const auto& s : springs) {
        if (s.indexA == iA || s.indexA == iB ||
            s.indexB == iA || s.indexB == iB) continue;
        Vec2 c = particules[s.indexA]->getBody()->getPosition();
        Vec2 d = particules[s.indexB]->getBody()->getPosition();
        if (segmentsIntersect(a, b, c, d)) return true;
    }
    return false;
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                          BILAN DES FORCES                                   ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

// ── 1. Ressorts (Hooke) — forces internes conservatives ───────────────────────
//
//   F_A = +k * (dist - L0) * dir_AB     Newton 3 : F_B = -F_A   ✓
//
void BallCells::applySprings() {
    for (const auto& s : springs) {
        Vec2  a    = particules[s.indexA]->getBody()->getPosition();
        Vec2  b    = particules[s.indexB]->getBody()->getPosition();
        Vec2  diff = { b.x - a.x, b.y - a.y };
        float dist = sqrt(diff.x*diff.x + diff.y*diff.y);
        if (dist < 1e-8f) continue;

        Vec2  dir   = { diff.x / dist, diff.y / dist };
        float force = s.stiffness * (dist - s.restLength);

        particules[s.indexA]->getBody()->applyForce(dir *  force);
        particules[s.indexB]->getBody()->applyForce(dir * -force);
    }
}

// ── 2. Anti-croisement — force de répulsion topologique ───────────────────────
//
//   Quand deux segments AB et CD se croisent en P(t,u) :
//   - AB reçoit  -F  distribuée en -(1-t)·F sur A  et  -t·F sur B
//   - CD reçoit  +F  distribuée en +(1-u)·F sur C  et  +u·F sur D
//   Somme = -F + F = 0   Newton 3 ✓
//   La magnitude est proportionnelle à la profondeur du croisement.
//
void BallCells::applyUncrossForces(float repulsion) {
    int ns = (int)springs.size();
    for (int i = 0; i < ns; i++) {
        for (int j = i + 1; j < ns; j++) {
            const Spring& si = springs[i];
            const Spring& sj = springs[j];

            // Ignore les springs qui partagent un vertex
            if (si.indexA == sj.indexA || si.indexA == sj.indexB ||
                si.indexB == sj.indexA || si.indexB == sj.indexB) continue;

            Vec2 a = particules[si.indexA]->getBody()->getPosition();
            Vec2 b = particules[si.indexB]->getBody()->getPosition();
            Vec2 c = particules[sj.indexA]->getBody()->getPosition();
            Vec2 d = particules[sj.indexB]->getBody()->getPosition();

            float t, u;
            if (!intersectionParams(a, b, c, d, t, u)) continue;
            if (t <= 0.f || t >= 1.f || u <= 0.f || u >= 1.f) continue;

            // Normale à AB, orientée vers CD
            Vec2  ab     = { b.x - a.x, b.y - a.y };
            float lenAB  = sqrt(ab.x*ab.x + ab.y*ab.y);
            if (lenAB < 1e-8f) continue;
            Vec2 normal = { -ab.y / lenAB, ab.x / lenAB };

            Vec2  cd_mid = { (c.x+d.x)*0.5f, (c.y+d.y)*0.5f };
            Vec2  ab_mid = { (a.x+b.x)*0.5f, (a.y+b.y)*0.5f };
            Vec2  sep    = { cd_mid.x - ab_mid.x, cd_mid.y - ab_mid.y };
            if (sep.x*normal.x + sep.y*normal.y < 0.f) {
                normal.x = -normal.x;
                normal.y = -normal.y;
            }

            // Profondeur ∈ [0,1] : max quand t=u=0.5 (croisement central)
            float depth = (0.5f - fabsf(t - 0.5f))
                        * (0.5f - fabsf(u - 0.5f)) * 4.f;
            float mag   = repulsion * depth;
            Vec2  F     = { normal.x * mag, normal.y * mag };

            particules[si.indexA]->getBody()->applyForce({ -(1.f-t)*F.x, -(1.f-t)*F.y });
            particules[si.indexB]->getBody()->applyForce({ -t      *F.x, -t      *F.y });
            particules[sj.indexA]->getBody()->applyForce({  (1.f-u)*F.x,  (1.f-u)*F.y });
            particules[sj.indexB]->getBody()->applyForce({  u      *F.x,  u      *F.y });
        }
    }
}

// ── 3. Horloge interne — force active + réaction Newton 3 ────────────────────
//
//   La particule active reçoit F_active.
//   La réaction -F_active est distribuée sur ses voisins directs (via springs),
//   pondérée par : masse du voisin × |cos(angle entre F et direction du ressort)|
//   → un ressort bien aligné ET une cellule lourde absorbent plus de réaction.
//   Somme totale = F_active - F_active * (Σ w_i / Σ w_i) = 0   Newton 3 ✓
//
void BallCells::applyClockForce(float dt) {
    int n = (int)particules.size();
    if (n == 0 || clockSteps.empty()) return;

    for (const auto& step : clockSteps) {
        if (theta < step.phaseStart || theta >= step.phaseEnd) continue;

        // Enveloppe sinusoïdale : force nulle aux bords de la fenêtre (0→1→0)
        float localPhase = (theta - step.phaseStart)
                         / (step.phaseEnd - step.phaseStart);
        float envelope   = sin(localPhase * PI);

        Vec2 F_active = {
            step.direction.x * amplitude * envelope,
            step.direction.y * amplitude * envelope
        };

        // Force active sur la particule motrice
        particules[step.activeIndex]->getBody()->applyForce(F_active);

        // Collecte des voisins connectés par un ressort
        /*struct Neighbour { int idx; float weight; };
        std::vector<Neighbour> neighbours;
        float totalWeight = 0.f;

        Vec2  posA  = particules[step.activeIndex]->getBody()->getPosition();
        float fLen  = sqrt(F_active.x*F_active.x + F_active.y*F_active.y);

        for (const auto& s : springs) {
            int otherIdx = -1;
            if      (s.indexA == step.activeIndex) otherIdx = s.indexB;
            else if (s.indexB == step.activeIndex) otherIdx = s.indexA;
            else continue;

            Vec2  posOther  = particules[otherIdx]->getBody()->getPosition();
            float massOther = particules[otherIdx]->getBody()->getMass();

            Vec2  diff = { posOther.x - posA.x, posOther.y - posA.y };
            float dist = sqrt(diff.x*diff.x + diff.y*diff.y);
            if (dist < 1e-8f) continue;

            Vec2  dir = { diff.x / dist, diff.y / dist };

            // Alignement entre la force et la direction du ressort
            float proj = (fLen > 1e-8f)
                ? fabsf(F_active.x*dir.x + F_active.y*dir.y) / fLen
                : 0.f;

            // Poids = alignement × masse (inertie du voisin)
            float weight = proj * massOther;

            neighbours.push_back({ otherIdx, weight });
            totalWeight += weight;
        }

        // Cas dégénéré : tous perpendiculaires → répartition par masse pure
        if (!neighbours.empty() && totalWeight < 1e-8f) {
            totalWeight = 0.f;
            for (auto& nb : neighbours) {
                nb.weight  = particules[nb.idx]->getBody()->getMass();
                totalWeight += nb.weight;
            }
        }

        // Répartition de la réaction sur les voisins
        if (totalWeight > 1e-8f) {
            for (const auto& nb : neighbours) {
                float share = nb.weight / totalWeight;
                particules[nb.idx]->getBody()->applyForce({
                    -F_active.x * share,
                    -F_active.y * share
                });
            }
        }
        */

        break; // une seule étape active par frame
    }

    // Avance la phase et comptabilise les cycles
    theta += omega * dt;
    if (theta >= 2.0f * PI) {
        theta -= 2.0f * PI;
        energyHistory.push_back({ totalTime, energyCycle });
        if (energyHistory.size() > 20) energyHistory.pop_front();
        std::cout << "Cycle " << ++cycleCount
                  << " | t=" << totalTime << "s"
                  << " | T=" << cycleTime << "s"
                  << " | E=" << energyCycle << "\n";
        energyCycle = 0.f;
        cycleTime   = 0.f;
    }
}

// ── 4. Force hydrodynamique (Resistive Force Theory, Gray-Hancock) ────────────
//
//   Pour chaque segment de spring, on décompose la vitesse moyenne
//   en composante tangentielle (v_t) et normale (v_n).
//   Le fluide résiste différemment selon la direction :
//       ξ_n ≈ 2 × ξ_t   (résistance normale double)
//   F_segment = -L * (ξ_t·v_t·t̂ + ξ_n·v_n·n̂)
//   Distribuée en F/2 sur chaque extrémité   Newton 3 ✓ (forces internes nulles)
//
//   C'est l'anisotropie ξ_n ≠ ξ_t qui crée une propulsion nette
//   quand la déformation est non-réciproque (théorème de Purcell).
//
//   DOIT être appelée en dernier, après toutes les autres forces,
//   car elle dépend des vitesses du pas courant.
//
void BallCells::applyHydrodynamicForces() {
    // Coefficients de Gray-Hancock pour un filament cylindrique fin
    const float xi_t = 2.f * PI * fluidDensity * segmentRadius;
    const float xi_n = 2.f * xi_t;  // ξ_n / ξ_t = 2

    for (const auto& s : springs) {
        Vec2 posA = particules[s.indexA]->getBody()->getPosition();
        Vec2 posB = particules[s.indexB]->getBody()->getPosition();
        Vec2 velA = particules[s.indexA]->getBody()->getVelocity();
        Vec2 velB = particules[s.indexB]->getBody()->getVelocity();

        Vec2  ab  = { posB.x - posA.x, posB.y - posA.y };
        float len = sqrt(ab.x*ab.x + ab.y*ab.y);
        if (len < 1e-8f) continue;

        Vec2 t_hat = { ab.x / len, ab.y / len };   // tangente
        Vec2 n_hat = { -t_hat.y,   t_hat.x   };    // normale

        // Vitesse moyenne du segment
        Vec2  velMid = { (velA.x + velB.x) * 0.5f,
                         (velA.y + velB.y) * 0.5f };

        // Projections scalaires
        float v_t = velMid.x*t_hat.x + velMid.y*t_hat.y;
        float v_n = velMid.x*n_hat.x + velMid.y*n_hat.y;

        // Force de résistance anisotrope (s'oppose au mouvement)
        Vec2 F = {
            -len * (xi_t * v_t * t_hat.x + xi_n * v_n * n_hat.x),
            -len * (xi_t * v_t * t_hat.y + xi_n * v_n * n_hat.y)
        };

        // F/2 sur chaque extrémité — Newton 3 ✓ (symétrie)
        particules[s.indexA]->getBody()->applyForce({ F.x * 0.5f, F.y * 0.5f });
        particules[s.indexB]->getBody()->applyForce({ F.x * 0.5f, F.y * 0.5f });
    }
}

// ── Mesure d'énergie cinétique par cycle ──────────────────────────────────────
void BallCells::measureEnergy(float dt) {
    for (auto& p : particules) {
        Vec2  vel   = p->getBody()->getVelocity();
        float speed = sqrt(vel.x*vel.x + vel.y*vel.y);
        energyCycle += 0.5f * p->getBody()->getMass() * speed * speed * dt;
        //             ^^^^ énergie cinétique réelle, pas juste la vitesse
    }
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                              UPDATE                                         ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

void BallCells::update(float dt) {
    totalTime += dt;
    cycleTime += dt;

    // ── Friction visqueuse de Stokes : γ = 6π·η·r  (par particule) ───────────
    // setGamma REMPLACE la valeur, ne l'accumule pas.
    for (auto& p : particules) {
        float r     = p->getBody()->getDimension();
        float gamma = 6.0f * 10000.f * PI * r;
        p->getBody()->setGamma(gamma);
    }

    // ── Ordre du bilan des forces ─────────────────────────────────────────────
    //
    //  1. Ressorts       : forces internes conservatives (Hooke)
    //  2. Anti-croisement: contrainte topologique (répulsion si croisement)
    //  3. Horloge        : force active + réaction Newton 3 sur voisins
    //  4. Hydrodynamique : résistance RFT — EN DERNIER car dépend des vitesses
    //
    //  5. Mesure énergie : avant intégration (vitesses du pas courant)
    //  6. Intégration    : Euler — met à jour positions et vitesses
    //  7. Centre de masse: recalculé après intégration
    //
    applySprings();
    applyUncrossForces(crossRepulsion);
    applyClockForce(dt);
    applyHydrodynamicForces();   // toujours en dernier

    measureEnergy(dt);

    for (auto& p : particules)
        p->update(dt);

    center = computeCenter();
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                           SÉQUENCEUR                                        ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

void BallCells::buildWaveSequencer(int steps) {
    clockSteps.clear();
    int   n     = (steps <= 0) ? (int)particules.size() : steps;
    float slice = (2.0f * PI) / n;

    for (int i = 0; i < n; i++) {
        int   idx   = i % (int)particules.size();
        float angle = (2.0f * PI * i) / n;
        Vec2  dir   = { -sin(angle), cos(angle) };  // tangente au cercle → onde rotative
        clockSteps.push_back({ idx, dir, i * slice, (i + 1) * slice });
    }
}

void BallCells::addClockStep(int activeIndex, Vec2 dir, float phaseStart, float phaseEnd) {
    float len = sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len > 1e-8f) dir = { dir.x/len, dir.y/len };
    clockSteps.push_back({ activeIndex, dir, phaseStart, phaseEnd });
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                              DESSIN                                         ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

void BallCells::draw(sf::RenderWindow& window) const {
    const float scale = 100.f;

    for (const auto& s : springs) {
        Vec2 a = particules[s.indexA]->getBody()->getPosition();
        Vec2 b = particules[s.indexB]->getBody()->getPosition();

        sf::Vertex line[2];
        line[0].position = { a.x * scale, a.y * scale };
        line[0].color    = sf::Color::Green;
        line[1].position = { b.x * scale, b.y * scale };
        line[1].color    = sf::Color::Green;
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    for (const auto& p : particules)
        p->draw(window);

    // Centre de masse (point rouge)
    sf::CircleShape dot(3.f);
    dot.setFillColor(sf::Color::Red);
    dot.setPosition({ center.x * scale - 3.f, center.y * scale - 3.f });
    window.draw(dot);
}

BallCells::~BallCells() {}