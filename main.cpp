#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <functional>
#include <random>
#include <cstdint>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <iomanip>

// Match History Record Structure
struct MatchRecord
{
    std::string heroName;
    int score;
    int crystals;
    std::string realm;
    std::string result;
};

// Realms / Locations Enum
enum class RealmLocation { ForestValley, MountainPeaks };

// Game State Enum
enum class GameState { LoginScreen, Playing };

// Gameplay Balance Tuning
struct Balance
{
    float dashIFrameTime = 0.12f;
    float dashCooldown = 1.2f;
    float bossMeleeDamage = 16.f;
    float bossSlamTelegraph = 0.7f;
    float bossMeleeWindup = 0.4f;
    float demonWindup = 0.3f;
    float enemyCollisionRadius = 14.f;
    float healDelayAfterDamage = 4.f;
    float healPerSecond = 3.f;
    float hitstopEnemyHit = 0.06f;
    float hitstopPlayerHit = 0.08f;
    float inputBufferTime = 0.15f;
    float chargerHealth = 3.f;
    float chargerDamage = 14.f;
    float chargerWalkSpeed = 80.f;
    float chargerChargeSpeed = 520.f;
    float chargerWindup = 0.6f;
    float chargerDuration = 0.5f;
    float chargerStun = 1.0f;
    float chargerChargeRange = 350.f;
    float bossSlamIntervalP1 = 3.4f;
    float bossSlamIntervalP2 = 2.6f;
    float bossSlamIntervalP3 = 2.0f;
    float bossPhase1Threshold = 0.6f;
    float bossPhase2Threshold = 0.3f;
    float bossPhaseInvulnTime = 1.5f;
    float bossPhaseTextTime = 1.2f;
    float bossPhase3SpeedMul = 1.2f;
    float bossSecondRingDelay = 0.5f;
    float bossSecondRingRadius = 360.f;
    sf::Color bossPhase3Tint{180, 20, 20, 40};

    // Scoring
    int scoreCrystal = 25;
    int scoreKillBasic = 20;
    int scoreKillDemon = 40;
    int scoreKillBoss = 150;
    float comboWindow = 3.f;
    float comboStep = 0.2f;
    float comboMaxMultiplier = 2.0f;
    int victoryBonus = 300;
    float noDamageMaxTaken = 0.f;
    float lowDamageMaxTaken = 50.f;
    float midDamageMaxTaken = 100.f;
    int noDamageBonusPerfect = 500;
    int noDamageBonusLow = 250;
    int noDamageBonusMid = 100;
    int perfectDodgePoints = 10;
    float perfectDodgeCooldown = 0.5f;
    float timeBonusTarget = 300.f;
    int timeBonusPerSecond = 2;
    float hintInterval = 4.f;    // seconds each first-run hint stays on screen
    float hintFadeTime = 0.6f;   // fade in/out duration for hints
    float hintsRunWindow = 30.f; // only show hints during the first 30 s of a run
};

inline constexpr Balance balance{};

// HUD / UI Styling (colors, sizes, trail speed)
struct UiStyle
{
    sf::Color hudPanelShadow{0, 0, 0, 130};
    sf::Color hudPanelBg{15, 25, 35, 215};
    sf::Color hudPanelBorder{212, 175, 90};
    sf::Color hudHpTop{90, 235, 140};
    sf::Color hudHpBottom{35, 170, 90};
    sf::Color hudHpMidTop{255, 210, 90};
    sf::Color hudHpMidBottom{205, 145, 35};
    sf::Color hudHpLowTop{255, 95, 95};
    sf::Color hudHpLowBottom{190, 40, 50};
    sf::Color hudTrail{255, 255, 255, 170};
    // Dash meter
    sf::Color hudDashTop{130, 246, 255};
    sf::Color hudDashBottom{45, 150, 210};
    sf::Color hudDashDim{130, 150, 168};
    sf::Color hudDashReadyGlow{140, 245, 255};
    float hudDashReadyFlashTime = 0.45f;
    // Score count-up animation (higher = faster "tick-up")
    float hudScoreCountUpSpeed = 14.f;
    float hudPanelCorner = 7.f;
    float hudPanelBorderThickness = 2.f;
    float hudPanelInset = 1.f;
    float hudPanelShadowOffset = 3.f;
    float hudHealthTrailTime = 0.5f;
};

inline constexpr UiStyle uiStyle{};

// Player-adjustable settings (persisted to settings.txt)
struct Settings
{
    int volume = 100;          // Master volume, 0-100 (scales every sound + BGM)
    int screenShakeMode = 0;   // 0 = On (1.0), 1 = Reduced (0.4), 2 = Off (0.0)
    int showHints = 1;         // 0 = off, 1 = on (first-run control hints)
};

inline float screenShakeScale(int mode)
{
    return (mode == 0) ? 1.0f : (mode == 1) ? 0.4f : 0.0f;
}

// Helper function to draw smooth radial light glow using SFML TriangleFan
void drawRadialGlow(sf::RenderWindow& window, sf::Vector2f center, float radius, sf::Color color, int points = 36)
{
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan, points + 2);
    fan[0].position = center;
    fan[0].color = color;

    for (int i = 0; i <= points; ++i)
    {
        float angle = i * 2.f * 3.1415926535f / points;
        fan[i + 1].position = center + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
        fan[i + 1].color = sf::Color(color.r, color.g, color.b, 0);
    }
    window.draw(fan);
}

// Elliptical radial glow (used by the Marvel-style teleport pod)
void drawRadialGlowEllipse(sf::RenderWindow& window, sf::Vector2f center, float rx, float ry, sf::Color color, int points = 40)
{
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan, points + 2);
    fan[0].position = center;
    fan[0].color = color;

    for (int i = 0; i <= points; ++i)
    {
        float angle = i * 2.f * 3.1415926535f / points;
        fan[i + 1].position = center + sf::Vector2f(std::cos(angle) * rx, std::sin(angle) * ry);
        fan[i + 1].color = sf::Color(color.r, color.g, color.b, 0);
    }
    window.draw(fan);
}

// ============================================================
// MARVEL-STYLE TELEPORT POD (Portal) SYSTEM
// Persistent geometry is built once and reused every frame so
// the effect stays cheap no matter how long the game runs.
// ============================================================
struct PortalSpark
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float size;
    float life;
    float maxLife;
};

struct TeleportPod
{
    sf::Vector2f pos{0.f, 0.f};
    sf::Color color{70, 220, 255};
    float time = 0.f;
    float open = 1.f;       // 0..1 opening scale animation
    float burst = 0.f;      // 0..1 teleport flash
    float sparkTimer = 0.f;
    float baseRX = 34.f;    // horizontal radius of the gateway
    float baseRY = 62.f;    // vertical radius of the gateway

    sf::VertexArray vortex;                 // rotating inner energy fan
    std::vector<sf::VertexArray> arcs;      // rotating ring energy segments
    std::vector<float> arcSpin;             // radians / second
    std::vector<float> arcSweep;            // arc length in radians
    std::vector<float> arcThick;            // band thickness in px
    std::vector<PortalSpark> sparks;        // emitted rim sparks
};

void buildTeleportPod(TeleportPod& pod)
{
    const int seg = 48;

    // Swirling energy disc: alternating bright/pale spokes create the spin
    pod.vortex = sf::VertexArray(sf::PrimitiveType::TriangleFan, seg + 2);
    pod.vortex[0].position = {0.f, 0.f};
    pod.vortex[0].color = sf::Color(255, 255, 255, 235);
    for (int i = 0; i <= seg; ++i)
    {
        float a = (float)i * 2.f * 3.1415926535f / (float)seg;
        pod.vortex[i + 1].position = {std::cos(a) * pod.baseRX, std::sin(a) * pod.baseRY};
        bool bright = (i % 2 == 0);
        pod.vortex[i + 1].color = bright
            ? sf::Color(pod.color.r, pod.color.g, pod.color.b, 165)
            : sf::Color(static_cast<std::uint8_t>(pod.color.r / 4), static_cast<std::uint8_t>(pod.color.g / 4), static_cast<std::uint8_t>(pod.color.b / 4), 70);
    }

    pod.arcs.clear();
    pod.arcSpin.clear();
    pod.arcSweep.clear();
    pod.arcThick.clear();

    const int arcSeg = 26;
    struct ArcDef { float spin; float sweep; float thick; };
    const ArcDef defs[3] = {
        {  1.30f, 1.05f, 6.f },
        { -1.85f, 0.80f, 9.f },
        {  0.85f, 1.55f, 4.f }
    };
    for (const auto& d : defs)
    {
        pod.arcs.push_back(sf::VertexArray(sf::PrimitiveType::TriangleStrip, (arcSeg + 1) * 2));
        pod.arcSpin.push_back(d.spin);
        pod.arcSweep.push_back(d.sweep);
        pod.arcThick.push_back(d.thick);
    }
}

void initTeleportPod(TeleportPod& pod, sf::Vector2f pos, sf::Color color, bool opening)
{
    pod.pos = pos;
    pod.color = color;
    pod.open = opening ? 0.f : 1.f;
    pod.burst = opening ? 1.f : 0.f;
    pod.sparkTimer = 0.f;
    pod.sparks.clear();
    buildTeleportPod(pod);
}

void updateTeleportPod(TeleportPod& pod, float dt, bool emitSparks)
{
    pod.time += dt;
    pod.open  = std::min(1.f, pod.open + dt * 2.4f);
    pod.burst = std::max(0.f, pod.burst - dt * 1.7f);

    const float breathe = 1.f + 0.03f * std::sin(pod.time * 5.f);
    const float breatheY = pod.open * (1.f + 0.03f * std::sin(pod.time * 5.f + 1.3f));

    // Rotating vortex disc (rotation/scale baked into the vertices)
    const int vcount = static_cast<int>(pod.vortex.getVertexCount());
    const int vSeg = vcount - 2;
    const float rot = pod.time * 2.6f;
    pod.vortex[0].position = pod.pos;
    for (int i = 1; i < vcount; ++i)
    {
        float a = (float)(i - 1) * 2.f * 3.1415926535f / (float)vSeg + rot;
        pod.vortex[i].position = pod.pos + sf::Vector2f(std::cos(a) * pod.baseRX * pod.open * breathe,
                                                        std::sin(a) * pod.baseRY * breatheY);
    }

    // Rotating ring energy bands
    for (std::size_t i = 0; i < pod.arcs.size(); ++i)
    {
        sf::VertexArray& arc = pod.arcs[i];
        const int segs = static_cast<int>(arc.getVertexCount()) / 2 - 1;

        const float sweep = pod.arcSweep[i] * (1.f + 0.12f * std::sin(pod.time * 3.f + (float)i));
        const float t = pod.arcThick[i];

        const float rxIn  = (pod.baseRX - t)           * pod.open;
        const float ryIn  = (pod.baseRY - t * 1.7f)     * pod.open;
        const float rxOut = (pod.baseRX + t)            * pod.open;
        const float ryOut = (pod.baseRY + t * 1.7f)     * pod.open;

        const sf::Color cIn (pod.color.r, pod.color.g, pod.color.b, 225);
        const sf::Color cOut(255, 255, 255, 40);

        for (int s = 0; s <= segs; ++s)
        {
            float a = pod.time * pod.arcSpin[i] + sweep * ((float)s / segs - 0.5f);
            float ca = std::cos(a), sa = std::sin(a);
            arc[s * 2].position     = pod.pos + sf::Vector2f(ca * rxIn,  sa * ryIn);
            arc[s * 2].color        = cIn;
            arc[s * 2 + 1].position = pod.pos + sf::Vector2f(ca * rxOut, sa * ryOut);
            arc[s * 2 + 1].color    = cOut;
        }
    }

    // Emit glowing sparks from the rim
    if (emitSparks && pod.open > 0.55f)
    {
        pod.sparkTimer += dt;
        const float interval = 1.f / 50.f;
        while (pod.sparkTimer >= interval && pod.sparks.size() < 70)
        {
            pod.sparkTimer -= interval;
            float a = (float)(rand() % 628) / 100.f;
            sf::Vector2f rim = pod.pos + sf::Vector2f(std::cos(a) * pod.baseRX * pod.open,
                                                      std::sin(a) * pod.baseRY * pod.open);
            sf::Vector2f outward(std::cos(a), std::sin(a));

            PortalSpark sp;
            sp.pos = rim;
            sp.vel = outward * (25.f + (float)(rand() % 60))
                   + sf::Vector2f((float)(rand() % 40 - 20), -30.f - (float)(rand() % 70));
            sp.color = (rand() % 3 == 0) ? sf::Color::White
                                         : sf::Color(pod.color.r, pod.color.g, pod.color.b);
            sp.size = 1.5f + (float)(rand() % 28) / 10.f;
            sp.life = 0.f;
            sp.maxLife = 0.45f + (float)(rand() % 55) / 100.f;
            pod.sparks.push_back(sp);
        }
    }

    // Integrate sparks
    for (int i = (int)pod.sparks.size() - 1; i >= 0; --i)
    {
        PortalSpark& sp = pod.sparks[i];
        sp.life += dt;
        sp.pos += sp.vel * dt;
        sp.vel.y += 110.f * dt;
        sp.vel *= (1.f - 1.6f * dt);
        if (sp.life >= sp.maxLife)
            pod.sparks.erase(pod.sparks.begin() + i);
    }
}

void drawTeleportPod(sf::RenderWindow& window, const TeleportPod& pod)
{
    if (pod.open <= 0.01f) return;

    sf::RenderStates add;
    add.blendMode = sf::BlendAdd;

    const float rx = pod.baseRX * pod.open;
    const float ry = pod.baseRY * pod.open;
    const sf::Color base = pod.color;

    // Layered atmospheric glow
    drawRadialGlowEllipse(window, pod.pos, rx * 2.6f, ry * 1.9f,
        sf::Color(base.r, base.g, base.b, static_cast<std::uint8_t>(60 * pod.open)));
    drawRadialGlowEllipse(window, pod.pos, rx * 1.5f, ry * 1.2f,
        sf::Color(255, 255, 255, static_cast<std::uint8_t>(28 * pod.open)));

    // Teleport burst shock ring
    if (pod.burst > 0.f)
    {
        float bt = pod.burst;
        float br = 16.f + (1.f - bt) * 150.f;
        sf::CircleShape flash(br, 48);
        flash.setOrigin({br, br});
        flash.setPosition(pod.pos);
        flash.setScale({0.75f, 1.35f});
        flash.setFillColor(sf::Color::Transparent);
        flash.setOutlineThickness(2.f + 7.f * bt);
        flash.setOutlineColor(sf::Color(base.r, base.g, base.b, static_cast<std::uint8_t>(210 * bt)));
        window.draw(flash, add);
    }

    // Rotating energy disc + ring bands
    window.draw(pod.vortex, add);
    for (const auto& arc : pod.arcs)
        window.draw(arc, add);

    // Hot core
    const float corePulse = 1.f + 0.15f * std::sin(pod.time * 9.f);
    sf::CircleShape core(16.f * corePulse, 28);
    core.setOrigin({16.f * corePulse, 16.f * corePulse});
    core.setPosition(pod.pos);
    core.setScale({0.7f, 1.25f});
    core.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(120 * pod.open)));
    window.draw(core, add);

    // Gateway rim: glow -> mid -> white-hot
    struct RimLayer { float thickness; float alpha; sf::Color col; };
    const RimLayer rim[3] = {
        { 14.f,  70.f, sf::Color(base.r, base.g, base.b) },
        {  7.f, 175.f, sf::Color(base.r, base.g, base.b) },
        {  3.f, 200.f, sf::Color(255, 255, 255) }
    };
    for (const auto& L : rim)
    {
        sf::CircleShape ring(40.f, 64);
        ring.setOrigin({40.f, 40.f});
        ring.setPosition(pod.pos);
        ring.setScale({rx / 40.f, ry / 40.f});
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(L.thickness);
        ring.setOutlineColor(sf::Color(L.col.r, L.col.g, L.col.b, static_cast<std::uint8_t>(L.alpha * pod.open)));
        window.draw(ring, add);
    }

    // Orbiting rune gem above the gateway
    const float bob = std::sin(pod.time * 3.f) * 4.f;
    sf::Vector2f gemPos = {pod.pos.x, pod.pos.y - ry - 24.f + bob};
    drawRadialGlowEllipse(window, gemPos, 40.f, 40.f, sf::Color(base.r, base.g, base.b, 75), 24);
    sf::CircleShape gem(9.f, 4);
    gem.setOrigin({9.f, 9.f});
    gem.setPosition(gemPos);
    gem.setRotation(sf::degrees(pod.time * 90.f));
    gem.setFillColor(sf::Color(base.r, base.g, base.b, 220));
    gem.setOutlineThickness(2.f);
    gem.setOutlineColor(sf::Color(255, 255, 255, 200));
    window.draw(gem, add);

    // Emitted rim sparks
    for (const auto& sp : pod.sparks)
    {
        float t = 1.f - sp.life / sp.maxLife;
        float r = sp.size * t + 0.5f;
        sf::CircleShape dot(r, 6);
        dot.setOrigin({r, r});
        dot.setPosition(sp.pos);
        dot.setFillColor(sf::Color(sp.color.r, sp.color.g, sp.color.b, static_cast<std::uint8_t>(255 * t)));
        window.draw(dot, add);
    }
}

// Particle Structure
struct Particle
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float size;
    float life;
    float maxLife;
};

// Player Ghost Trail (Dash Afterimage)
struct GhostTrail
{
    sf::Vector2f pos;
    sf::Vector2f facing;
    float alpha;
};

// Shockwave Ring Structure (Boss Slam)
struct Shockwave
{
    sf::Vector2f pos;
    float radius;
    float maxRadius;
    float life;
    float maxLife;
    float damage;
    bool hitPlayer;
};

// Floating Damage Number
struct FloatingNumber
{
    bool active = false;
    bool dodge = false; // renders "+N DODGE" instead of "-N"
    sf::Vector2f pos{0.f, 0.f};
    int amount = 0;
    sf::Color color{sf::Color::White};
    float life = 0.f;
    float maxLife = 0.8f;
};

// Enemy Types & Structure
enum class EnemyType { Crawler, Demon, Boss, Charger };

struct Enemy
{
    EnemyType type;
    RealmLocation realm;
    sf::Vector2f pos;
    float health;
    float maxHealth;
    float speed;
    float damage;
    float attackTimer;
    float hitTimer;
    bool isAlive;
    std::string name;
    float windupTimer = 0.f;
    bool isWindingUp = false;
    bool slamWarned = false;
    float stuckTimer = 0.f;
    float nudgeTimer = 0.f;
    sf::Vector2f nudgeDir{0.f, 0.f};
    float prevDistToPlayer = -1.f;
    int chargePhase = 0;                 // 0 walk, 1 wind-up, 2 charging, 3 stunned
    float chargeTimer = 0.f;
    sf::Vector2f chargeDir{1.f, 0.f};
    float speedScale = 1.f;
    int phase = 1;                       // Shadow Lord phase (1..3)
    float phaseInvulnTimer = 0.f;
    float phaseTextTimer = 0.f;
    bool pendingSecondRing = false;
    float secondRingTimer = 0.f;
};

// Crystal with realm location
struct Crystal
{
    sf::Vector2f pos;
    RealmLocation realm;
    bool collected;
    sf::Color color;
};

// Environment Structures
struct Tree
{
    sf::Vector2f pos;
    float scale;
};

struct Boulder
{
    sf::Vector2f pos;
    float radius;
};

struct Mushroom
{
    sf::Vector2f pos;
    sf::Color color;
};

// Mountain Peak Structure (পাহাড় ও পর্বত)
struct MountainPeak
{
    sf::Vector2f peak;
    float halfWidth;
    float height;
    sf::Color litColor;
    sf::Color shadowColor;
};

// Y-Sorted Renderable Structure
struct Renderable
{
    float y;
    std::function<void()> draw;
};

// Shared circular-vs-obstacle resolution used by both the player and enemies.
// Pushes the body out of any obstacle it overlaps, then clamps to the map.
sf::Vector2f resolveObstacleCollision(
    sf::Vector2f pos,
    float bodyRadius,
    RealmLocation realm,
    const std::vector<Boulder>& forestBoulders,
    const std::vector<Tree>& forestTrees,
    const std::vector<Boulder>& mountainCrags)
{
    if (realm == RealmLocation::ForestValley)
    {
        for (const auto& b : forestBoulders)
        {
            float dx = pos.x - b.pos.x;
            float dy = pos.y - b.pos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float minDist = b.radius + bodyRadius;
            if (dist < minDist && dist > 0.f)
                pos = b.pos + sf::Vector2f(dx / dist * minDist, dy / dist * minDist);
        }
        for (const auto& t : forestTrees)
        {
            float dx = pos.x - t.pos.x;
            float dy = pos.y - (t.pos.y + 20.f);
            float dist = std::sqrt(dx * dx + dy * dy);
            float minDist = 22.f * t.scale + (bodyRadius - 18.f);
            if (minDist < 2.f) minDist = 2.f;
            if (dist < minDist && dist > 0.f)
                pos = sf::Vector2f(t.pos.x, t.pos.y + 20.f) + sf::Vector2f(dx / dist * minDist, dy / dist * minDist);
        }
    }
    else
    {
        for (const auto& c : mountainCrags)
        {
            float dx = pos.x - c.pos.x;
            float dy = pos.y - c.pos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float minDist = c.radius + bodyRadius;
            if (dist < minDist && dist > 0.f)
                pos = c.pos + sf::Vector2f(dx / dist * minDist, dy / dist * minDist);
        }

        // Mountain top peak boundary (can't walk off the top of the mountains)
        if (pos.y < 210.f) pos.y = 210.f;
    }

    pos.x = std::clamp(pos.x, 30.f, 1070.f);
    pos.y = std::clamp(pos.y, 30.f, 720.f);
    return pos;
}

// Move an enemy toward the player with obstacle sliding. If the direct move
// is blocked, its X-only and Y-only components are tried so the enemy slides
// around obstacles instead of freezing. If no progress toward the player is
// made for ~1 s, a sideways nudge is applied for 0.5 s. No allocations.
void moveEnemyWithCollision(
    Enemy& enemy,
    sf::Vector2f playerPos,
    float dt,
    RealmLocation realm,
    float bodyRadius,
    const std::vector<Boulder>& forestBoulders,
    const std::vector<Tree>& forestTrees,
    const std::vector<Boulder>& mountainCrags)
{
    if (dt <= 0.f) return;

    float dx = playerPos.x - enemy.pos.x;
    float dy = playerPos.y - enemy.pos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist <= 1.f) return;

    sf::Vector2f dir(dx / dist, dy / dist);
    sf::Vector2f perp(-dir.y, dir.x);

    if (enemy.prevDistToPlayer < 0.f)
        enemy.prevDistToPlayer = dist;

    // Stuck detection: if we aren't getting closer for ~1 s, nudge sideways.
    if (enemy.nudgeTimer <= 0.f)
    {
        if (enemy.prevDistToPlayer - dist < 0.5f)
            enemy.stuckTimer += dt;
        else
            enemy.stuckTimer = 0.f;

        if (enemy.stuckTimer >= 1.0f)
        {
            enemy.nudgeTimer = 0.5f;
            enemy.nudgeDir = perp;
            enemy.stuckTimer = 0.f;
        }
    }
    enemy.prevDistToPlayer = dist;

    sf::Vector2f desired = dir * enemy.speed * enemy.speedScale;
    if (enemy.nudgeTimer > 0.f)
    {
        enemy.nudgeTimer = std::max(0.f, enemy.nudgeTimer - dt);
        desired = enemy.nudgeDir * enemy.speed * enemy.speedScale;
    }

    sf::Vector2f start = enemy.pos;
    sf::Vector2f step = desired * dt;

    sf::Vector2f full = resolveObstacleCollision(start + step, bodyRadius, realm,
                                                 forestBoulders, forestTrees, mountainCrags);
    sf::Vector2f best = full;
    float bestProgress = (full.x - start.x) * desired.x + (full.y - start.y) * desired.y;

    // If the full move was adjusted by a collision, try axis-only slides.
    float resX = full.x - (start.x + step.x);
    float resY = full.y - (start.y + step.y);
    if (resX * resX + resY * resY > 0.0001f)
    {
        sf::Vector2f xCand = resolveObstacleCollision(start + sf::Vector2f(step.x, 0.f), bodyRadius, realm,
                                                      forestBoulders, forestTrees, mountainCrags);
        sf::Vector2f yCand = resolveObstacleCollision(start + sf::Vector2f(0.f, step.y), bodyRadius, realm,
                                                      forestBoulders, forestTrees, mountainCrags);

        float xProg = (xCand.x - start.x) * desired.x + (xCand.y - start.y) * desired.y;
        float yProg = (yCand.x - start.x) * desired.x + (yCand.y - start.y) * desired.y;

        if (xProg > bestProgress) { bestProgress = xProg; best = xCand; }
        if (yProg > bestProgress) { bestProgress = yProg; best = yCand; }
    }

    enemy.pos = best;
}

// Returns true if the straight segment from a to b is not blocked by any
// realm obstacle. Used by the Charger to only commit to a charge with a
// clear line to the player. No allocations.
bool hasClearLine(
    sf::Vector2f a,
    sf::Vector2f b,
    RealmLocation realm,
    const std::vector<Boulder>& forestBoulders,
    const std::vector<Tree>& forestTrees,
    const std::vector<Boulder>& mountainCrags)
{
    sf::Vector2f ab = b - a;
    float len2 = ab.x * ab.x + ab.y * ab.y;
    if (len2 < 0.0001f) return true;

    auto blocked = [&](sf::Vector2f c, float r) {
        float t = ((c.x - a.x) * ab.x + (c.y - a.y) * ab.y) / len2;
        t = std::clamp(t, 0.f, 1.f);
        float px = a.x + ab.x * t;
        float py = a.y + ab.y * t;
        float dx = c.x - px;
        float dy = c.y - py;
        return (dx * dx + dy * dy) < r * r;
    };

    if (realm == RealmLocation::ForestValley)
    {
        for (const auto& bo : forestBoulders)
            if (blocked(bo.pos, bo.radius)) return false;
        for (const auto& t : forestTrees)
            if (blocked({t.pos.x, t.pos.y + 20.f}, 22.f * t.scale)) return false;
    }
    else
    {
        for (const auto& c : mountainCrags)
            if (blocked(c.pos, c.radius)) return false;
    }
    return true;
}

// Procedural Audio Synthesizer
void generateProceduralAudio(
    sf::SoundBuffer& swingBuf,
    sf::SoundBuffer& hitBuf,
    sf::SoundBuffer& crystalBuf,
    sf::SoundBuffer& dashBuf,
    sf::SoundBuffer& slamBuf,
    sf::SoundBuffer& teleportBuf,
    sf::SoundBuffer& bgmBuf,
    sf::SoundBuffer& warnBuf
) {
    const unsigned int sampleRate = 44100;

    // 1. Sword Swing (Whoosh: 420 Hz -> 140 Hz)
    {
        std::vector<std::int16_t> s(6500);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float freq = 420.f - t * 1800.f;
            float env = std::sin((float)i / s.size() * 3.14159f);
            float v = std::sin(2.f * 3.14159f * freq * t) * 0.7f + ((rand() % 100 - 50) / 100.f) * 0.3f;
            s[i] = static_cast<std::int16_t>(v * env * 16000.f);
        }
        (void)swingBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 2. Hit Crunch (Impact decay)
    {
        std::vector<std::int16_t> s(7500);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 24.f);
            float v = std::sin(2.f * 3.14159f * (160.f - t * 400.f) * t) * 0.65f + ((rand() % 100 - 50) / 100.f) * 0.35f;
            s[i] = static_cast<std::int16_t>(v * env * 22000.f);
        }
        (void)hitBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 3. Crystal Chime (Harmonic Crystalline Chime)
    {
        std::vector<std::int16_t> s(16000);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 8.f);
            float v = std::sin(2.f * 3.14159f * 1046.f * t) * 0.35f
                    + std::sin(2.f * 3.14159f * 1318.f * t) * 0.35f
                    + std::sin(2.f * 3.14159f * 2093.f * t) * 0.30f;
            s[i] = static_cast<std::int16_t>(v * env * 20000.f);
        }
        (void)crystalBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 4. Dash Swoosh
    {
        std::vector<std::int16_t> s(8800);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin((float)i / s.size() * 3.14159f);
            float v = std::sin(2.f * 3.14159f * (320.f - t * 900.f) * t) * 0.5f + ((rand() % 100 - 50) / 100.f) * 0.5f;
            s[i] = static_cast<std::int16_t>(v * env * 17000.f);
        }
        (void)dashBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 5. Boss Shockwave Slam (Heavy Bass Rumble)
    {
        std::vector<std::int16_t> s(20000);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 5.f);
            float v = std::sin(2.f * 3.14159f * (85.f - t * 50.f) * t) * 0.75f + ((rand() % 100 - 50) / 100.f) * 0.25f;
            s[i] = static_cast<std::int16_t>(v * env * 25000.f);
        }
        (void)slamBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 6. Teleport Warp Sound (Magical Swirl)
    {
        std::vector<std::int16_t> s(14000);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float freq = 220.f + t * 950.f;
            float env = std::sin((float)i / s.size() * 3.14159f);
            float v = std::sin(2.f * 3.14159f * freq * t + std::sin(t * 35.f) * 1.5f);
            s[i] = static_cast<std::int16_t>(v * env * 19000.f);
        }
        (void)teleportBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 7. Ambient BGM Drone
    {
        std::vector<std::int16_t> s(44100 * 3);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float v = std::sin(2.f * 3.14159f * 146.8f * t) * 0.35f
                    + std::sin(2.f * 3.14159f * 220.0f * t) * 0.30f
                    + std::sin(2.f * 3.14159f * 349.2f * t) * 0.20f
                    + std::sin(2.f * 3.14159f * 440.0f * t) * 0.15f;
            s[i] = static_cast<std::int16_t>(v * 12000.f);
        }
        (void)bgmBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 8. Boss Slam Warning Sound (Short telegraph ping / alarm)
    {
        std::vector<std::int16_t> s(7000);
        for (size_t i = 0; i < s.size(); ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin((float)i / s.size() * 3.14159f);
            float freq = 580.f + std::sin(t * 60.f) * 120.f;
            float v = std::sin(2.f * 3.14159f * freq * t);
            s[i] = static_cast<std::int16_t>(v * env * 22000.f);
        }
        (void)warnBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }
}

// Function to draw 3D Shaded Mountain Peaks (পাহাড়-পর্বতমালা)
void drawMountainRidge(sf::RenderWindow& window, sf::Vector2f peak, float halfWidth, float height, sf::Color litColor, sf::Color shadowColor)
{
    // Left Lit Mountain Slope
    sf::ConvexShape leftSlope(3);
    leftSlope.setPoint(0, peak);
    leftSlope.setPoint(1, {peak.x - halfWidth, peak.y + height});
    leftSlope.setPoint(2, {peak.x, peak.y + height});
    leftSlope.setFillColor(litColor);
    window.draw(leftSlope);

    // Right Shadow Mountain Slope
    sf::ConvexShape rightSlope(3);
    rightSlope.setPoint(0, peak);
    rightSlope.setPoint(1, {peak.x, peak.y + height});
    rightSlope.setPoint(2, {peak.x + halfWidth, peak.y + height});
    rightSlope.setFillColor(shadowColor);
    window.draw(rightSlope);

    // Mountain Ridge Spine Line
    sf::RectangleShape spine({2.5f, height});
    spine.setFillColor(sf::Color(255, 255, 255, 140));
    spine.setPosition({peak.x - 1.25f, peak.y});
    window.draw(spine);
}

// Function to Render Hall of Heroes / Match History Dashboard
void drawHistoryDashboard(
    sf::RenderWindow& window,
    const sf::Font& font,
    bool fontLoaded,
    const std::vector<MatchRecord>& matchHistory,
    int highScore,
    float totalTime
) {
    window.setView(window.getDefaultView());

    // Dimmed ambient overlay
    sf::RectangleShape dim({1100.f, 750.f});
    dim.setFillColor(sf::Color(8, 12, 20, 235));
    window.draw(dim);

    // Radial background celestial glow
    drawRadialGlow(window, {550.f, 375.f}, 480.f, sf::Color(0, 160, 240, 35));
    drawRadialGlow(window, {550.f, 375.f}, 250.f, sf::Color(80, 220, 255, 30));

    // Modal Glass Container
    sf::RectangleShape modal({840.f, 590.f});
    modal.setPosition({130.f, 75.f});
    modal.setFillColor(sf::Color(13, 20, 34, 248));
    modal.setOutlineThickness(3.f);
    modal.setOutlineColor(sf::Color(65, 185, 240, 215));
    window.draw(modal);

    // Top Highlight Bar
    sf::RectangleShape topBar({834.f, 4.f});
    topBar.setPosition({133.f, 78.f});
    topBar.setFillColor(sf::Color(100, 245, 255, 190));
    window.draw(topBar);

    if (!fontLoaded) return;

    // Header Title
    sf::Text title(font, "HALL OF HEROES  *  EXPEDITION ARCHIVE", 26);
    title.setFillColor(sf::Color(255, 222, 95));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
    title.setPosition({550.f, 112.f});
    window.draw(title);

    sf::Text subtitle(font, "CHRONICLES OF BATTLE HISTORY & REALM EXPEDITIONS", 11);
    subtitle.setFillColor(sf::Color(140, 230, 255));
    sf::FloatRect sb = subtitle.getLocalBounds();
    subtitle.setOrigin({sb.position.x + sb.size.x / 2.f, sb.position.y + sb.size.y / 2.f});
    subtitle.setPosition({550.f, 138.f});
    window.draw(subtitle);

    // Compute Summary Metrics
    int totalVictories = 0;
    int totalCrystals = 0;
    for (const auto& r : matchHistory) {
        if (r.result == "VICTORY") totalVictories++;
        totalCrystals += r.crystals;
    }

    // 4 Metric Stat Badges
    struct StatCard { std::string label; std::string value; sf::Color col; };
    std::vector<StatCard> stats = {
        {"TOTAL EXPEDITIONS", std::to_string(matchHistory.size()), sf::Color(100, 240, 255)},
        {"VICTORIES CLAIMED", std::to_string(totalVictories), sf::Color(90, 255, 170)},
        {"ALL-TIME RECORD", std::to_string(highScore) + " PTS", sf::Color(255, 220, 100)},
        {"CRYSTALS HARVESTED", std::to_string(totalCrystals), sf::Color(240, 140, 255)}
    };

    float cardW = 186.f;
    float cardH = 48.f;
    float startX = 152.f;
    float statY = 160.f;

    for (size_t i = 0; i < stats.size(); ++i) {
        float cx = startX + i * (cardW + 16.f);
        sf::RectangleShape sc({cardW, cardH});
        sc.setPosition({cx, statY});
        sc.setFillColor(sf::Color(9, 16, 26, 220));
        sc.setOutlineThickness(1.8f);
        sc.setOutlineColor(sf::Color(55, 140, 190, 170));
        window.draw(sc);

        sf::Text lbl(font, stats[i].label, 9);
        lbl.setFillColor(sf::Color(150, 185, 215));
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
        lbl.setPosition({cx + cardW / 2.f, statY + 14.f});
        window.draw(lbl);

        sf::Text val(font, stats[i].value, 15);
        val.setFillColor(stats[i].col);
        sf::FloatRect vb = val.getLocalBounds();
        val.setOrigin({vb.position.x + vb.size.x / 2.f, vb.position.y + vb.size.y / 2.f});
        val.setPosition({cx + cardW / 2.f, statY + 33.f});
        window.draw(val);
    }

    // Table Header Bar
    sf::RectangleShape thBar({790.f, 26.f});
    thBar.setPosition({155.f, 222.f});
    thBar.setFillColor(sf::Color(20, 32, 50, 230));
    thBar.setOutlineThickness(1.5f);
    thBar.setOutlineColor(sf::Color(70, 170, 220, 150));
    window.draw(thBar);

    struct ColumnHeader { std::string name; float x; };
    std::vector<ColumnHeader> cols = {
        {"#", 175.f},
        {"HERO NAME", 225.f},
        {"SCORE", 390.f},
        {"CRYSTALS", 510.f},
        {"EXPEDITION REALM", 630.f},
        {"STATUS", 860.f}
    };

    for (const auto& c : cols) {
        sf::Text ht(font, c.name, 11);
        ht.setFillColor(sf::Color(180, 225, 255));
        if (c.name == "STATUS") {
            sf::FloatRect cb = ht.getLocalBounds();
            ht.setOrigin({cb.position.x + cb.size.x / 2.f, 0.f});
            ht.setPosition({c.x, 227.f});
        } else {
            ht.setPosition({c.x, 227.f});
        }
        window.draw(ht);
    }

    // Display Rows (Most recent 7 runs)
    if (matchHistory.empty()) {
        sf::Text emptyMsg(font, "No expedition records found yet.\nEnter the realms and achieve glory to record your saga!", 16);
        emptyMsg.setFillColor(sf::Color(140, 175, 205, 180));
        sf::FloatRect eb = emptyMsg.getLocalBounds();
        emptyMsg.setOrigin({eb.position.x + eb.size.x / 2.f, eb.position.y + eb.size.y / 2.f});
        emptyMsg.setPosition({550.f, 380.f});
        window.draw(emptyMsg);
    } else {
        int maxRows = 7;
        int total = (int)matchHistory.size();
        int displayCount = std::min(maxRows, total);

        for (int i = 0; i < displayCount; ++i) {
            int matchIdx = total - 1 - i;
            const auto& rec = matchHistory[matchIdx];

            float rowY = 258.f + i * 44.f;

            // Row Container
            sf::RectangleShape rowBox({790.f, 38.f});
            rowBox.setPosition({155.f, rowY});
            rowBox.setFillColor((i % 2 == 0) ? sf::Color(14, 24, 38, 200) : sf::Color(10, 18, 30, 200));
            rowBox.setOutlineThickness(1.f);
            rowBox.setOutlineColor(sf::Color(50, 95, 135, 120));
            window.draw(rowBox);

            // Index
            sf::Text idxText(font, "#" + std::to_string(matchIdx + 1), 13);
            idxText.setFillColor(sf::Color(140, 170, 200));
            idxText.setPosition({175.f, rowY + 9.f});
            window.draw(idxText);

            // Hero Name
            sf::Text nameText(font, rec.heroName, 13);
            nameText.setFillColor(sf::Color(240, 250, 255));
            nameText.setPosition({225.f, rowY + 9.f});
            window.draw(nameText);

            // Score
            sf::Text scoreText(font, std::to_string(rec.score) + " pts", 13);
            scoreText.setFillColor(sf::Color(255, 215, 110));
            scoreText.setPosition({390.f, rowY + 9.f});
            window.draw(scoreText);

            // Crystals
            sf::Text crysText(font, std::to_string(rec.crystals) + " / 4", 13);
            crysText.setFillColor(sf::Color(100, 240, 255));
            crysText.setPosition({510.f, rowY + 9.f});
            window.draw(crysText);

            // Realm
            sf::Text realmText(font, rec.realm, 12);
            realmText.setFillColor(sf::Color(190, 180, 245));
            realmText.setPosition({630.f, rowY + 10.f});
            window.draw(realmText);

            // Outcome Badge
            bool isVic = (rec.result == "VICTORY");
            sf::RectangleShape badge({95.f, 24.f});
            badge.setOrigin({47.5f, 12.f});
            badge.setPosition({860.f, rowY + 19.f});
            badge.setFillColor(isVic ? sf::Color(20, 75, 45, 230) : sf::Color(75, 20, 30, 230));
            badge.setOutlineThickness(1.5f);
            badge.setOutlineColor(isVic ? sf::Color(80, 255, 160) : sf::Color(255, 80, 100));
            window.draw(badge);

            sf::Text outcomeText(font, isVic ? "VICTORY" : "FALLEN", 11);
            outcomeText.setFillColor(isVic ? sf::Color(120, 255, 190) : sf::Color(255, 120, 130));
            sf::FloatRect ob = outcomeText.getLocalBounds();
            outcomeText.setOrigin({ob.position.x + ob.size.x / 2.f, ob.position.y + ob.size.y / 2.f});
            outcomeText.setPosition({860.f, rowY + 19.f});
            window.draw(outcomeText);
        }
    }

    // Close Button at Bottom
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mPos = window.mapPixelToCoords(pixelPos);
    bool closeHover = (mPos.x >= 425.f && mPos.x <= 675.f && mPos.y >= 585.f && mPos.y <= 627.f);

    if (closeHover) {
        drawRadialGlow(window, {550.f, 606.f}, 110.f, sf::Color(60, 230, 210, 70));
    }

    sf::RectangleShape closeBtn({250.f, 42.f});
    closeBtn.setPosition({425.f, 585.f});
    closeBtn.setFillColor(closeHover ? sf::Color(26, 95, 125, 245) : sf::Color(16, 52, 75, 225));
    closeBtn.setOutlineThickness(closeHover ? 2.5f : 2.f);
    closeBtn.setOutlineColor(closeHover ? sf::Color(100, 255, 225) : sf::Color(65, 185, 225, 190));
    window.draw(closeBtn);

    sf::Text closeText(font, closeHover ? "[ BACK / CLOSE ]" : "BACK / CLOSE", 15);
    closeText.setFillColor(closeHover ? sf::Color::White : sf::Color(210, 245, 255));
    sf::FloatRect cb = closeText.getLocalBounds();
    closeText.setOrigin({cb.position.x + cb.size.x / 2.f, cb.position.y + cb.size.y / 2.f});
    closeText.setPosition({550.f, 606.f});
    window.draw(closeText);

    sf::Text prompt(font, "Press [ESC] or [H] to Close Dashboard", 11);
    prompt.setFillColor(sf::Color(140, 175, 200, 160));
    sf::FloatRect pb = prompt.getLocalBounds();
    prompt.setOrigin({pb.position.x + pb.size.x / 2.f, pb.position.y + pb.size.y / 2.f});
    prompt.setPosition({550.f, 642.f});
    window.draw(prompt);
}

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode({1100, 750}),
        "Shardfall: Realm of the Lost Crystals",
        sf::Style::Default,
        sf::State::Windowed,
        settings
    );

    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = font.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("C:/Windows/Fonts/segoeui.ttf");
    }

    // Audio Setup
    sf::SoundBuffer swingBuf, hitBuf, crystalBuf, dashBuf, slamBuf, teleportBuf, bgmBuf, warnBuf;
    generateProceduralAudio(swingBuf, hitBuf, crystalBuf, dashBuf, slamBuf, teleportBuf, bgmBuf, warnBuf);

    sf::Sound swingSound(swingBuf);
    sf::Sound hitSound(hitBuf);
    sf::Sound crystalSound(crystalBuf);
    sf::Sound dashSound(dashBuf);
    sf::Sound slamSound(slamBuf);
    sf::Sound teleportSound(teleportBuf);
    sf::Sound warnSound(warnBuf);
    warnSound.setVolume(55.f);
    sf::Sound bgmSound(bgmBuf);
    // Ambient background drone disabled by user request
    // bgmSound.setLooping(true);
    // bgmSound.setVolume(30.f);
    // bgmSound.play();

    // High Score System
    int highScore = 0;
    {
        std::ifstream fin("highscore.txt");
        if (fin >> highScore) {}
    }

    auto saveHighScore = [&](int currentScore) {
        if (currentScore > highScore) {
            highScore = currentScore;
            std::ofstream fout("highscore.txt");
            fout << highScore;
        }
    };

    // Match History System
    std::vector<MatchRecord> matchHistory;
    {
        std::ifstream fin("history.txt");
        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            MatchRecord rec;
            if (std::getline(ss, rec.heroName, '\t') &&
                (ss >> rec.score) &&
                (ss.ignore(), (ss >> rec.crystals)) &&
                (ss.ignore(), std::getline(ss, rec.realm, '\t')) &&
                std::getline(ss, rec.result))
            {
                if (!rec.result.empty() && rec.result.back() == '\r') rec.result.pop_back();
                matchHistory.push_back(rec);
            }
        }
    }

    bool showHistoryDashboard = false;
    bool showDebugOverlay = false;
    float smoothedFps = 60.f;
    bool matchRecorded = false;
    bool endIsNewHigh = false; // set once per run end when score beats the stored high score

    // Pause Menu
    bool paused = false;
    int pauseSelection = 0;

    // Settings: load persisted values (missing file -> defaults), then apply volume.
    Settings gameSettings;
    {
        std::ifstream fin("settings.txt");
        std::string line;
        while (std::getline(fin, line))
        {
            std::size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            int val = std::atoi(line.c_str() + eq + 1);
            if (key == "volume") gameSettings.volume = val;
            else if (key == "screenShake") gameSettings.screenShakeMode = val;
            else if (key == "showHints") gameSettings.showHints = val;
        }
        gameSettings.volume = std::clamp(gameSettings.volume, 0, 100);
        gameSettings.screenShakeMode = std::clamp(gameSettings.screenShakeMode, 0, 2);
        gameSettings.showHints = std::clamp(gameSettings.showHints, 0, 1);
    }

    auto saveSettings = [&]() {
        std::ofstream fout("settings.txt");
        if (fout)
            fout << "volume=" << gameSettings.volume << "\n"
                 << "screenShake=" << gameSettings.screenShakeMode << "\n"
                 << "showHints=" << gameSettings.showHints << "\n";
    };

    auto applyVolume = [&]() {
        float v = static_cast<float>(gameSettings.volume);
        swingSound.setVolume(v);
        hitSound.setVolume(v);
        crystalSound.setVolume(v);
        dashSound.setVolume(v);
        slamSound.setVolume(v);
        teleportSound.setVolume(v);
        warnSound.setVolume(0.55f * v);
        bgmSound.setVolume(v);
    };
    applyVolume();

    sf::Clock clock;
    float totalTime = 0.f;

    // Game State & User Profile
    GameState gameState = GameState::LoginScreen;
    std::string playerName = "Hero";

    // Location / Realm System (Forest Valley vs Craggy Mountain Peaks)
    RealmLocation currentRealm = RealmLocation::ForestValley;
    float portalCooldown = 0.f;

    int score = 0;
    float scoreDisplay = 0.f;   // HUD-animated score (counts up toward score)
    int collectedCount = 0;
    float totalDamageTaken = 0.f;

    // Combo multiplier (grows while chaining kills within balance.comboWindow)
    int comboCount = 0;
    float comboTimer = 0.f;
    float comboMultiplier = 1.f;

    // Perfect dodge throttling & run timer
    float perfectDodgeCooldown = 0.f;
    float runTime = 0.f;

    // Per-run score breakdown (shown on the end screen)
    int breakdownCrystals = 0;
    int breakdownKills = 0;
    int breakdownCombo = 0;
    int breakdownDodges = 0;
    int breakdownDodgePts = 0;
    int breakdownNoDamage = 0;
    int breakdownTime = 0;
    int breakdownVictory = 0;
    float endAnimTime = 0.f;

    // First-run control hints (persisted per hero name)
    const char* hintTexts[4] = {
        "Move: WASD / Arrow keys",
        "Attack: Space",
        "Dash: Shift / Ctrl (brief invulnerability)",
        "Pause: Esc",
    };
    bool hintDone[4] = {false, false, false, false};
    bool hintsActive = false;
    int hintIndex = 0;
    float hintTimer = 0.f;

    // Heroes that already saw their first-run hints.
    std::vector<std::string> hintsSeenNames;
    {
        std::ifstream fin("hints_seen.txt");
        std::string line;
        while (std::getline(fin, line))
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) hintsSeenNames.push_back(line);
        }
    }

    auto markHeroHintsSeen = [&](const std::string& name) {
        for (const auto& n : hintsSeenNames)
            if (n == name) return;
        hintsSeenNames.push_back(name);
        std::ofstream fout("hints_seen.txt", std::ios::app);
        if (fout) fout << name << "\n";
    };

    auto dismissHint = [&](int actionIdx) {
        if (actionIdx < 4 && !hintDone[actionIdx])
        {
            hintDone[actionIdx] = true;
            if (hintIndex == actionIdx)
            {
                hintIndex++;
                hintTimer = 0.f;
            }
        }
    };

    auto beginRunHints = [&]() {
        hintDone[0] = hintDone[1] = hintDone[2] = hintDone[3] = false;
        hintIndex = 0;
        hintTimer = 0.f;

        bool heroSeen = false;
        for (const auto& n : hintsSeenNames)
            if (n == playerName) { heroSeen = true; break; }

        hintsActive = (gameSettings.showHints == 1) && !heroSeen && runTime <= balance.hintsRunWindow;
        if (hintsActive) markHeroHintsSeen(playerName);
    };

    bool gameOver = false;
    bool victory = false;

    auto recordMatch = [&](const std::string& outcome) {
        if (matchRecorded) return;
        matchRecorded = true;
        endIsNewHigh = (score > highScore);
        saveHighScore(score);

        MatchRecord rec;
        rec.heroName = playerName.empty() ? "Hero" : playerName;
        rec.score = score;
        rec.crystals = collectedCount;
        rec.realm = (currentRealm == RealmLocation::ForestValley) ? "Forest Valley" : "Mountain Peaks";
        rec.result = outcome;

        matchHistory.push_back(rec);

        std::ofstream fout("history.txt", std::ios::app);
        if (fout) {
            fout << rec.heroName << "\t" << rec.score << "\t" << rec.crystals << "\t"
                 << rec.realm << "\t" << rec.result << "\n";
        }
    };

    // Player Stats & States
    float health = 100.f;
    float maxHealth = 100.f;
    float healthTrail = 100.f; // lagging HUD damage-trail value
    float damageTimer = 0.f;
    float attackTimer = 0.f;
    float attackCooldown = 0.f;
    float attackBufferTimer = 0.f;

    // Auto-Healing System (paused for a while after taking real damage)
    float healBlockTimer = 0.f;
    float healSparkTimer = 0.f;

    // Dash System
    float dashTimer = 0.f;
    float dashIFrameTimer = 0.f;
    float dashCooldown = 0.f;
    float dashBufferTimer = 0.f;
    float dashMaxCooldown = balance.dashCooldown;
    float dashReadyFlash = 0.f;  // glow timer when the dash meter refills
    bool dashWasReady = true;    // avoids a flash at spawn
    sf::Vector2f dashDir(1.f, 0.f);

    // Screen Shake System
    float shakeTimer = 0.f;
    float shakeIntensity = 0.f;

    // Hitstop System (brief gameplay freeze on impactful hits)
    float hitstopTimer = 0.f;
    constexpr float maxHitstop = 0.12f;

    sf::Vector2f facing(1.f, 0.f);
    sf::Vector2f playerPos(450.f, 400.f);

    // Crystals across both realms
    std::vector<Crystal> crystals = {
        {{350.f, 250.f}, RealmLocation::ForestValley, false, sf::Color(100, 240, 255)},
        {{650.f, 190.f}, RealmLocation::ForestValley, false, sf::Color(100, 240, 255)},
        {{760.f, 440.f}, RealmLocation::MountainPeaks, false, sf::Color(255, 120, 60)},
        {{320.f, 520.f}, RealmLocation::MountainPeaks, false, sf::Color(255, 60, 60)}
    };

    // Enemies across both realms
    std::vector<Enemy> enemies = {
        // Forest Valley Enemies
        {EnemyType::Crawler, RealmLocation::ForestValley, {180.f, 180.f}, 2.f, 2.f, 135.f, 7.f, 0.f, 0.f, true, "Crawler"},
        {EnemyType::Crawler, RealmLocation::ForestValley, {850.f, 560.f}, 2.f, 2.f, 135.f, 7.f, 0.f, 0.f, true, "Crawler"},
        {EnemyType::Demon,   RealmLocation::ForestValley, {220.f, 500.f}, 4.f, 4.f, 95.f,  12.f, 0.f, 0.f, true, "Shadow Demon"},

        // Mountain Peaks Enemies (with SHADOW LORD BOSS in his Mountain Fortress)
        {EnemyType::Boss,    RealmLocation::MountainPeaks, {550.f, 220.f}, 14.f, 14.f, 75.f, balance.bossMeleeDamage, 0.f, 0.f, true, "SHADOW LORD"},
        {EnemyType::Crawler, RealmLocation::MountainPeaks, {280.f, 440.f}, 2.f,  2.f,  140.f, 8.f, 0.f, 0.f, true, "Crag Crawler"},
        {EnemyType::Crawler, RealmLocation::MountainPeaks, {820.f, 440.f}, 2.f,  2.f,  140.f, 8.f, 0.f, 0.f, true, "Crag Crawler"},

        // Mountain Peaks Chargers (blitz along the crags)
        {EnemyType::Charger, RealmLocation::MountainPeaks, {430.f, 430.f}, balance.chargerHealth, balance.chargerHealth, balance.chargerWalkSpeed, balance.chargerDamage, 0.f, 0.f, true, "Charger"},
        {EnemyType::Charger, RealmLocation::MountainPeaks, {680.f, 430.f}, balance.chargerHealth, balance.chargerHealth, balance.chargerWalkSpeed, balance.chargerDamage, 0.f, 0.f, true, "Charger"}
    };

    // Environment: Forest Valley (গাছপালা ও শ্যাওলা পাথর)
    std::vector<Tree> forestTrees = {
        {{120.f, 130.f}, 1.1f}, {{280.f, 110.f}, 0.95f}, {{850.f, 120.f}, 1.2f},
        {{110.f, 580.f}, 1.15f}, {{320.f, 650.f}, 1.0f}, {{820.f, 620.f}, 1.25f},
        {{700.f, 120.f}, 1.05f}, {{460.f, 80.f},  0.9f},  {{80.f,  360.f}, 1.1f}
    };

    std::vector<Boulder> forestBoulders = {
        {{420.f, 220.f}, 22.f}, {{760.f, 380.f}, 28.f}, {{220.f, 440.f}, 25.f},
        {{640.f, 520.f}, 20.f}
    };

    // Environment: Mountain Peaks (বিশাল পাহাড় ও পর্বতমালা)
    std::vector<MountainPeak> backgroundMountains = {
        {{200.f, 40.f}, 180.f, 240.f, sf::Color(75, 65, 80), sf::Color(45, 38, 50)},
        {{550.f, 20.f}, 220.f, 280.f, sf::Color(85, 75, 95), sf::Color(50, 42, 60)},
        {{880.f, 35.f}, 190.f, 250.f, sf::Color(78, 68, 85), sf::Color(48, 40, 52)},
        {{380.f, 70.f}, 150.f, 200.f, sf::Color(95, 82, 105), sf::Color(58, 48, 68)},
        {{720.f, 65.f}, 160.f, 210.f, sf::Color(92, 80, 100), sf::Color(55, 45, 65)}
    };

    std::vector<Boulder> mountainCrags = {
        {{280.f, 380.f}, 35.f}, {{820.f, 380.f}, 38.f},
        {{380.f, 560.f}, 30.f}, {{720.f, 560.f}, 32.f},
        {{180.f, 500.f}, 28.f}, {{920.f, 500.f}, 28.f}
    };

    // Weather Particles
    std::vector<Particle> weatherMotes;
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> distX(20.f, 1080.f);
    std::uniform_real_distribution<float> distY(20.f, 730.f);
    for (int i = 0; i < 40; ++i) {
        weatherMotes.push_back({
            {distX(rng), distY(rng)},
            {(float((rand() % 100) - 50) / 100.f) * 15.f, (float((rand() % 100) - 50) / 100.f) * 15.f},
            sf::Color(255, 230, 120, 180),
            float(3 + rand() % 3),
            float(rand() % 100) / 20.f,
            5.f
        });
    }

    std::vector<Particle> fxParticles;
    std::vector<GhostTrail> ghostTrails;
    std::vector<Shockwave> shockwaves;

    // Fixed-size pool for floating damage numbers (zero per-frame allocations)
    static constexpr size_t MAX_FLOATING_NUMBERS = 64;
    std::array<FloatingNumber, MAX_FLOATING_NUMBERS> floatingNumbers{};

    auto spawnFloatingNumber = [&](sf::Vector2f pos, int amount, sf::Color color, bool isDodge = false) {
        for (auto& fn : floatingNumbers)
        {
            if (!fn.active)
            {
                fn.active = true;
                fn.dodge = isDodge;
                fn.pos = {pos.x, pos.y - 25.f};
                fn.amount = amount;
                fn.color = color;
                fn.life = 0.8f;
                fn.maxLife = 0.8f;
                break;
            }
        }
    };

    // Perfect dodge: +points near the player, throttled so one attack can't award repeatedly.
    auto awardPerfectDodge = [&]() {
        perfectDodgeCooldown = balance.perfectDodgeCooldown;
        score += balance.perfectDodgePoints;
        breakdownDodges++;
        breakdownDodgePts += balance.perfectDodgePoints;
        spawnFloatingNumber(playerPos, balance.perfectDodgePoints, sf::Color(110, 240, 255), true);
    };

    // Fixed-size circular buffer for damage taken tracking (last 10s)
    struct DamageHistoryRecord
    {
        float timestamp = -100.f;
        float amount = 0.f;
    };
    static constexpr size_t MAX_DAMAGE_HISTORY = 64;
    std::array<DamageHistoryRecord, MAX_DAMAGE_HISTORY> damageHistory{};
    size_t damageHistoryHead = 0;

    auto recordPlayerDamage = [&](float amount) {
        damageHistory[damageHistoryHead % MAX_DAMAGE_HISTORY] = { totalTime, amount };
        damageHistoryHead++;
    };

    // Marvel-style teleport pod (realm gateway)
    TeleportPod teleportPod;
    initTeleportPod(teleportPod,
        (currentRealm == RealmLocation::ForestValley) ? sf::Vector2f(1000.f, 360.f) : sf::Vector2f(100.f, 440.f),
        (currentRealm == RealmLocation::ForestValley) ? sf::Color(70, 220, 255) : sf::Color(255, 140, 45),
        false);

    auto addScreenShake = [&](float intensity, float duration = 0.22f) {
        shakeIntensity = intensity * screenShakeScale(gameSettings.screenShakeMode);
        shakeTimer = duration;
    };

    auto spawnSparks = [&](sf::Vector2f pos, sf::Color color, int count = 12) {
        for (int i = 0; i < count; ++i) {
            float angle = (float(rand() % 360) * 3.14159f) / 180.f;
            float speed = 80.f + float(rand() % 150);
            fxParticles.push_back({
                pos,
                {std::cos(angle) * speed, std::sin(angle) * speed},
                color,
                3.f + float(rand() % 4),
                0.f,
                0.3f + float(rand() % 30) / 100.f
            });
        }
    };

    auto performAttack = [&]() {
        attackTimer = 0.22f;
        attackCooldown = 0.35f;
        swingSound.play();
        spawnSparks(playerPos + facing * 35.f, sf::Color(180, 240, 255, 220), 8);
    };

    auto performDash = [&]() {
        dashTimer = 0.22f;
        dashIFrameTimer = balance.dashIFrameTime;
        dashCooldown = balance.dashCooldown;
        dashDir = facing;
        dashSound.play();
        spawnSparks(playerPos, sf::Color(100, 220, 255), 14);
        addScreenShake(3.5f, 0.15f);
    };

    // Records damage actually dealt to the player and breaks any active combo.
    auto registerPlayerDamage = [&](float amount) {
        totalDamageTaken += amount;
        comboCount = 0;
        comboTimer = 0.f;
        comboMultiplier = 1.f;
    };

    // Rounded rectangle without any per-frame heap allocation (rects + corner circles).
    auto drawRoundedRect = [&](sf::FloatRect r, float radius, sf::Color color) {
        const float x = r.position.x;
        const float y = r.position.y;
        const float w = r.size.x;
        const float h = r.size.y;
        const float rad = std::min(radius, std::min(w, h) * 0.5f);

        sf::RectangleShape mid({w - 2.f * rad, h});
        mid.setPosition({x + rad, y});
        mid.setFillColor(color);
        window.draw(mid);

        sf::RectangleShape side({rad, h - 2.f * rad});
        side.setFillColor(color);
        side.setPosition({x, y + rad});
        window.draw(side);
        side.setPosition({x + w - rad, y + rad});
        window.draw(side);

        sf::CircleShape corner(rad, 14);
        corner.setOrigin({rad, rad});
        corner.setFillColor(color);
        corner.setPosition({x + rad, y + rad});          window.draw(corner);
        corner.setPosition({x + w - rad, y + rad});       window.draw(corner);
        corner.setPosition({x + rad, y + h - rad});       window.draw(corner);
        corner.setPosition({x + w - rad, y + h - rad});   window.draw(corner);
    };

    // Full run reset used by the pause menu's "Quit to Login".
    auto quitToLogin = [&]() {
        health = 100.f;
        healthTrail = 100.f;
        damageTimer = 0.f;
        attackTimer = 0.f;
        attackCooldown = 0.f;
        attackBufferTimer = 0.f;
        dashTimer = 0.f;
        dashIFrameTimer = 0.f;
        dashCooldown = 0.f;
        dashBufferTimer = 0.f;
        healBlockTimer = 0.f;
        healSparkTimer = 0.f;
        portalCooldown = 0.f;
        hitstopTimer = 0.f;
        dashReadyFlash = 0.f;
        dashWasReady = true;

        score = 0;
        scoreDisplay = 0.f;
        collectedCount = 0;
        totalDamageTaken = 0.f;
        comboCount = 0;
        comboTimer = 0.f;
        comboMultiplier = 1.f;
        perfectDodgeCooldown = 0.f;
        runTime = 0.f;
        breakdownCrystals = 0;
        breakdownKills = 0;
        breakdownCombo = 0;
        breakdownDodges = 0;
        breakdownDodgePts = 0;
        breakdownNoDamage = 0;
        breakdownTime = 0;
        breakdownVictory = 0;
        endAnimTime = 0.f;
        endIsNewHigh = false;
        currentRealm = RealmLocation::ForestValley;

        gameOver = false;
        victory = false;
        matchRecorded = false;
        showHistoryDashboard = false;
        paused = false;

        facing = {1.f, 0.f};
        playerPos = {450.f, 400.f};

        for (auto& c : crystals) c.collected = false;
        for (auto& e : enemies) {
            e.isAlive = true;
            e.health = e.maxHealth;
            e.attackTimer = 0.f;
            e.hitTimer = 0.f;
            e.windupTimer = 0.f;
            e.isWindingUp = false;
            e.slamWarned = false;
            e.stuckTimer = 0.f;
            e.nudgeTimer = 0.f;
            e.nudgeDir = {0.f, 0.f};
            e.prevDistToPlayer = -1.f;
            e.chargePhase = 0;
            e.chargeTimer = 0.f;
            e.chargeDir = {1.f, 0.f};
            e.speedScale = 1.f;
            e.phase = 1;
            e.phaseInvulnTimer = 0.f;
            e.phaseTextTimer = 0.f;
            e.pendingSecondRing = false;
            e.secondRingTimer = 0.f;
        }

        shockwaves.clear();
        ghostTrails.clear();
        for (auto& fn : floatingNumbers) fn.active = false;
        for (auto& dh : damageHistory) { dh.timestamp = -100.f; dh.amount = 0.f; }
        damageHistoryHead = 0;
        gameState = GameState::LoginScreen;
        clock.restart();
    };

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                bgmSound.stop();
                window.close();
                std::exit(0);
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Escape)
                {
                    if (showHistoryDashboard)
                    {
                        showHistoryDashboard = false;
                    }
                    else if (paused)
                    {
                        paused = false; // Esc resumes
                    }
                    else if (gameState == GameState::Playing && !gameOver && !victory)
                    {
                        dismissHint(3); // performed the pause action
                        paused = true;
                        pauseSelection = 0;
                    }
                    else
                    {
                        bgmSound.stop();
                        window.close();
                        std::exit(0);
                    }
                }
                else if (key->code == sf::Keyboard::Key::P)
                {
                    if (!showHistoryDashboard && gameState == GameState::Playing && !gameOver && !victory)
                    {
                        dismissHint(3); // performed the pause action
                        paused = !paused;
                        if (paused) pauseSelection = 0;
                    }
                }
                else if (paused)
                {
                    if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
                    {
                        pauseSelection = (pauseSelection + 4) % 5;
                    }
                    else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
                    {
                        pauseSelection = (pauseSelection + 1) % 5;
                    }
                    else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A)
                    {
                        if (pauseSelection == 1)
                        {
                            gameSettings.volume = std::max(0, gameSettings.volume - 5);
                            applyVolume();
                            saveSettings();
                        }
                        else if (pauseSelection == 2)
                        {
                            gameSettings.screenShakeMode = (gameSettings.screenShakeMode + 2) % 3;
                            saveSettings();
                        }
                        else if (pauseSelection == 3)
                        {
                            gameSettings.showHints = (gameSettings.showHints == 1) ? 0 : 1;
                            saveSettings();
                        }
                    }
                    else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D)
                    {
                        if (pauseSelection == 1)
                        {
                            gameSettings.volume = std::min(100, gameSettings.volume + 5);
                            applyVolume();
                            saveSettings();
                        }
                        else if (pauseSelection == 2)
                        {
                            gameSettings.screenShakeMode = (gameSettings.screenShakeMode + 1) % 3;
                            saveSettings();
                        }
                        else if (pauseSelection == 3)
                        {
                            gameSettings.showHints = (gameSettings.showHints == 1) ? 0 : 1;
                            saveSettings();
                        }
                    }
                    else if (key->code == sf::Keyboard::Key::Enter)
                    {
                        if (pauseSelection == 0)
                        {
                            paused = false;
                        }
                        else if (pauseSelection == 4)
                        {
                            quitToLogin();
                        }
                    }
                }
                else if (key->code == sf::Keyboard::Key::F3)
                {
                    showDebugOverlay = !showDebugOverlay;
                }
                else if (key->code == sf::Keyboard::Key::H)
                {
                    if (showHistoryDashboard)
                    {
                        showHistoryDashboard = false;
                    }
                    else if (gameState == GameState::Playing || gameOver || victory)
                    {
                        showHistoryDashboard = true;
                    }
                }
                else if (showHistoryDashboard)
                {
                    // While dashboard is active, suppress other key interactions
                }
                else if ((gameOver || victory) &&
                         (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space))
                {
                    // Skip the end-screen reveal animation.
                    endAnimTime = 999.f;
                }
                else if (gameState == GameState::LoginScreen)
                {
                    if (key->code == sf::Keyboard::Key::Enter)
                    {
                        if (playerName.empty()) playerName = "Hero";
                        gameState = GameState::Playing;
                        crystalSound.play();
                        clock.restart();
                        beginRunHints();
                    }
                }
                else if (gameState == GameState::Playing)
                {
                    // Attack (input-buffered: remember the press if still on cooldown)
                    if (key->code == sf::Keyboard::Key::Space && !gameOver && !victory)
                    {
                        dismissHint(1); // performed the attack action
                        if (attackCooldown <= 0.f)
                            performAttack();
                        else
                            attackBufferTimer = balance.inputBufferTime;
                    }

                    // Dash (input-buffered: remember the press if still on cooldown)
                    if ((key->code == sf::Keyboard::Key::LShift || key->code == sf::Keyboard::Key::RShift || key->code == sf::Keyboard::Key::LControl) &&
                        !gameOver && !victory)
                    {
                        dismissHint(2); // performed the dash action
                        if (dashCooldown <= 0.f)
                            performDash();
                        else
                            dashBufferTimer = balance.inputBufferTime;
                    }

                    // Restart
                    if (key->code == sf::Keyboard::Key::R && (gameOver || victory))
                    {
                        health = 100.f;
                        healthTrail = 100.f;
                        damageTimer = 0.f;
                        attackTimer = 0.f;
                        attackCooldown = 0.f;
                        attackBufferTimer = 0.f;
                        dashTimer = 0.f;
                        dashIFrameTimer = 0.f;
                        dashCooldown = 0.f;
                        dashBufferTimer = 0.f;
                        healBlockTimer = 0.f;
                        healSparkTimer = 0.f;
                        portalCooldown = 0.f;
                        hitstopTimer = 0.f;
                        dashReadyFlash = 0.f;
                        dashWasReady = true;

                        score = 0;
                        scoreDisplay = 0.f;
                        collectedCount = 0;
                        totalDamageTaken = 0.f;
                        comboCount = 0;
                        comboTimer = 0.f;
                        comboMultiplier = 1.f;
                        perfectDodgeCooldown = 0.f;
                        runTime = 0.f;
                        breakdownCrystals = 0;
                        breakdownKills = 0;
                        breakdownCombo = 0;
                        breakdownDodges = 0;
                        breakdownDodgePts = 0;
                        breakdownNoDamage = 0;
                        breakdownTime = 0;
                        breakdownVictory = 0;
                        endAnimTime = 0.f;
                        endIsNewHigh = false;
                        currentRealm = RealmLocation::ForestValley;

                        gameOver = false;
                        victory = false;
                        matchRecorded = false;
                        showHistoryDashboard = false;

                        facing = {1.f, 0.f};
                        playerPos = {450.f, 400.f};

                        for (auto& c : crystals) c.collected = false;
                        for (auto& e : enemies) {
                            e.isAlive = true;
                            e.health = e.maxHealth;
                            e.attackTimer = 0.f;
                            e.hitTimer = 0.f;
                            e.windupTimer = 0.f;
                            e.isWindingUp = false;
                            e.slamWarned = false;
                            e.stuckTimer = 0.f;
                            e.nudgeTimer = 0.f;
                            e.nudgeDir = {0.f, 0.f};
                            e.prevDistToPlayer = -1.f;
                            e.chargePhase = 0;
                            e.chargeTimer = 0.f;
                            e.chargeDir = {1.f, 0.f};
                            e.speedScale = 1.f;
                            e.phase = 1;
                            e.phaseInvulnTimer = 0.f;
                            e.phaseTextTimer = 0.f;
                            e.pendingSecondRing = false;
                            e.secondRingTimer = 0.f;
                        }

                        shockwaves.clear();
                        ghostTrails.clear();
                        for (auto& fn : floatingNumbers) fn.active = false;
                        for (auto& dh : damageHistory) { dh.timestamp = -100.f; dh.amount = 0.f; }
                        damageHistoryHead = 0;
                        clock.restart();
                    }

                    // Return to Login Screen on 'L'
                    if (key->code == sf::Keyboard::Key::L && (gameOver || victory))
                    {
                        health = 100.f;
                        healthTrail = 100.f;
                        damageTimer = 0.f;
                        attackTimer = 0.f;
                        attackCooldown = 0.f;
                        attackBufferTimer = 0.f;
                        dashTimer = 0.f;
                        dashIFrameTimer = 0.f;
                        dashCooldown = 0.f;
                        dashBufferTimer = 0.f;
                        healBlockTimer = 0.f;
                        healSparkTimer = 0.f;
                        portalCooldown = 0.f;
                        hitstopTimer = 0.f;
                        dashReadyFlash = 0.f;
                        dashWasReady = true;

                        score = 0;
                        scoreDisplay = 0.f;
                        collectedCount = 0;
                        totalDamageTaken = 0.f;
                        comboCount = 0;
                        comboTimer = 0.f;
                        comboMultiplier = 1.f;
                        perfectDodgeCooldown = 0.f;
                        runTime = 0.f;
                        breakdownCrystals = 0;
                        breakdownKills = 0;
                        breakdownCombo = 0;
                        breakdownDodges = 0;
                        breakdownDodgePts = 0;
                        breakdownNoDamage = 0;
                        breakdownTime = 0;
                        breakdownVictory = 0;
                        endAnimTime = 0.f;
                        endIsNewHigh = false;
                        currentRealm = RealmLocation::ForestValley;

                        gameOver = false;
                        victory = false;
                        matchRecorded = false;
                        showHistoryDashboard = false;

                        facing = {1.f, 0.f};
                        playerPos = {450.f, 400.f};

                        for (auto& c : crystals) c.collected = false;
                        for (auto& e : enemies) {
                            e.isAlive = true;
                            e.health = e.maxHealth;
                            e.attackTimer = 0.f;
                            e.hitTimer = 0.f;
                            e.windupTimer = 0.f;
                            e.isWindingUp = false;
                            e.slamWarned = false;
                            e.stuckTimer = 0.f;
                            e.nudgeTimer = 0.f;
                            e.nudgeDir = {0.f, 0.f};
                            e.prevDistToPlayer = -1.f;
                            e.chargePhase = 0;
                            e.chargeTimer = 0.f;
                            e.chargeDir = {1.f, 0.f};
                            e.speedScale = 1.f;
                            e.phase = 1;
                            e.phaseInvulnTimer = 0.f;
                            e.phaseTextTimer = 0.f;
                            e.pendingSecondRing = false;
                            e.secondRingTimer = 0.f;
                        }

                        shockwaves.clear();
                        ghostTrails.clear();
                        for (auto& fn : floatingNumbers) fn.active = false;
                        for (auto& dh : damageHistory) { dh.timestamp = -100.f; dh.amount = 0.f; }
                        damageHistoryHead = 0;
                        gameState = GameState::LoginScreen;
                        clock.restart();
                    }
                }
            }

            // Text input for Login Screen
            if (gameState == GameState::LoginScreen && !showHistoryDashboard)
            {
                if (const auto* text = event->getIf<sf::Event::TextEntered>())
                {
                    char32_t unicode = text->unicode;
                    if (unicode == 8) // Backspace
                    {
                        if (!playerName.empty()) playerName.pop_back();
                    }
                    else if (unicode == 13 || unicode == 10) // Enter
                    {
                        if (playerName.empty()) playerName = "Hero";
                        gameState = GameState::Playing;
                        crystalSound.play();
                        clock.restart();
                        beginRunHints();
                    }
                    else if (unicode >= 32 && unicode < 127 && playerName.size() < 14)
                    {
                        playerName += static_cast<char>(unicode);
                    }
                }
            }

            // Mouse Clicks
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);

                    if (showHistoryDashboard)
                    {
                        // Check Dashboard Close Button: x [425..675], y [585..627]
                        if (mousePos.x >= 425.f && mousePos.x <= 675.f &&
                            mousePos.y >= 585.f && mousePos.y <= 627.f)
                        {
                            showHistoryDashboard = false;
                        }
                    }
                    else if (gameState == GameState::LoginScreen)
                    {
                        // Check Enter Realm Button: x [375..725], y [440..488]
                        if (mousePos.x >= 375.f && mousePos.x <= 725.f &&
                            mousePos.y >= 440.f && mousePos.y <= 488.f)
                        {
                            if (playerName.empty()) playerName = "Hero";
                            gameState = GameState::Playing;
                            crystalSound.play();
                            clock.restart();
                        }
                        // Check Hall of Heroes / Dashboard Button: x [375..725], y [498..542]
                        else if (mousePos.x >= 375.f && mousePos.x <= 725.f &&
                                 mousePos.y >= 498.f && mousePos.y <= 542.f)
                        {
                            showHistoryDashboard = true;
                        }
                    }
                }
            }
        }

        float dt = std::min(clock.restart().asSeconds(), 0.05f);
        totalTime += dt;
        if (dt > 0.0001f)
        {
            smoothedFps = smoothedFps * 0.92f + (1.f / dt) * 0.08f;
        }

        if (showHistoryDashboard && gameState == GameState::Playing)
        {
            dt = 0.f; // Pause game updates while dashboard modal is displayed
        }

        if (paused)
        {
            dt = 0.f; // Freeze world updates while the pause menu is open
        }

        // Hitstop: freeze gameplay updates (movement, timers, AI) while still rendering.
        // Consumes real time so the freeze lasts the intended wall-clock duration and
        // never accumulates past maxHitstop.
        if (hitstopTimer > 0.f && dt > 0.f)
        {
            hitstopTimer = std::max(0.f, hitstopTimer - dt);
            dt = 0.f;
        }

        // ===============================================
        // LOGIN SCREEN RENDERING
        // ===============================================
        if (gameState == GameState::LoginScreen)
        {
            // Ambient particles floating smoothly
            for (auto& ff : weatherMotes)
            {
                ff.pos += ff.vel * dt;
                ff.pos.x += std::sin(totalTime * 2.f + ff.pos.y) * 12.f * dt;
                ff.pos.y -= 25.f * dt;
                if (ff.pos.y < 0.f) ff.pos.y = 750.f;
                if (ff.pos.x < 0.f) ff.pos.x = 1100.f;
                if (ff.pos.x > 1100.f) ff.pos.x = 0.f;
            }

            window.setView(window.getDefaultView());
            window.clear(sf::Color(10, 14, 22));

            // Atmospheric Mountain Peak Silhouettes in background
            drawMountainRidge(window, {200.f, 440.f}, 260.f, 310.f, sf::Color(22, 26, 42, 160), sf::Color(14, 16, 28, 160));
            drawMountainRidge(window, {900.f, 420.f}, 280.f, 330.f, sf::Color(24, 28, 44, 160), sf::Color(15, 18, 30, 160));
            drawMountainRidge(window, {550.f, 490.f}, 350.f, 260.f, sf::Color(18, 22, 36, 200), sf::Color(11, 14, 24, 200));

            // Floating luminous motes
            for (const auto& ff : weatherMotes)
            {
                sf::CircleShape dot(ff.size * 0.9f, 8);
                dot.setFillColor(sf::Color(140, 230, 255, 140));
                dot.setPosition(ff.pos);
                window.draw(dot);
            }

            // Central Radial Cosmic / Rune Glow
            drawRadialGlow(window, {550.f, 370.f}, 380.f, sf::Color(0, 140, 220, 30));
            drawRadialGlow(window, {550.f, 370.f}, 220.f, sf::Color(70, 200, 255, 25));

            // Glassmorphic Card Frame
            sf::RectangleShape card({560.f, 530.f});
            card.setPosition({270.f, 105.f});
            card.setFillColor(sf::Color(13, 20, 32, 240));
            card.setOutlineThickness(3.f);
            card.setOutlineColor(sf::Color(64, 185, 235, 210));
            window.draw(card);

            // Card Inner Header Glow Bar
            sf::RectangleShape headerBar({554.f, 4.f});
            headerBar.setPosition({273.f, 108.f});
            headerBar.setFillColor(sf::Color(100, 240, 255, 180));
            window.draw(headerBar);

            // Title & Subtitle
            if (fontLoaded)
            {
                sf::Text titleShadow(font, "Shardfall", 36);
                titleShadow.setFillColor(sf::Color(180, 100, 20, 160));
                sf::FloatRect tb = titleShadow.getLocalBounds();
                titleShadow.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
                titleShadow.setPosition({552.f, 152.f});
                window.draw(titleShadow);

                sf::Text title(font, "Shardfall", 36);
                title.setFillColor(sf::Color(255, 222, 95));
                title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
                title.setPosition({550.f, 150.f});
                window.draw(title);

                sf::Text subtitle(font, "REALM OF THE LOST CRYSTALS", 13);
                subtitle.setFillColor(sf::Color(140, 230, 255));
                sf::FloatRect sb = subtitle.getLocalBounds();
                subtitle.setOrigin({sb.position.x + sb.size.x / 2.f, sb.position.y + sb.size.y / 2.f});
                subtitle.setPosition({550.f, 188.f});
                window.draw(subtitle);
            }

            // Decorative Divider Line & Center Gem
            sf::RectangleShape lineL({150.f, 2.f});
            lineL.setPosition({330.f, 208.f});
            lineL.setFillColor(sf::Color(70, 180, 230, 160));
            window.draw(lineL);

            sf::RectangleShape lineR({150.f, 2.f});
            lineR.setPosition({620.f, 208.f});
            lineR.setFillColor(sf::Color(70, 180, 230, 160));
            window.draw(lineR);

            sf::RectangleShape gem({12.f, 12.f});
            gem.setOrigin({6.f, 6.f});
            gem.setPosition({550.f, 209.f});
            gem.setRotation(sf::degrees(45.f));
            gem.setFillColor(sf::Color(100, 245, 255));
            window.draw(gem);

            // Hero Avatar Emblem (Knight with glowing visor)
            float bob = std::sin(totalTime * 4.f) * 2.5f;
            sf::CircleShape shieldBadge(28.f, 28);
            shieldBadge.setFillColor(sf::Color(18, 28, 44));
            shieldBadge.setOutlineThickness(2.5f);
            shieldBadge.setOutlineColor(sf::Color(255, 215, 100));
            shieldBadge.setOrigin({28.f, 28.f});
            shieldBadge.setPosition({550.f, 258.f});
            window.draw(shieldBadge);

            // Mini Helmet
            sf::CircleShape miniHelmet(12.f, 20);
            miniHelmet.setFillColor(sf::Color(65, 85, 105));
            miniHelmet.setOutlineThickness(2.f);
            miniHelmet.setOutlineColor(sf::Color(200, 235, 255));
            miniHelmet.setOrigin({12.f, 12.f});
            miniHelmet.setPosition({550.f, 256.f + bob});
            window.draw(miniHelmet);

            // Glowing Visor
            sf::RectangleShape miniVisor({15.f, 3.5f});
            miniVisor.setFillColor(sf::Color(100, 245, 255));
            miniVisor.setOrigin({7.5f, 1.75f});
            miniVisor.setPosition({550.f, 256.f + bob});
            window.draw(miniVisor);

            if (fontLoaded)
            {
                sf::Text avatarLabel(font, "KNIGHT HERO PROFILE", 10);
                avatarLabel.setFillColor(sf::Color(160, 200, 230));
                sf::FloatRect ab = avatarLabel.getLocalBounds();
                avatarLabel.setOrigin({ab.position.x + ab.size.x / 2.f, ab.position.y + ab.size.y / 2.f});
                avatarLabel.setPosition({550.f, 298.f});
                window.draw(avatarLabel);

                // Input Label
                sf::Text inputLabel(font, "ENTER HERO / USER NAME", 13);
                inputLabel.setFillColor(sf::Color(210, 235, 255));
                sf::FloatRect ilb = inputLabel.getLocalBounds();
                inputLabel.setOrigin({ilb.position.x + ilb.size.x / 2.f, ilb.position.y + ilb.size.y / 2.f});
                inputLabel.setPosition({550.f, 328.f});
                window.draw(inputLabel);
            }

            // Input Box
            sf::RectangleShape inputBox({380.f, 46.f});
            inputBox.setPosition({360.f, 348.f});
            inputBox.setFillColor(sf::Color(8, 14, 24, 240));
            inputBox.setOutlineThickness(2.5f);
            inputBox.setOutlineColor(sf::Color(80, 210, 255, 230));
            window.draw(inputBox);

            if (fontLoaded)
            {
                bool showBlink = (std::fmod(totalTime, 0.8f) < 0.45f);
                std::string displayText = playerName;
                if (showBlink) displayText += "|";

                if (playerName.empty() && !showBlink)
                {
                    sf::Text placeholder(font, "Type your name here...", 16);
                    placeholder.setFillColor(sf::Color(110, 140, 170, 160));
                    placeholder.setPosition({375.f, 360.f});
                    window.draw(placeholder);
                }
                else
                {
                    sf::Text nameText(font, displayText, 17);
                    nameText.setFillColor(sf::Color(245, 255, 255));
                    nameText.setPosition({375.f, 359.f});
                    window.draw(nameText);
                }

                // High Score Record Info
                sf::Text recordText(font, "REALM BEST RECORD: " + std::to_string(highScore) + " PTS", 12);
                recordText.setFillColor(sf::Color(255, 215, 105));
                sf::FloatRect rb = recordText.getLocalBounds();
                recordText.setOrigin({rb.position.x + rb.size.x / 2.f, rb.position.y + rb.size.y / 2.f});
                recordText.setPosition({550.f, 418.f});
                window.draw(recordText);
            }

            // Mouse hover check for Enter Realm Button
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mPos = window.mapPixelToCoords(pixelPos);
            bool btnHover = (!showHistoryDashboard && mPos.x >= 375.f && mPos.x <= 725.f && mPos.y >= 440.f && mPos.y <= 488.f);

            if (btnHover)
            {
                drawRadialGlow(window, {550.f, 464.f}, 130.f, sf::Color(60, 240, 210, 75));
            }

            // Enter Realm Button
            sf::RectangleShape enterBtn({350.f, 48.f});
            enterBtn.setPosition({375.f, 440.f});
            enterBtn.setFillColor(btnHover ? sf::Color(26, 95, 125, 245) : sf::Color(16, 52, 75, 225));
            enterBtn.setOutlineThickness(btnHover ? 3.f : 2.5f);
            enterBtn.setOutlineColor(btnHover ? sf::Color(100, 255, 225) : sf::Color(65, 185, 225, 200));
            window.draw(enterBtn);

            // Dashboard / Hall of Heroes Button
            bool dashBtnHover = (!showHistoryDashboard && mPos.x >= 375.f && mPos.x <= 725.f && mPos.y >= 498.f && mPos.y <= 542.f);
            if (dashBtnHover)
            {
                drawRadialGlow(window, {550.f, 520.f}, 120.f, sf::Color(255, 205, 80, 65));
            }

            sf::RectangleShape dashBtn({350.f, 44.f});
            dashBtn.setPosition({375.f, 498.f});
            dashBtn.setFillColor(dashBtnHover ? sf::Color(45, 38, 22, 245) : sf::Color(22, 28, 38, 225));
            dashBtn.setOutlineThickness(dashBtnHover ? 2.5f : 1.8f);
            dashBtn.setOutlineColor(dashBtnHover ? sf::Color(255, 220, 100) : sf::Color(200, 160, 60, 180));
            window.draw(dashBtn);

            if (fontLoaded)
            {
                sf::Text btnText(font, btnHover ? "[ ENTER REALM ]" : "ENTER REALM", 17);
                btnText.setFillColor(btnHover ? sf::Color::White : sf::Color(210, 245, 255));
                sf::FloatRect bb = btnText.getLocalBounds();
                btnText.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
                btnText.setPosition({550.f, 464.f});
                window.draw(btnText);

                sf::Text dashBtnText(font, dashBtnHover ? "[ HALL OF HEROES / ARCHIVE ]" : "HALL OF HEROES / ARCHIVE", 13);
                dashBtnText.setFillColor(dashBtnHover ? sf::Color(255, 235, 140) : sf::Color(230, 205, 120));
                sf::FloatRect db = dashBtnText.getLocalBounds();
                dashBtnText.setOrigin({db.position.x + db.size.x / 2.f, db.position.y + db.size.y / 2.f});
                dashBtnText.setPosition({550.f, 520.f});
                window.draw(dashBtnText);

                // Helper prompts
                sf::Text prompt1(font, "Press [ENTER] to Play   *   Click buttons to navigate", 11);
                prompt1.setFillColor(sf::Color(165, 205, 230, 180));
                sf::FloatRect p1b = prompt1.getLocalBounds();
                prompt1.setOrigin({p1b.position.x + p1b.size.x / 2.f, p1b.position.y + p1b.size.y / 2.f});
                prompt1.setPosition({550.f, 562.f});
                window.draw(prompt1);

                sf::Text prompt2(font, "Press [ESC] to Exit Game", 10);
                prompt2.setFillColor(sf::Color(135, 160, 185, 150));
                sf::FloatRect p2b = prompt2.getLocalBounds();
                prompt2.setOrigin({p2b.position.x + p2b.size.x / 2.f, p2b.position.y + p2b.size.y / 2.f});
                prompt2.setPosition({550.f, 582.f});
                window.draw(prompt2);
            }

            if (showHistoryDashboard)
            {
                drawHistoryDashboard(window, font, fontLoaded, matchHistory, highScore, totalTime);
            }

            window.display();
            continue;
        }

        attackCooldown = std::max(0.f, attackCooldown - dt);
        attackTimer = std::max(0.f, attackTimer - dt);
        dashCooldown = std::max(0.f, dashCooldown - dt);
        dashTimer = std::max(0.f, dashTimer - dt);
        dashIFrameTimer = std::max(0.f, dashIFrameTimer - dt);
        portalCooldown = std::max(0.f, portalCooldown - dt);
        perfectDodgeCooldown = std::max(0.f, perfectDodgeCooldown - dt);

        // Dash ready-flash: start a short glow the moment the meter refills.
        {
            bool dashNowReady = (dashCooldown <= 0.f);
            if (dashNowReady && !dashWasReady)
                dashReadyFlash = uiStyle.hudDashReadyFlashTime;
            dashWasReady = dashNowReady;
            dashReadyFlash = std::max(0.f, dashReadyFlash - dt);
        }

        // Score count-up: ease the HUD value toward the real score quickly.
        scoreDisplay += (static_cast<float>(score) - scoreDisplay) *
                        std::min(1.f, dt * uiStyle.hudScoreCountUpSpeed);
        if (std::fabs(scoreDisplay - static_cast<float>(score)) < 0.5f)
            scoreDisplay = static_cast<float>(score);

        // Run timer: accumulates only during actual play (paused/game-over freeze it).
        if (!gameOver && !victory)
        {
            runTime += dt;
        }

        // First-run hints: step the banner only while hints can be shown.
        if (hintsActive && !gameOver && !victory && runTime <= balance.hintsRunWindow)
        {
            hintTimer += dt;
            if (hintTimer >= balance.hintInterval)
            {
                hintDone[hintIndex] = true;
                hintIndex++;
                hintTimer = 0.f;
            }
            if (hintIndex >= 4) hintsActive = false;
        }

        // End-screen timeline advances on the defeat / victory overlay.
        if (gameOver || victory)
        {
            endAnimTime += dt;
        }

        // Combo window: expires back to x1.0 after balance.comboWindow with no kill.
        if (comboTimer > 0.f)
        {
            comboTimer -= dt;
            if (comboTimer <= 0.f)
            {
                comboTimer = 0.f;
                comboCount = 0;
                comboMultiplier = 1.f;
            }
        }

        // Buffered attack / dash: fire once the action becomes available, then clear.
        attackBufferTimer = std::max(0.f, attackBufferTimer - dt);
        dashBufferTimer = std::max(0.f, dashBufferTimer - dt);

        if (!gameOver && !victory && !paused)
        {
            if (attackBufferTimer > 0.f && attackCooldown <= 0.f)
            {
                attackBufferTimer = 0.f;
                performAttack();
            }
            if (dashBufferTimer > 0.f && dashCooldown <= 0.f)
            {
                dashBufferTimer = 0.f;
                performDash();
            }
        }

        // Screen Shake
        sf::Vector2f shakeOffset(0.f, 0.f);
        if (shakeTimer > 0.f)
        {
            shakeTimer -= dt;
            float curIntensity = (shakeTimer / 0.22f) * shakeIntensity;
            shakeOffset = sf::Vector2f(
                (float(rand() % 100 - 50) / 50.f) * curIntensity,
                (float(rand() % 100 - 50) / 50.f) * curIntensity
            );
        }

        // Weather Particles (Fireflies in forest, cold Mountain Snow/Wind motes in mountains)
        for (auto& ff : weatherMotes)
        {
            if (currentRealm == RealmLocation::ForestValley)
            {
                ff.pos += ff.vel * dt;
                ff.pos.x += std::sin(totalTime * 2.f + ff.pos.y) * 12.f * dt;
                ff.pos.y += std::cos(totalTime * 1.5f + ff.pos.x) * 12.f * dt;
            }
            else
            {
                // Cold high mountain wind & snow motes blowing diagonally
                ff.pos.x -= 85.f * dt;
                ff.pos.y += 35.f * dt;
            }

            if (ff.pos.x < 0) ff.pos.x = 1100.f;
            if (ff.pos.x > 1100.f) ff.pos.x = 0.f;
            if (ff.pos.y < 0) ff.pos.y = 750.f;
            if (ff.pos.y > 750.f) ff.pos.y = 0.f;
        }

        // FX Particles
        for (int i = (int)fxParticles.size() - 1; i >= 0; --i)
        {
            fxParticles[i].life += dt;
            fxParticles[i].pos += fxParticles[i].vel * dt;
            if (fxParticles[i].life >= fxParticles[i].maxLife)
            {
                fxParticles.erase(fxParticles.begin() + i);
            }
        }

        // Ghost Trails
        for (int i = (int)ghostTrails.size() - 1; i >= 0; --i)
        {
            ghostTrails[i].alpha -= dt * 4.5f;
            if (ghostTrails[i].alpha <= 0.f)
            {
                ghostTrails.erase(ghostTrails.begin() + i);
            }
        }

        // Shockwaves
        for (int i = (int)shockwaves.size() - 1; i >= 0; --i)
        {
            shockwaves[i].life += dt;
            shockwaves[i].radius += 240.f * dt;

            if (!shockwaves[i].hitPlayer && !gameOver && !victory)
            {
                float dx = playerPos.x - shockwaves[i].pos.x;
                float dy = playerPos.y - shockwaves[i].pos.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (std::abs(dist - shockwaves[i].radius) < 22.f)
                {
                    shockwaves[i].hitPlayer = true; // consumed on touch, dodged or not

                    if (dashIFrameTimer > 0.f)
                    {
                        // Boss shockwave slipped through the dash i-frames: perfect dodge.
                        if (perfectDodgeCooldown <= 0.f) awardPerfectDodge();
                    }
                    else
                    {
                        registerPlayerDamage(std::min(health, shockwaves[i].damage));
                        health = std::max(0.f, health - shockwaves[i].damage);
                        healBlockTimer = balance.healDelayAfterDamage;
                        hitstopTimer = std::min(hitstopTimer + balance.hitstopPlayerHit, maxHitstop);
                        int dmg = static_cast<int>(std::round(shockwaves[i].damage));
                        spawnFloatingNumber(playerPos, dmg, sf::Color(255, 65, 65));
                        recordPlayerDamage(static_cast<float>(dmg));

                        hitSound.play();
                        addScreenShake(12.f, 0.35f);
                        spawnSparks(playerPos, sf::Color(255, 60, 60), 20);

                        if (health <= 0.f)
                        {
                            gameOver = true;
                            recordMatch("DEFEATED");
                        }
                    }
                }
            }

            if (shockwaves[i].life >= shockwaves[i].maxLife || shockwaves[i].radius >= shockwaves[i].maxRadius)
            {
                shockwaves.erase(shockwaves.begin() + i);
            }
        }

        // Floating Damage Numbers Update
        for (auto& fn : floatingNumbers)
        {
            if (!fn.active) continue;
            fn.life -= dt;
            fn.pos.y -= 38.f * dt;
            if (fn.life <= 0.f)
            {
                fn.active = false;
            }
        }

        // Marvel-style teleport pod animation
        updateTeleportPod(teleportPod, dt, !gameOver && !victory);

        // Gameplay Update
        sf::Vector2f moveDir(0.f, 0.f);

        if (!gameOver && !victory && !paused)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                moveDir.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                moveDir.x += 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                moveDir.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                moveDir.y += 1.f;

            if (moveDir.x != 0.f || moveDir.y != 0.f)
                dismissHint(0); // performed the move action

            if (moveDir.x != 0.f && moveDir.y != 0.f)
            {
                moveDir.x *= 0.7071f;
                moveDir.y *= 0.7071f;
            }

            if (moveDir.x != 0.f || moveDir.y != 0.f)
                facing = moveDir;

            float moveSpeed = (dashTimer > 0.f) ? 680.f : 250.f;
            sf::Vector2f actualMove = (dashTimer > 0.f) ? dashDir : moveDir;

            if (dashTimer > 0.f && (rand() % 2 == 0))
            {
                ghostTrails.push_back({playerPos, facing, 0.8f});
            }

            sf::Vector2f nextPos = playerPos + actualMove * moveSpeed * dt;

            // Realm Collisions (shared with enemy movement)
            nextPos = resolveObstacleCollision(nextPos, 18.f, currentRealm,
                                               forestBoulders, forestTrees, mountainCrags);
            playerPos = nextPos;

            // ===============================================
            // REALM PORTAL TELEPORTATION LOGIC
            // ===============================================
            sf::Vector2f portalPos = (currentRealm == RealmLocation::ForestValley) ? sf::Vector2f(1000.f, 360.f) : sf::Vector2f(100.f, 440.f);
            float pdist = std::sqrt((playerPos.x - portalPos.x) * (playerPos.x - portalPos.x) + (playerPos.y - portalPos.y) * (playerPos.y - portalPos.y));

            if (pdist < 50.f && portalCooldown <= 0.f)
            {
                portalCooldown = 1.2f;
                teleportSound.play();
                addScreenShake(8.f, 0.25f);

                if (currentRealm == RealmLocation::ForestValley)
                {
                    currentRealm = RealmLocation::MountainPeaks;
                    playerPos = {170.f, 440.f};
                    spawnSparks(playerPos, sf::Color(255, 140, 60), 30);
                }
                else
                {
                    currentRealm = RealmLocation::ForestValley;
                    playerPos = {940.f, 360.f};
                    spawnSparks(playerPos, sf::Color(100, 240, 255), 30);
                }

                // Re-open the gateway we just emerged from
                initTeleportPod(teleportPod,
                    (currentRealm == RealmLocation::ForestValley) ? sf::Vector2f(1000.f, 360.f) : sf::Vector2f(100.f, 440.f),
                    (currentRealm == RealmLocation::ForestValley) ? sf::Color(70, 220, 255) : sf::Color(255, 140, 45),
                    true);
            }

            // Collect Crystals
            for (size_t i = 0; i < crystals.size(); ++i)
            {
                if (crystals[i].realm != currentRealm || crystals[i].collected) continue;

                float dx = playerPos.x - crystals[i].pos.x;
                float dy = playerPos.y - crystals[i].pos.y;
                if (std::sqrt(dx * dx + dy * dy) < 38.f)
                {
                    crystals[i].collected = true;
                    collectedCount++;
                    score += balance.scoreCrystal;
                    breakdownCrystals += balance.scoreCrystal;
                    health = std::min(maxHealth, health + 20.f);

                    crystalSound.play();
                    spawnSparks(crystals[i].pos, crystals[i].color, 35);
                    spawnSparks(playerPos, sf::Color(80, 255, 140), 20);
                }
            }

            // Auto-Healing: disabled for balance.healDelayAfterDamage seconds after
            // taking real damage, then restores balance.healPerSecond HP/s until full.
            damageTimer -= dt;
            healBlockTimer = std::max(0.f, healBlockTimer - dt);

            if (healBlockTimer <= 0.f && health < maxHealth && !gameOver && !victory)
            {
                health = std::min(maxHealth, health + balance.healPerSecond * dt);

                healSparkTimer += dt;
                if (healSparkTimer >= 0.5f)
                {
                    healSparkTimer = 0.f;
                    spawnSparks(playerPos, sf::Color(80, 255, 140), 4);
                }
            }
            else
            {
                healSparkTimer = 0.f;
            }

            // Enemy Logic in current realm
            for (auto& enemy : enemies)
            {
                if (enemy.realm != currentRealm || !enemy.isAlive) continue;
                enemy.hitTimer = std::max(0.f, enemy.hitTimer - dt);

                // Sword Hit
                if (attackTimer > 0.f)
                {
                    float dx = enemy.pos.x - playerPos.x;
                    float dy = enemy.pos.y - playerPos.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    float hitRadius = (enemy.type == EnemyType::Boss) ? 120.f : 85.f;

                    if (dist < hitRadius && dist > 0.f)
                    {
                        float dot = (facing.x * dx + facing.y * dy) / dist;
                        if (dot > 0.15f && enemy.hitTimer <= 0.f &&
                            !(enemy.type == EnemyType::Boss && enemy.phaseInvulnTimer > 0.f))
                        {
                            enemy.health -= 1.f;
                            enemy.hitTimer = 0.28f;
                            hitstopTimer = std::min(hitstopTimer + balance.hitstopEnemyHit, maxHitstop);
                            spawnFloatingNumber(enemy.pos, 1, sf::Color(255, 255, 255));
                            hitSound.play();
                            spawnSparks(enemy.pos, sf::Color(255, 80, 100), 22);
                            addScreenShake(6.f, 0.2f);

                            float kbDist = (enemy.type == EnemyType::Boss) ? 15.f : 45.f;
                            enemy.pos += sf::Vector2f(dx / dist * kbDist, dy / dist * kbDist);
                            enemy.pos = resolveObstacleCollision(enemy.pos, balance.enemyCollisionRadius, currentRealm,
                                                                 forestBoulders, forestTrees, mountainCrags);

                            if (enemy.health <= 0.f)
                            {
                                enemy.isAlive = false;

                                // Combo grows only while chaining kills inside the window.
                                if (comboTimer > 0.f) comboCount++;
                                else comboCount = 1;
                                comboTimer = balance.comboWindow;
                                comboMultiplier = std::min(balance.comboMaxMultiplier,
                                    1.f + balance.comboStep * static_cast<float>(comboCount - 1));

                                int killValue = (enemy.type == EnemyType::Boss) ? balance.scoreKillBoss
                                              : (enemy.type == EnemyType::Demon) ? balance.scoreKillDemon
                                              : balance.scoreKillBasic;
                                int killGain = static_cast<int>(std::round(killValue * comboMultiplier));
                                score += killGain;
                                breakdownKills += killValue;
                                breakdownCombo += killGain - killValue;

                                spawnSparks(enemy.pos, sf::Color(220, 80, 255), 45);
                                addScreenShake(12.f, 0.35f);
                            }
                        }
                    }
                }

                // Boss Phases & Slam
                if (enemy.type == EnemyType::Boss)
                {
                    // Phase progression (health only falls, so phases never go backward).
                    int desiredPhase = (enemy.health <= enemy.maxHealth * balance.bossPhase2Threshold) ? 3
                                     : (enemy.health <= enemy.maxHealth * balance.bossPhase1Threshold) ? 2
                                     : 1;
                    if (desiredPhase > enemy.phase)
                    {
                        enemy.phase = desiredPhase;
                        enemy.speedScale = (enemy.phase >= 3) ? balance.bossPhase3SpeedMul : 1.f;
                        enemy.phaseInvulnTimer = balance.bossPhaseInvulnTime;
                        enemy.phaseTextTimer = balance.bossPhaseTextTime;
                        enemy.attackTimer = 0.f;
                        enemy.slamWarned = false;
                        enemy.isWindingUp = false;
                        enemy.pendingSecondRing = false;
                        enemy.secondRingTimer = 0.f;
                        warnSound.play();                 // short roar (reused)
                        addScreenShake(16.f, 0.5f);
                    }

                    if (enemy.phaseTextTimer > 0.f)
                        enemy.phaseTextTimer = std::max(0.f, enemy.phaseTextTimer - dt);

                    if (enemy.phaseInvulnTimer > 0.f)
                    {
                        // Invulnerable & stationary during the phase-change pause.
                        enemy.phaseInvulnTimer = std::max(0.f, enemy.phaseInvulnTimer - dt);
                    }
                    else
                    {
                        float slamInterval = (enemy.phase == 3) ? balance.bossSlamIntervalP3
                                           : (enemy.phase == 2) ? balance.bossSlamIntervalP2
                                           : balance.bossSlamIntervalP1;

                        enemy.attackTimer += dt;
                        if (enemy.attackTimer >= (slamInterval - balance.bossSlamTelegraph) && !enemy.slamWarned)
                        {
                            enemy.slamWarned = true;
                            warnSound.play();
                        }
                        if (enemy.attackTimer >= slamInterval)
                        {
                            enemy.attackTimer = 0.f;
                            enemy.slamWarned = false;
                            shockwaves.push_back({enemy.pos, 10.f, 240.f, 0.f, 1.2f, 16.f, false});
                            slamSound.play();
                            addScreenShake(14.f, 0.4f);
                            spawnSparks(enemy.pos, sf::Color(255, 200, 60), 30);

                            if (enemy.phase >= 2) // schedule the extra, wider ring
                            {
                                enemy.pendingSecondRing = true;
                                enemy.secondRingTimer = 0.f;
                            }
                        }

                        // Second, wider shockwave ring after the delay.
                        if (enemy.pendingSecondRing)
                        {
                            enemy.secondRingTimer += dt;
                            if (enemy.secondRingTimer >= balance.bossSecondRingDelay)
                            {
                                enemy.pendingSecondRing = false;
                                enemy.secondRingTimer = 0.f;
                                shockwaves.push_back({enemy.pos, 10.f, balance.bossSecondRingRadius, 0.f, 1.6f, 16.f, false});
                                slamSound.play();
                                addScreenShake(14.f, 0.4f);
                                spawnSparks(enemy.pos, sf::Color(255, 120, 60), 30);
                            }
                        }
                    }
                }

                // Enemy Movement & Contact Damage
                float dx = playerPos.x - enemy.pos.x;
                float dy = playerPos.y - enemy.pos.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                float touchDist = (enemy.type == EnemyType::Boss) ? 65.f : 44.f;

                bool hasWindup = (enemy.type == EnemyType::Boss || enemy.type == EnemyType::Demon);
                float maxWindup = (enemy.type == EnemyType::Boss) ? balance.bossMeleeWindup : balance.demonWindup;

                if (hasWindup)
                {
                    // Boss is invulnerable & stationary during a phase-change pause.
                    bool bossFrozen = (enemy.type == EnemyType::Boss && enemy.phaseInvulnTimer > 0.f);

                    if (!bossFrozen && dist < touchDist && !enemy.isWindingUp)
                    {
                        enemy.isWindingUp = true;
                        enemy.windupTimer = 0.f;
                    }

                    if (enemy.isWindingUp)
                    {
                        // During a wind-up the enemy keeps its position (does not chase)
                        enemy.windupTimer += dt;
                        if (enemy.windupTimer >= maxWindup)
                        {
                            enemy.isWindingUp = false;
                            enemy.windupTimer = 0.f;

                            // Deal contact damage if player is still within touch range
                            if (dist < touchDist && dashIFrameTimer <= 0.f && damageTimer <= 0.f)
                            {
                                registerPlayerDamage(std::min(health, enemy.damage));
                                health = std::max(0.f, health - enemy.damage);
                                healBlockTimer = balance.healDelayAfterDamage;
                                hitstopTimer = std::min(hitstopTimer + balance.hitstopPlayerHit, maxHitstop);
                                damageTimer = 0.65f;
                                int dmg = static_cast<int>(std::round(enemy.damage));
                                spawnFloatingNumber(playerPos, dmg, sf::Color(255, 65, 65));
                                recordPlayerDamage(static_cast<float>(dmg));

                                hitSound.play();
                                addScreenShake(8.f, 0.25f);
                                spawnSparks(playerPos, sf::Color(255, 50, 50), 18);

                                if (health <= 0.f)
                                {
                                    gameOver = true;
                                    recordMatch("DEFEATED");
                                }
                            }
                            else if (dist < touchDist && dashIFrameTimer > 0.f && perfectDodgeCooldown <= 0.f)
                            {
                                // Dashed through the melee swing: perfect dodge.
                                awardPerfectDodge();
                            }
                        }
                    }
                    else if (!bossFrozen)
                    {
                        moveEnemyWithCollision(enemy, playerPos, dt, currentRealm,
                            balance.enemyCollisionRadius, forestBoulders, forestTrees, mountainCrags);
                    }
                }
                else if (enemy.type == EnemyType::Charger)
                {
                    // Charger: walk -> wind-up flash -> straight charge -> stun.
                    if (enemy.chargePhase == 3) // stunned
                    {
                        enemy.chargeTimer += dt;
                        if (enemy.chargeTimer >= balance.chargerStun)
                        {
                            enemy.chargePhase = 0;
                            enemy.chargeTimer = 0.f;
                        }
                    }
                    else if (enemy.chargePhase == 2) // charging
                    {
                        enemy.chargeTimer += dt;

                        sf::Vector2f next = enemy.pos + enemy.chargeDir * balance.chargerChargeSpeed * dt;
                        sf::Vector2f resolved = resolveObstacleCollision(next, balance.enemyCollisionRadius, currentRealm,
                                                                         forestBoulders, forestTrees, mountainCrags);
                        float bx = resolved.x - next.x;
                        float by = resolved.y - next.y;
                        enemy.pos = resolved;

                        // Hitting an obstacle ends the charge and stuns the Charger.
                        if (bx * bx + by * by > 0.0001f || enemy.chargeTimer >= balance.chargerDuration)
                        {
                            enemy.chargePhase = 3;
                            enemy.chargeTimer = 0.f;
                        }

                        if (dist < touchDist && dashIFrameTimer <= 0.f && damageTimer <= 0.f)
                        {
                            registerPlayerDamage(std::min(health, enemy.damage));
                            health = std::max(0.f, health - enemy.damage);
                            healBlockTimer = balance.healDelayAfterDamage;
                            hitstopTimer = std::min(hitstopTimer + balance.hitstopPlayerHit, maxHitstop);
                            damageTimer = 0.65f;
                            int dmg = static_cast<int>(std::round(enemy.damage));
                            spawnFloatingNumber(playerPos, dmg, sf::Color(255, 65, 65));
                            recordPlayerDamage(static_cast<float>(dmg));

                            hitSound.play();
                            addScreenShake(8.f, 0.25f);
                            spawnSparks(playerPos, sf::Color(255, 50, 50), 18);

                            if (health <= 0.f)
                            {
                                gameOver = true;
                                recordMatch("DEFEATED");
                            }
                        }
                        else if (dist < touchDist && dashIFrameTimer > 0.f && perfectDodgeCooldown <= 0.f)
                        {
                            // Dashed through the charge: perfect dodge.
                            awardPerfectDodge();
                        }
                    }
                    else if (enemy.chargePhase == 1) // wind-up
                    {
                        enemy.chargeTimer += dt;
                        if (enemy.chargeTimer >= balance.chargerWindup)
                        {
                            float cdx = playerPos.x - enemy.pos.x;
                            float cdy = playerPos.y - enemy.pos.y;
                            float cd = std::sqrt(cdx * cdx + cdy * cdy);
                            enemy.chargeDir = (cd > 0.0001f) ? sf::Vector2f(cdx / cd, cdy / cd)
                                                             : sf::Vector2f(1.f, 0.f);
                            enemy.chargePhase = 2;
                            enemy.chargeTimer = 0.f;
                            enemy.isWindingUp = false;
                        }
                    }
                    else // walking (phase 0)
                    {
                        enemy.isWindingUp = false;
                        enemy.chargeDir = (dist > 0.0001f) ? sf::Vector2f(dx / dist, dy / dist)
                                                           : sf::Vector2f(1.f, 0.f);

                        if (dist <= balance.chargerChargeRange && dist > 0.f &&
                            hasClearLine(enemy.pos, playerPos, currentRealm, forestBoulders, forestTrees, mountainCrags))
                        {
                            enemy.chargePhase = 1; // flash, then commit to the charge
                            enemy.chargeTimer = 0.f;
                            enemy.isWindingUp = true;
                        }
                        else
                        {
                            moveEnemyWithCollision(enemy, playerPos, dt, currentRealm,
                                balance.enemyCollisionRadius, forestBoulders, forestTrees, mountainCrags);
                        }
                    }
                }
                else
                {
                    // Crawlers: chase the player while sliding around obstacles
                    moveEnemyWithCollision(enemy, playerPos, dt, currentRealm,
                        balance.enemyCollisionRadius, forestBoulders, forestTrees, mountainCrags);

                    if (dist < touchDist && dashIFrameTimer <= 0.f && damageTimer <= 0.f)
                    {
                        registerPlayerDamage(std::min(health, enemy.damage));
                        health = std::max(0.f, health - enemy.damage);
                        healBlockTimer = balance.healDelayAfterDamage;
                        hitstopTimer = std::min(hitstopTimer + balance.hitstopPlayerHit, maxHitstop);
                        damageTimer = 0.65f;
                        int dmg = static_cast<int>(std::round(enemy.damage));
                        spawnFloatingNumber(playerPos, dmg, sf::Color(255, 65, 65));
                        recordPlayerDamage(static_cast<float>(dmg));

                        hitSound.play();
                        addScreenShake(8.f, 0.25f);
                        spawnSparks(playerPos, sf::Color(255, 50, 50), 18);

                        if (health <= 0.f)
                        {
                            gameOver = true;
                            recordMatch("DEFEATED");
                        }
                    }
                    else if (dist < touchDist && dashIFrameTimer > 0.f && perfectDodgeCooldown <= 0.f)
                    {
                        // Dashed through the crawler: perfect dodge.
                        awardPerfectDodge();
                    }
                }
            }

            // Epic Victory: Collect all 4 crystals and defeat all enemies
            bool allEnemiesDead = std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.isAlive; });
            if (collectedCount == (int)crystals.size() && allEnemiesDead)
            {
                victory = true;
                breakdownVictory = balance.victoryBonus;
                score += balance.victoryBonus;

                int damageBonus = (totalDamageTaken <= balance.noDamageMaxTaken) ? balance.noDamageBonusPerfect
                                : (totalDamageTaken <= balance.lowDamageMaxTaken) ? balance.noDamageBonusLow
                                : (totalDamageTaken <= balance.midDamageMaxTaken) ? balance.noDamageBonusMid
                                : 0;
                breakdownNoDamage = damageBonus;
                score += damageBonus;

                // Time bonus: 2 pts per full second saved under the time target.
                if (runTime < balance.timeBonusTarget)
                {
                    breakdownTime = balance.timeBonusPerSecond *
                                    static_cast<int>(std::floor(balance.timeBonusTarget - runTime));
                    score += breakdownTime;
                }

                recordMatch("VICTORY");
            }
        }

        // ==========================================
        // RENDERING PIPELINE (Complete Environments)
        // ==========================================

        sf::View worldView(sf::FloatRect({0.f, 0.f}, {1100.f, 750.f}));
        worldView.setCenter({550.f + shakeOffset.x, 375.f + shakeOffset.y});
        window.setView(worldView);

        if (currentRealm == RealmLocation::ForestValley)
        {
            // ==========================================
            // ENVIRONMENT 1: MYSTIC FOREST VALLEY
            // ==========================================
            window.clear(sf::Color(24, 42, 32)); // Forest soil

            // Grass Patches
            sf::RectangleShape patch({160.f, 120.f});
            patch.setFillColor(sf::Color(29, 52, 38));
            for (int i = 0; i < 8; ++i) {
                patch.setPosition({float((i * 197 + 30) % 1000), float((i * 131 + 50) % 650)});
                window.draw(patch);
            }

            // River
            sf::VertexArray river(sf::PrimitiveType::TriangleStrip);
            for (int y = 0; y <= 750; y += 15)
            {
                float curve = std::sin((float)y * 0.012f + 0.8f) * 28.f;
                float leftX = 525.f + curve;
                float rightX = 615.f + curve;

                sf::Color riverCol(35, 115, 165, 220);
                sf::Vertex v1, v2;
                v1.position = {leftX, (float)y};
                v1.color = riverCol;
                v2.position = {rightX, (float)y};
                v2.color = riverCol;
                river.append(v1);
                river.append(v2);
            }
            window.draw(river);

            // River Shimmer
            for (int i = 0; i < 20; ++i) {
                float ry = std::fmod(totalTime * 65.f + i * 36.f, 750.f);
                float curveAtY = std::sin(ry * 0.012f + 0.8f) * 28.f;
                float rx = 525.f + curveAtY + (15.f + std::fmod(i * 17.f, 60.f));
                sf::RectangleShape shimmer({14.f, 3.f});
                shimmer.setFillColor(sf::Color(170, 240, 255, 190));
                shimmer.setPosition({rx, ry});
                window.draw(shimmer);
            }

            // Dirt Path
            sf::VertexArray dirtPath(sf::PrimitiveType::TriangleStrip);
            for (int x = 0; x <= 1100; x += 40)
            {
                float curve = std::sin(x * 0.005f + 1.2f) * 45.f;
                float topY = 360.f + curve - 55.f;
                float botY = 360.f + curve + 55.f;
                sf::Color pathCol(95, 78, 56);
                sf::Vertex v1, v2;
                v1.position = {(float)x, topY};
                v1.color = pathCol;
                v2.position = {(float)x, botY};
                v2.color = pathCol;
                dirtPath.append(v1);
                dirtPath.append(v2);
            }
            window.draw(dirtPath);
        }
        else
        {
            // ==========================================
            // ENVIRONMENT 2: CRAGGY MOUNTAIN PEAKS & CANYON GORGE
            // ==========================================
            window.clear(sf::Color(32, 26, 36)); // Dark mountain bedrock

            // 1. Distant High Mountain Peaks (বিশাল পাহাড় ও পর্বতমালা)
            for (const auto& mp : backgroundMountains)
            {
                drawMountainRidge(window, mp.peak, mp.halfWidth, mp.height, mp.litColor, mp.shadowColor);
            }

            // Mountain Sky Horizon Fog
            sf::RectangleShape fog({1100.f, 60.f});
            fog.setPosition({0.f, 180.f});
            fog.setFillColor(sf::Color(60, 48, 65, 160));
            window.draw(fog);

            // 2. Mountain Canyon Gorge (গিরিখাত)
            sf::RectangleShape gorge({1100.f, 90.f});
            gorge.setPosition({0.f, 320.f});
            gorge.setFillColor(sf::Color(15, 10, 20)); // Bottomless abyss
            window.draw(gorge);

            // Glowing magma vein at bottom of canyon
            sf::RectangleShape magmaVein({1100.f, 12.f});
            magmaVein.setPosition({0.f, 360.f});
            magmaVein.setFillColor(sf::Color(255, 80, 20, 210));
            window.draw(magmaVein);

            // 3. Wooden & Stone Mountain Bridges across the Gorge
            for (float bx : {250.f, 550.f, 850.f})
            {
                sf::RectangleShape bridge({70.f, 120.f});
                bridge.setPosition({bx - 35.f, 305.f});
                bridge.setFillColor(sf::Color(75, 52, 38));
                bridge.setOutlineThickness(2.5f);
                bridge.setOutlineColor(sf::Color(45, 30, 20));
                window.draw(bridge);

                // Bridge Planks
                for (int pl = 0; pl < 6; ++pl)
                {
                    sf::RectangleShape plank({66.f, 4.f});
                    plank.setPosition({bx - 33.f, 315.f + pl * 17.f});
                    plank.setFillColor(sf::Color(105, 75, 50));
                    window.draw(plank);
                }
            }

            // 4. Rocky Cliff Steppes
            sf::RectangleShape cliffShelf({1100.f, 25.f});
            cliffShelf.setPosition({0.f, 225.f});
            cliffShelf.setFillColor(sf::Color(55, 45, 60));
            cliffShelf.setOutlineThickness(2.f);
            cliffShelf.setOutlineColor(sf::Color(35, 28, 40));
            window.draw(cliffShelf);
        }

        // Boss Shockwaves
        for (const auto& sw : shockwaves)
        {
            sf::CircleShape ring(sw.radius, 36);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(6.f);
            ring.setOutlineColor(sf::Color(255, 180, 40, static_cast<std::uint8_t>(220 * (1.f - sw.life / sw.maxLife))));
            ring.setOrigin({sw.radius, sw.radius});
            ring.setPosition(sw.pos);
            window.draw(ring);
        }

        // Boss Slam Ground Telegraphs (Pulsing Red Rings)
        for (const auto& enemy : enemies)
        {
            if (enemy.type == EnemyType::Boss && enemy.isAlive && enemy.realm == currentRealm)
            {
                float slamInterval = (enemy.phase == 3) ? balance.bossSlamIntervalP3
                                   : (enemy.phase == 2) ? balance.bossSlamIntervalP2
                                   : balance.bossSlamIntervalP1;

                if (enemy.phaseInvulnTimer <= 0.f && enemy.attackTimer >= (slamInterval - balance.bossSlamTelegraph))
                {
                    float pulse = (std::sin(totalTime * 18.f) + 1.f) * 0.5f;
                    sf::CircleShape slamRing(240.f, 48);
                    slamRing.setOrigin({240.f, 240.f});
                    slamRing.setPosition(enemy.pos);
                    slamRing.setFillColor(sf::Color(255, 30, 40, static_cast<std::uint8_t>(20 + pulse * 35)));
                    slamRing.setOutlineThickness(3.5f + pulse * 2.5f);
                    slamRing.setOutlineColor(sf::Color(255, 45, 60, static_cast<std::uint8_t>(170 + pulse * 85)));
                    window.draw(slamRing);
                }

                // Wider second-ring telegraph during the post-slam delay (phase 2+)
                if (enemy.pendingSecondRing)
                {
                    float pulse2 = (std::sin(totalTime * 24.f) + 1.f) * 0.5f;
                    const float rr = balance.bossSecondRingRadius;
                    sf::CircleShape ring2(rr, 64);
                    ring2.setOrigin({rr, rr});
                    ring2.setPosition(enemy.pos);
                    ring2.setFillColor(sf::Color(255, 120, 40, static_cast<std::uint8_t>(15 + pulse2 * 30)));
                    ring2.setOutlineThickness(4.f + pulse2 * 3.f);
                    ring2.setOutlineColor(sf::Color(255, 150, 60, static_cast<std::uint8_t>(150 + pulse2 * 95)));
                    window.draw(ring2);
                }
            }
        }

        // Shadows
        sf::CircleShape dropShadow;
        dropShadow.setFillColor(sf::Color(0, 0, 0, 85));

        if (currentRealm == RealmLocation::ForestValley)
        {
            for (const auto& tree : forestTrees) {
                dropShadow.setRadius(32.f * tree.scale);
                dropShadow.setScale({1.6f, 0.6f});
                dropShadow.setOrigin({32.f * tree.scale, 32.f * tree.scale});
                dropShadow.setPosition({tree.pos.x + 15.f, tree.pos.y + 35.f});
                window.draw(dropShadow);
            }
            for (const auto& boulder : forestBoulders) {
                dropShadow.setRadius(boulder.radius);
                dropShadow.setScale({1.4f, 0.5f});
                dropShadow.setOrigin({boulder.radius, boulder.radius});
                dropShadow.setPosition({boulder.pos.x + 8.f, boulder.pos.y + 10.f});
                window.draw(dropShadow);
            }
        }
        else
        {
            for (const auto& crag : mountainCrags) {
                dropShadow.setRadius(crag.radius);
                dropShadow.setScale({1.5f, 0.5f});
                dropShadow.setOrigin({crag.radius, crag.radius});
                dropShadow.setPosition({crag.pos.x + 12.f, crag.pos.y + 14.f});
                window.draw(dropShadow);
            }
        }

        dropShadow.setRadius(18.f);
        dropShadow.setScale({1.3f, 0.5f});
        dropShadow.setOrigin({18.f, 18.f});
        dropShadow.setPosition({playerPos.x, playerPos.y + 16.f});
        window.draw(dropShadow);

        // ===============================================
        // Y-SORTED ENTITY RENDERING
        // ===============================================
        std::vector<Renderable> renderables;

        if (currentRealm == RealmLocation::ForestValley)
        {
            // Forest Boulders
            for (const auto& b : forestBoulders) {
                renderables.push_back({
                    b.pos.y,
                    [&window, b]() {
                        sf::CircleShape rock(b.radius, 7);
                        rock.setFillColor(sf::Color(80, 92, 98));
                        rock.setOutlineColor(sf::Color(45, 52, 58));
                        rock.setOutlineThickness(2.5f);
                        rock.setOrigin({b.radius, b.radius});
                        rock.setPosition(b.pos);
                        window.draw(rock);
                    }
                });
            }

            // Forest Trees
            for (const auto& tree : forestTrees) {
                renderables.push_back({
                    tree.pos.y + 35.f * tree.scale,
                    [&window, tree]() {
                        sf::RectangleShape trunk({22.f * tree.scale, 42.f * tree.scale});
                        trunk.setFillColor(sf::Color(82, 50, 32));
                        trunk.setOutlineThickness(2.f);
                        trunk.setOutlineColor(sf::Color(48, 28, 16));
                        trunk.setOrigin({11.f * tree.scale, 0.f});
                        trunk.setPosition({tree.pos.x, tree.pos.y});
                        window.draw(trunk);

                        sf::CircleShape canopy(36.f * tree.scale, 24);
                        canopy.setFillColor(sf::Color(25, 88, 52));
                        canopy.setOutlineThickness(3.f);
                        canopy.setOutlineColor(sf::Color(40, 115, 68));
                        canopy.setOrigin({36.f * tree.scale, 36.f * tree.scale});
                        canopy.setPosition({tree.pos.x, tree.pos.y - 10.f * tree.scale});
                        window.draw(canopy);
                    }
                });
            }
        }
        else
        {
            // Mountain Rocks & Cliffs (পাহাড়ের খাড়া পাথর ও শৃঙ্গ)
            for (const auto& crag : mountainCrags) {
                renderables.push_back({
                    crag.pos.y,
                    [&window, crag]() {
                        // Jagged crag polygon
                        sf::ConvexShape rock(6);
                        rock.setPoint(0, {-crag.radius * 0.8f, -crag.radius * 0.6f});
                        rock.setPoint(1, {0.f, -crag.radius * 1.1f});
                        rock.setPoint(2, {crag.radius * 0.9f, -crag.radius * 0.4f});
                        rock.setPoint(3, {crag.radius * 0.8f, crag.radius * 0.8f});
                        rock.setPoint(4, {-crag.radius * 0.2f, crag.radius * 1.0f});
                        rock.setPoint(5, {-crag.radius * 0.9f, crag.radius * 0.5f});
                        rock.setFillColor(sf::Color(65, 55, 75));
                        rock.setOutlineThickness(3.f);
                        rock.setOutlineColor(sf::Color(120, 105, 135));
                        rock.setPosition(crag.pos);
                        window.draw(rock);
                    }
                });
            }
        }

        // Marvel-style Teleport Pod (realm gateway)
        renderables.push_back({
            teleportPod.pos.y + teleportPod.baseRY,
            [&window, &teleportPod]() { drawTeleportPod(window, teleportPod); }
        });

        // Crystals in Current Realm
        for (size_t i = 0; i < crystals.size(); ++i) {
            if (crystals[i].realm != currentRealm || crystals[i].collected) continue;
            sf::Vector2f cpos = crystals[i].pos;
            sf::Color ccol = crystals[i].color;

            renderables.push_back({
                cpos.y + 15.f,
                [&window, cpos, totalTime, i, ccol]() {
                    float floatOffset = std::sin(totalTime * 3.5f + i * 1.5f) * 6.f;
                    float pulse = (std::sin(totalTime * 4.f + i * 2.f) + 1.f) * 0.5f;

                    sf::Vector2f crystalCenter = {cpos.x, cpos.y - 12.f + floatOffset};
                    drawRadialGlow(window, crystalCenter, 80.f + pulse * 15.f, sf::Color(ccol.r, ccol.g, ccol.b, static_cast<std::uint8_t>(60 + pulse * 30)));

                    sf::ConvexShape leftFacet(4);
                    leftFacet.setPoint(0, {0.f, -24.f});
                    leftFacet.setPoint(1, {-12.f, 0.f});
                    leftFacet.setPoint(2, {0.f, 18.f});
                    leftFacet.setPoint(3, {0.f, -24.f});
                    leftFacet.setFillColor(sf::Color(ccol.r / 2, ccol.g / 2, ccol.b / 2));
                    leftFacet.setPosition(crystalCenter);
                    window.draw(leftFacet);

                    sf::ConvexShape rightFacet(4);
                    rightFacet.setPoint(0, {0.f, -24.f});
                    rightFacet.setPoint(1, {12.f, 0.f});
                    rightFacet.setPoint(2, {0.f, 18.f});
                    rightFacet.setPoint(3, {0.f, -24.f});
                    rightFacet.setFillColor(ccol);
                    rightFacet.setPosition(crystalCenter);
                    window.draw(rightFacet);

                    sf::ConvexShape outline(4);
                    outline.setPoint(0, {0.f, -24.f});
                    outline.setPoint(1, {12.f, 0.f});
                    outline.setPoint(2, {0.f, 18.f});
                    outline.setPoint(3, {-12.f, 0.f});
                    outline.setFillColor(sf::Color::Transparent);
                    outline.setOutlineThickness(2.f);
                    outline.setOutlineColor(sf::Color::White);
                    outline.setPosition(crystalCenter);
                    window.draw(outline);
                }
            });
        }

        // Enemies in Current Realm
        for (const auto& enemy : enemies) {
            if (enemy.realm != currentRealm || !enemy.isAlive) continue;

            renderables.push_back({
                enemy.pos.y + ((enemy.type == EnemyType::Boss) ? 50.f : 20.f),
                [&window, enemy, totalTime]() {
                    sf::Vector2f epos = enemy.pos;
                    sf::Color baseColor = (enemy.type == EnemyType::Boss) ? sf::Color(255, 40, 60) : (enemy.type == EnemyType::Crawler ? sf::Color(80, 220, 120) : sf::Color(200, 35, 65));

                    if (enemy.type == EnemyType::Boss)
                    {
                        drawRadialGlow(window, epos, 160.f, sf::Color(255, 30, 60, 70));

                        bool isFlashing = enemy.isWindingUp && (std::fmod(totalTime * 20.f, 2.f) < 1.f);
                        sf::Color flashColor = isFlashing ? sf::Color::White : sf::Color(255, 35, 50);

                        float pulse = std::sin(totalTime * 8.f) * 4.f;
                        sf::CircleShape body(48.f + pulse, 36);
                        body.setFillColor(enemy.isWindingUp ? (isFlashing ? sf::Color(180, 50, 70) : sf::Color(70, 10, 20)) : sf::Color(25, 8, 35));
                        body.setOutlineThickness(5.f);
                        body.setOutlineColor(enemy.hitTimer > 0.f ? sf::Color::White : (enemy.isWindingUp ? flashColor : sf::Color(255, 40, 70)));
                        body.setOrigin({48.f + pulse, 48.f + pulse});
                        body.setPosition(epos);
                        window.draw(body);

                        sf::ConvexShape lHorn(4);
                        lHorn.setPoint(0, {-25.f, -30.f});
                        lHorn.setPoint(1, {-50.f, -70.f});
                        lHorn.setPoint(2, {-15.f, -50.f});
                        lHorn.setPoint(3, {-10.f, -30.f});
                        lHorn.setFillColor(sf::Color(15, 5, 20));
                        lHorn.setOutlineThickness(2.5f);
                        lHorn.setOutlineColor(sf::Color(255, 60, 80));
                        lHorn.setPosition(epos);
                        window.draw(lHorn);

                        sf::ConvexShape rHorn(4);
                        rHorn.setPoint(0, {25.f, -30.f});
                        rHorn.setPoint(1, {50.f, -70.f});
                        rHorn.setPoint(2, {15.f, -50.f});
                        rHorn.setPoint(3, {10.f, -30.f});
                        rHorn.setFillColor(sf::Color(15, 5, 20));
                        rHorn.setOutlineThickness(2.5f);
                        rHorn.setOutlineColor(sf::Color(255, 60, 80));
                        rHorn.setPosition(epos);
                        window.draw(rHorn);

                        sf::CircleShape eye1(8.f, 16);
                        eye1.setFillColor(sf::Color(255, 230, 40));
                        eye1.setPosition({epos.x - 22.f, epos.y - 18.f});

                        sf::CircleShape eye2(8.f, 16);
                        eye2.setFillColor(sf::Color(255, 230, 40));
                        eye2.setPosition({epos.x + 6.f, epos.y - 18.f});

                        window.draw(eye1);
                        window.draw(eye2);

                        sf::RectangleShape barBG({110.f, 12.f});
                        barBG.setPosition({epos.x - 55.f, epos.y - 85.f});
                        barBG.setFillColor(sf::Color(20, 10, 25, 220));
                        barBG.setOutlineThickness(2.f);
                        barBG.setOutlineColor(sf::Color(255, 50, 70, 220));
                        window.draw(barBG);

                        sf::RectangleShape barFill({110.f * (enemy.health / enemy.maxHealth), 12.f});
                        barFill.setPosition({epos.x - 55.f, epos.y - 85.f});
                        barFill.setFillColor(sf::Color(255, 35, 55));
                        window.draw(barFill);
                    }
                    else if (enemy.type == EnemyType::Crawler)
                    {
                        sf::CircleShape body(16.f, 20);
                        body.setFillColor((enemy.realm == RealmLocation::ForestValley) ? sf::Color(20, 45, 35) : sf::Color(55, 25, 20));
                        body.setOutlineThickness(2.5f);
                        body.setOutlineColor(enemy.hitTimer > 0.f ? sf::Color::White : baseColor);
                        body.setOrigin({16.f, 16.f});
                        body.setPosition(epos);
                        window.draw(body);
                    }
                    else if (enemy.type == EnemyType::Charger)
                    {
                        bool isFlashing = enemy.isWindingUp && (std::fmod(totalTime * 20.f, 2.f) < 1.f);
                        bool isStunned = (enemy.chargePhase == 3);
                        float ang = std::atan2(enemy.chargeDir.y, enemy.chargeDir.x) * 180.f / 3.14159265f;

                        sf::ConvexShape body(3);
                        body.setPoint(0, {24.f, 0.f});
                        body.setPoint(1, {-15.f, -16.f});
                        body.setPoint(2, {-15.f, 16.f});
                        body.setFillColor(isStunned ? sf::Color(45, 40, 55)
                                                    : (enemy.isWindingUp ? (isFlashing ? sf::Color(255, 235, 150) : sf::Color(120, 50, 10))
                                                                         : sf::Color(95, 45, 15)));
                        body.setOutlineThickness(3.f);
                        body.setOutlineColor(enemy.hitTimer > 0.f ? sf::Color::White : sf::Color(255, 150, 40));
                        body.setPosition(epos);
                        body.setRotation(sf::degrees(ang));
                        window.draw(body);
                    }
                    else
                    {
                        bool isFlashing = enemy.isWindingUp && (std::fmod(totalTime * 20.f, 2.f) < 1.f);
                        sf::Color flashColor = isFlashing ? sf::Color::White : sf::Color(255, 35, 50);

                        sf::CircleShape body(26.f, 24);
                        body.setFillColor(enemy.isWindingUp ? (isFlashing ? sf::Color(160, 40, 60) : sf::Color(65, 15, 25)) : sf::Color(35, 12, 45));
                        body.setOutlineThickness(3.f);
                        body.setOutlineColor(enemy.hitTimer > 0.f ? sf::Color::White : (enemy.isWindingUp ? flashColor : baseColor));
                        body.setOrigin({26.f, 26.f});
                        body.setPosition(epos);
                        window.draw(body);
                    }
                }
            });
        }

        // Ghost Trails
        for (const auto& gt : ghostTrails) {
            sf::RectangleShape ghost({22.f, 22.f});
            ghost.setFillColor(sf::Color(100, 220, 255, static_cast<std::uint8_t>(200 * gt.alpha)));
            ghost.setOrigin({11.f, 11.f});
            ghost.setPosition(gt.pos);
            window.draw(ghost);
        }

        // Player
        renderables.push_back({
            playerPos.y + 20.f,
            [&window, playerPos, facing, attackTimer, totalTime, gameOver, victory, moveDir, dashTimer]() {
                float angle = std::atan2(facing.y, facing.x) * 180.f / 3.14159265f;
                bool isMoving = (moveDir.x != 0.f || moveDir.y != 0.f);
                float walkStep = isMoving ? std::sin(totalTime * 14.f) * 7.f : 0.f;

                sf::RectangleShape leftLeg({7.f, 16.f});
                leftLeg.setFillColor(sf::Color(45, 35, 30));
                leftLeg.setOrigin({3.5f, 0.f});
                leftLeg.setPosition({playerPos.x - 6.f, playerPos.y + 4.f + walkStep});
                window.draw(leftLeg);

                sf::RectangleShape rightLeg({7.f, 16.f});
                rightLeg.setFillColor(sf::Color(45, 35, 30));
                rightLeg.setOrigin({3.5f, 0.f});
                rightLeg.setPosition({playerPos.x + 6.f, playerPos.y + 4.f - walkStep});
                window.draw(rightLeg);

                sf::RectangleShape torso({22.f, 22.f});
                torso.setFillColor((dashTimer > 0.f) ? sf::Color(140, 240, 255) : sf::Color(70, 165, 200));
                torso.setOutlineThickness(2.5f);
                torso.setOutlineColor(sf::Color(210, 245, 255));
                torso.setOrigin({11.f, 11.f});
                torso.setPosition({playerPos.x, playerPos.y - 2.f});
                window.draw(torso);

                sf::CircleShape helmet(11.f, 20);
                helmet.setFillColor(sf::Color(55, 75, 95));
                helmet.setOutlineThickness(2.f);
                helmet.setOutlineColor(sf::Color(190, 235, 255));
                helmet.setOrigin({11.f, 11.f});
                helmet.setPosition({playerPos.x, playerPos.y - 16.f});
                window.draw(helmet);

                sf::Vector2f lanternArm = playerPos + sf::Vector2f(-facing.y * 15.f + facing.x * 6.f, facing.x * 15.f + facing.y * 6.f);
                sf::CircleShape lantern(8.f, 16);
                lantern.setFillColor(sf::Color(255, 215, 80));
                lantern.setOutlineThickness(2.5f);
                lantern.setOutlineColor(sf::Color(160, 110, 20));
                lantern.setOrigin({8.f, 8.f});
                lantern.setPosition(lanternArm);
                window.draw(lantern);

                if (attackTimer > 0.f && !gameOver && !victory)
                {
                    sf::Vector2f swordPos = playerPos + facing * 40.f;

                    sf::RectangleShape sword({10.f, 44.f});
                    sword.setOrigin({5.f, 22.f});
                    sword.setFillColor(sf::Color(230, 250, 255));
                    sword.setOutlineThickness(2.5f);
                    sword.setOutlineColor(sf::Color::White);
                    sword.setPosition(swordPos);
                    sword.setRotation(sf::degrees(angle + 90.f));
                    window.draw(sword);

                    sf::CircleShape slashArc(42.f, 24);
                    slashArc.setFillColor(sf::Color(120, 230, 255, 130));
                    slashArc.setOutlineThickness(4.f);
                    slashArc.setOutlineColor(sf::Color::White);
                    slashArc.setOrigin({42.f, 42.f});
                    slashArc.setPosition(playerPos + facing * 22.f);
                    slashArc.setScale({1.25f, 0.5f});
                    slashArc.setRotation(sf::degrees(angle));
                    window.draw(slashArc);
                }
            }
        });

        std::sort(renderables.begin(), renderables.end(), [](const Renderable& a, const Renderable& b) {
            return a.y < b.y;
        });

        for (const auto& obj : renderables) {
            obj.draw();
        }

        // Lighting
        drawRadialGlow(window, playerPos, 220.f, sf::Color(255, 205, 90, 60));
        drawRadialGlow(window, playerPos, 110.f, sf::Color(255, 230, 140, 85));

        // FX Particles
        for (const auto& p : fxParticles) {
            float alphaPct = 1.f - (p.life / p.maxLife);
            sf::CircleShape part(p.size * alphaPct, 12);
            part.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, static_cast<std::uint8_t>(p.color.a * alphaPct)));
            part.setOrigin({p.size * alphaPct, p.size * alphaPct});
            part.setPosition(p.pos);
            window.draw(part);
        }

        // Weather particles
        for (const auto& ff : weatherMotes) {
            sf::Color weatherCol = (currentRealm == RealmLocation::ForestValley) ? sf::Color(255, 240, 140) : sf::Color(220, 235, 255);
            sf::CircleShape dot(ff.size, 10);
            dot.setFillColor(weatherCol);
            dot.setOrigin({ff.size, ff.size});
            dot.setPosition(ff.pos);
            window.draw(dot);
        }

        // Ambient Tint
        sf::RectangleShape vignette({1100.f, 750.f});
        vignette.setFillColor((currentRealm == RealmLocation::ForestValley) ? sf::Color(10, 18, 25, 45) : sf::Color(25, 20, 35, 50));
        window.draw(vignette);

        // Phase 3 arena tint (slightly red while the enraged boss lives)
        bool bossEnraged = false;
        for (const auto& enemy : enemies)
        {
            if (enemy.type == EnemyType::Boss && enemy.isAlive && enemy.realm == currentRealm && enemy.phase >= 3)
            {
                bossEnraged = true;
                break;
            }
        }
        if (bossEnraged)
        {
            sf::RectangleShape rageTint({1100.f, 750.f});
            rageTint.setFillColor(balance.bossPhase3Tint);
            window.draw(rageTint);
        }

        // Floating Damage Numbers (Rendered in World Space)
        if (fontLoaded)
        {
            for (const auto& fn : floatingNumbers)
            {
                if (!fn.active) continue;
                float alpha = std::clamp(fn.life / fn.maxLife, 0.f, 1.f);
                std::string numLabel = fn.dodge
                    ? "+" + std::to_string(fn.amount) + " DODGE"
                    : "-" + std::to_string(fn.amount);
                sf::Text numText(font, numLabel, 16);
                numText.setFillColor(sf::Color(fn.color.r, fn.color.g, fn.color.b, static_cast<std::uint8_t>(alpha * 255.f)));
                numText.setOutlineColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha * 220.f)));
                numText.setOutlineThickness(1.5f);
                sf::FloatRect nb = numText.getLocalBounds();
                numText.setOrigin({nb.position.x + nb.size.x / 2.f, nb.position.y + nb.size.y / 2.f});
                numText.setPosition(fn.pos);
                window.draw(numText);
            }
        }

        // Reset View for Static HUD
        window.setView(window.getDefaultView());

        // ---- Shared HUD panel helper: rounded dark translucent panel, thin gold border, drop shadow ----
        auto drawHudPanel = [&](const sf::FloatRect& r) {
            drawRoundedRect(
                sf::FloatRect({r.position.x + uiStyle.hudPanelShadowOffset,
                               r.position.y + uiStyle.hudPanelShadowOffset},
                              r.size),
                uiStyle.hudPanelCorner, uiStyle.hudPanelShadow);
            drawRoundedRect(r, uiStyle.hudPanelCorner, uiStyle.hudPanelBorder);
            sf::FloatRect inner({r.position.x + uiStyle.hudPanelBorderThickness,
                                 r.position.y + uiStyle.hudPanelBorderThickness},
                                {r.size.x - 2.f * uiStyle.hudPanelBorderThickness,
                                 r.size.y - 2.f * uiStyle.hudPanelBorderThickness});
            drawRoundedRect(inner, uiStyle.hudPanelCorner - 1.f, uiStyle.hudPanelBg);
        };

        // Left cluster shell (Health + Dash) at the common 25px margin
        drawHudPanel(sf::FloatRect({25.f, 25.f}, {270.f, 100.f}));

        // ---- Restyled HUD health bar: rounded panel + gradient fill + damage trail ----
        const sf::FloatRect hpBar({45.f, 47.f}, {210.f, 13.f});
        const float hpBorder = uiStyle.hudPanelBorderThickness;
        const sf::FloatRect hpInner(
            {hpBar.position.x + hpBorder, hpBar.position.y + hpBorder},
            {hpBar.size.x - 2.f * hpBorder, hpBar.size.y - 2.f * hpBorder});
        const sf::FloatRect hpFillArea(
            {hpInner.position.x + uiStyle.hudPanelInset, hpInner.position.y + uiStyle.hudPanelInset},
            {hpInner.size.x - 2.f * uiStyle.hudPanelInset, hpInner.size.y - 2.f * uiStyle.hudPanelInset});
        const float hpFrac = std::clamp(health / maxHealth, 0.f, 1.f);

        // Lagging trail value: falls toward HP smoothly, snaps up on heal.
        if (healthTrail > health)
        {
            healthTrail -= (healthTrail - health) * std::min(1.f, dt / uiStyle.hudHealthTrailTime);
            if (healthTrail < health) healthTrail = health;
        }
        else
        {
            healthTrail = health;
        }
        const float hpTrailFrac = std::clamp(healthTrail / maxHealth, 0.f, 1.f);

        // Subtle drop shadow
        drawRoundedRect(
            sf::FloatRect({hpBar.position.x + uiStyle.hudPanelShadowOffset,
                           hpBar.position.y + uiStyle.hudPanelShadowOffset},
                          hpBar.size),
            uiStyle.hudPanelCorner, uiStyle.hudPanelShadow);

        // Thin gold border, then dark semi-transparent background
        drawRoundedRect(hpBar, uiStyle.hudPanelCorner, uiStyle.hudPanelBorder);
        drawRoundedRect(hpInner, uiStyle.hudPanelCorner - hpBorder, uiStyle.hudPanelBg);

        // Lagging white damage trail (drawn behind the fill)
        if (hpTrailFrac > hpFrac + 0.001f)
        {
            sf::RectangleShape trail({hpFillArea.size.x * hpTrailFrac, hpFillArea.size.y});
            trail.setPosition(hpFillArea.position);
            trail.setFillColor(uiStyle.hudTrail);
            window.draw(trail);
        }

        // Vertical-gradient fill (lighter on top) with existing green/amber/red tiers
        if (hpFrac > 0.f)
        {
            const bool midTier = (health <= 60.f && health > 30.f);
            const bool lowTier = (health <= 30.f);
            const sf::Color top = lowTier ? uiStyle.hudHpLowTop : (midTier ? uiStyle.hudHpMidTop : uiStyle.hudHpTop);
            const sf::Color bottom = lowTier ? uiStyle.hudHpLowBottom : (midTier ? uiStyle.hudHpMidBottom : uiStyle.hudHpBottom);

            const float fx = hpFillArea.position.x;
            const float fy = hpFillArea.position.y;
            const float fw = hpFillArea.size.x * hpFrac;
            const float fh = hpFillArea.size.y;

            sf::Vertex grad[4];
            grad[0].position = {fx, fy};            grad[0].color = top;
            grad[1].position = {fx + fw, fy};       grad[1].color = top;
            grad[2].position = {fx + fw, fy + fh};  grad[2].color = bottom;
            grad[3].position = {fx, fy + fh};       grad[3].color = bottom;
            window.draw(grad, 4, sf::PrimitiveType::TriangleStrip);
        }

        // ---- Dash meter: same panel style, gradient fill, glow flash when ready ----
        const sf::FloatRect dashBar({45.f, 78.f}, {210.f, 13.f});
        const float dashBorder = uiStyle.hudPanelBorderThickness;
        const sf::FloatRect dashInner(
            {dashBar.position.x + dashBorder, dashBar.position.y + dashBorder},
            {dashBar.size.x - 2.f * dashBorder, dashBar.size.y - 2.f * dashBorder});
        const sf::FloatRect dashFillArea(
            {dashInner.position.x + uiStyle.hudPanelInset, dashInner.position.y + uiStyle.hudPanelInset},
            {dashInner.size.x - 2.f * uiStyle.hudPanelInset, dashInner.size.y - 2.f * uiStyle.hudPanelInset});
        const float dashFrac = std::clamp(1.f - (dashCooldown / dashMaxCooldown), 0.f, 1.f);
        const bool dashReady = (dashCooldown <= 0.f);

        drawRoundedRect(
            sf::FloatRect({dashBar.position.x + uiStyle.hudPanelShadowOffset,
                           dashBar.position.y + uiStyle.hudPanelShadowOffset},
                          dashBar.size),
            uiStyle.hudPanelCorner, uiStyle.hudPanelShadow);
        drawRoundedRect(dashBar, uiStyle.hudPanelCorner, uiStyle.hudPanelBorder);
        drawRoundedRect(dashInner, uiStyle.hudPanelCorner - dashBorder, uiStyle.hudPanelBg);

        // Gradient fill (cyan when ready; dimmed grey while recharging)
        if (dashFrac > 0.f)
        {
            const float dfx = dashFillArea.position.x;
            const float dfy = dashFillArea.position.y;
            const float dfw = dashFillArea.size.x * dashFrac;
            const float dfh = dashFillArea.size.y;
            sf::Vertex dgrad[4];
            dgrad[0].position = {dfx, dfy};            dgrad[0].color = dashReady ? uiStyle.hudDashTop : uiStyle.hudDashDim;
            dgrad[1].position = {dfx + dfw, dfy};      dgrad[1].color = dashReady ? uiStyle.hudDashTop : uiStyle.hudDashDim;
            dgrad[2].position = {dfx + dfw, dfy + dfh}; dgrad[2].color = dashReady ? uiStyle.hudDashBottom : uiStyle.hudDashDim;
            dgrad[3].position = {dfx, dfy + dfh};      dgrad[3].color = dashReady ? uiStyle.hudDashBottom : uiStyle.hudDashDim;
            window.draw(dgrad, 4, sf::PrimitiveType::TriangleStrip);
        }

        // Ready flash: brief bright fill + soft halo when the gauge refills.
        if (dashReadyFlash > 0.f)
        {
            float gp = std::clamp(dashReadyFlash / uiStyle.hudDashReadyFlashTime, 0.f, 1.f);
            sf::RectangleShape glowFill(dashFillArea.size);
            glowFill.setPosition(dashFillArea.position);
            glowFill.setFillColor(sf::Color(uiStyle.hudDashReadyGlow.r, uiStyle.hudDashReadyGlow.g,
                                            uiStyle.hudDashReadyGlow.b, static_cast<std::uint8_t>(200.f * gp)));
            window.draw(glowFill);

            sf::RectangleShape glowBorder({dashBar.size.x + 10.f, dashBar.size.y + 10.f});
            glowBorder.setOrigin({glowBorder.getSize().x / 2.f, glowBorder.getSize().y / 2.f});
            glowBorder.setPosition({dashBar.position.x + dashBar.size.x / 2.f, dashBar.position.y + dashBar.size.y / 2.f});
            glowBorder.setFillColor(sf::Color::Transparent);
            glowBorder.setOutlineThickness(2.5f);
            glowBorder.setOutlineColor(sf::Color(uiStyle.hudDashReadyGlow.r, uiStyle.hudDashReadyGlow.g,
                                                 uiStyle.hudDashReadyGlow.b, static_cast<std::uint8_t>(120.f * gp)));
            window.draw(glowBorder);
        }

        if (fontLoaded)
        {
            sf::Text hpText(font, "HEALTH", 12);
            hpText.setFillColor(sf::Color(200, 235, 255));
            hpText.setPosition({45.f, 31.f});
            window.draw(hpText);

            std::string hpStr = "HP " + std::to_string(static_cast<int>(std::round(health))) + " / " + std::to_string(static_cast<int>(std::round(maxHealth)));
            sf::Text hpNumText(font, hpStr, 11);
            hpNumText.setFillColor(sf::Color(255, 255, 255));
            hpNumText.setOutlineColor(sf::Color(0, 0, 0, 220));
            hpNumText.setOutlineThickness(1.2f);
            sf::FloatRect hpb = hpNumText.getLocalBounds();
            hpNumText.setOrigin({hpb.position.x + hpb.size.x / 2.f, hpb.position.y + hpb.size.y / 2.f});
            hpNumText.setPosition({150.f, 52.f});
            window.draw(hpNumText);

            bool healingNow = (healBlockTimer <= 0.f && health < maxHealth && !gameOver && !victory);
            std::string healLabel = "+" + std::to_string(static_cast<int>(balance.healPerSecond)) + "HP/s";
            sf::Text healText(font, healLabel, 10);
            healText.setFillColor(healingNow ? sf::Color(100, 255, 160) : sf::Color(90, 120, 110));
            sf::FloatRect hlB = healText.getLocalBounds();
            healText.setOrigin({hlB.position.x + hlB.size.x, 0.f});
            healText.setPosition({270.f, 32.f});
            window.draw(healText);

            sf::Text dashLabel(font, "DASH", 10);
            dashLabel.setFillColor(dashReady ? sf::Color(140, 240, 255) : sf::Color(150, 170, 190));
            dashLabel.setPosition({45.f, 68.f});
            window.draw(dashLabel);

            sf::Text dashStatus(font, dashReady ? "[SHIFT] READY" : "CHARGING...", 10);
            dashStatus.setFillColor(dashReady ? sf::Color(140, 240, 255) : sf::Color(150, 170, 190));
            sf::FloatRect dsB = dashStatus.getLocalBounds();
            dashStatus.setOrigin({dsB.position.x + dsB.size.x, 0.f});
            dashStatus.setPosition({270.f, 68.f});
            window.draw(dashStatus);

            // Location Badge
            drawHudPanel(sf::FloatRect({345.f, 25.f}, {410.f, 32.f}));

            std::string realmName = (currentRealm == RealmLocation::ForestValley) ? "REALM: MYSTIC FOREST VALLEY" : "REALM: CRAGGY MOUNTAIN PEAKS";
            sf::Text realmText(font, realmName, 12);
            realmText.setFillColor((currentRealm == RealmLocation::ForestValley) ? sf::Color(100, 255, 190) : sf::Color(210, 180, 255));
            sf::FloatRect rb = realmText.getLocalBounds();
            realmText.setOrigin({rb.position.x + rb.size.x / 2.f, rb.position.y + rb.size.y / 2.f});
            realmText.setPosition({550.f, 40.f});
            window.draw(realmText);

            // Hero Profile Badge
            drawHudPanel(sf::FloatRect({345.f, 62.f}, {410.f, 32.f}));

            sf::Text heroText(font, "HERO: " + playerName + " [KNIGHT]", 12);
            heroText.setFillColor(sf::Color(255, 225, 120));
            sf::FloatRect hb = heroText.getLocalBounds();
            heroText.setOrigin({hb.position.x + hb.size.x / 2.f, hb.position.y + hb.size.y / 2.f});
            heroText.setPosition({550.f, 77.f});
            window.draw(heroText);

            // Stats Card (Score with count-up, High, COMBO, Crystals, Portal)
            drawHudPanel(sf::FloatRect({805.f, 25.f}, {270.f, 100.f}));

            {
                int shownScore = static_cast<int>(std::round(scoreDisplay));
                sf::Text scoreText(font, "Score: " + std::to_string(shownScore), 15);
                scoreText.setFillColor(sf::Color(255, 222, 95));
                scoreText.setPosition({823.f, 31.f});
                window.draw(scoreText);

                // COMBO sits beside the score, right-aligned to the panel.
                if (comboCount >= 2 && comboTimer > 0.f)
                {
                    int cm = static_cast<int>(std::round(comboMultiplier * 10.f));
                    std::string comboStr = "COMBO x" + std::to_string(cm / 10) + "." + std::to_string(cm % 10);
                    sf::Text comboText(font, comboStr, 13);
                    comboText.setFillColor(sf::Color(255, 170, 60));
                    sf::FloatRect cb = comboText.getLocalBounds();
                    comboText.setOrigin({cb.position.x + cb.size.x, cb.position.y + cb.size.y / 2.f});
                    comboText.setPosition({1055.f, 43.f});
                    window.draw(comboText);
                }

                sf::Text highText(font, "High: " + std::to_string(highScore), 11);
                highText.setFillColor(sf::Color(200, 215, 230));
                highText.setPosition({823.f, 54.f});
                window.draw(highText);

                sf::Text crystalText(font, "Crystals: " + std::to_string(collectedCount) + "/" + std::to_string(crystals.size()), 12);
                crystalText.setFillColor((currentRealm == RealmLocation::MountainPeaks) ? sf::Color(210, 180, 255) : sf::Color(100, 240, 255));
                crystalText.setPosition({823.f, 73.f});
                window.draw(crystalText);

                sf::Text portalText(font, "[PORTAL] Step In to Teleport", 10);
                portalText.setFillColor(sf::Color(140, 160, 180));
                portalText.setPosition({823.f, 92.f});
                window.draw(portalText);
            }

            // Controls Prompt
            sf::Text helpText(font, "[SPACE] Attack  |  [SHIFT] Dash Roll  |  [PORTAL] Realms  |  [F3] Debug  |  [H] Dashboard  |  [L] Logout", 13);
            helpText.setFillColor(sf::Color(180, 210, 225, 180));
            sf::FloatRect htb = helpText.getLocalBounds();
            helpText.setOrigin({htb.position.x + htb.size.x / 2.f, htb.position.y + htb.size.y / 2.f});
            helpText.setPosition({550.f, 725.f});
            window.draw(helpText);

            // First-run control hints (banner, bottom center)
            if (hintsActive && hintIndex < 4 && gameSettings.showHints == 1 &&
                gameState == GameState::Playing && !gameOver && !victory &&
                !paused && !showHistoryDashboard && runTime <= balance.hintsRunWindow)
            {
                float alphaT = std::min({1.f, hintTimer / balance.hintFadeTime,
                                         (balance.hintInterval - hintTimer) / balance.hintFadeTime});
                std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(alphaT, 0.f, 1.f) * 255.f);
                if (alpha > 0)
                {
                    sf::FloatRect hintRect({300.f, 660.f}, {500.f, 34.f});
                    drawRoundedRect(sf::FloatRect({hintRect.position.x + 3.f, hintRect.position.y + 3.f}, hintRect.size),
                                    6.f, sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha * 0.55f)));
                    drawRoundedRect(hintRect, 6.f, sf::Color(212, 175, 90, alpha));
                    sf::FloatRect hintInner({hintRect.position.x + 3.f, hintRect.position.y + 3.f},
                                            {hintRect.size.x - 6.f, hintRect.size.y - 6.f});
                    drawRoundedRect(hintInner, 5.f, sf::Color(13, 20, 34, static_cast<std::uint8_t>(alpha * 0.97f)));

                    sf::Text hintText(font, hintTexts[hintIndex], 18);
                    hintText.setFillColor(sf::Color(255, 236, 150, alpha));
                    sf::FloatRect hhb = hintText.getLocalBounds();
                    hintText.setOrigin({hhb.position.x + hhb.size.x / 2.f, hhb.position.y + hhb.size.y / 2.f});
                    hintText.setPosition({550.f, 678.f});
                    window.draw(hintText);
                }
            }
        }

        // Toggleable Debug Overlay (F3)
        if (showDebugOverlay && fontLoaded)
        {
            float damageLast10s = 0.f;
            for (const auto& dh : damageHistory)
            {
                if (dh.amount > 0.f && (totalTime - dh.timestamp) <= 10.f && totalTime >= dh.timestamp)
                {
                    damageLast10s += dh.amount;
                }
            }

            sf::RectangleShape dbgCard({260.f, 96.f});
            dbgCard.setPosition({25.f, 108.f});
            dbgCard.setFillColor(sf::Color(10, 18, 28, 235));
            dbgCard.setOutlineThickness(2.f);
            dbgCard.setOutlineColor(sf::Color(255, 195, 50, 220));
            window.draw(dbgCard);

            std::ostringstream ss;
            ss << "DEBUG OVERLAY [F3]\n"
               << "FPS: " << static_cast<int>(std::round(smoothedFps)) << "\n"
               << "Player HP: " << static_cast<int>(std::round(health)) << " / " << static_cast<int>(std::round(maxHealth)) << "\n"
               << "Dash Cooldown: " << std::fixed << std::setprecision(2) << std::max(0.f, dashCooldown) << " s\n"
               << "Dmg Taken (10s): " << static_cast<int>(std::round(damageLast10s));

            sf::Text dbgText(font, ss.str(), 11);
            dbgText.setFillColor(sf::Color(255, 235, 170));
            dbgText.setPosition({35.f, 114.f});
            window.draw(dbgText);
        }

        // Boss phase-change text flash
        if (fontLoaded)
        {
            for (const auto& enemy : enemies)
            {
                if (enemy.type == EnemyType::Boss && enemy.isAlive && enemy.realm == currentRealm && enemy.phaseTextTimer > 0.f)
                {
                    float a = std::clamp(enemy.phaseTextTimer / balance.bossPhaseTextTime, 0.f, 1.f);
                    sf::Text phaseText(font, "PHASE " + std::to_string(enemy.phase), 48);
                    phaseText.setFillColor(sf::Color(255, 90, 80, static_cast<std::uint8_t>(a * 255.f)));
                    phaseText.setOutlineColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(a * 200.f)));
                    phaseText.setOutlineThickness(3.f);
                    sf::FloatRect pb = phaseText.getLocalBounds();
                    phaseText.setOrigin({pb.position.x + pb.size.x / 2.f, pb.position.y + pb.size.y / 2.f});
                    phaseText.setPosition({550.f, 300.f});
                    window.draw(phaseText);
                    break;
                }
            }
        }

        // VICTORY / GAME OVER OVERLAY
        if (gameOver || victory)
        {
            sf::RectangleShape modalDim({1100.f, 750.f});
            modalDim.setFillColor(sf::Color(8, 14, 20, 220));
            window.draw(modalDim);

            sf::RectangleShape panel({560.f, 460.f});
            panel.setPosition({270.f, 165.f});

            if (victory)
            {
                panel.setFillColor(sf::Color(18, 55, 42, 230));
                panel.setOutlineColor(sf::Color(80, 255, 180));
            }
            else
            {
                panel.setFillColor(sf::Color(55, 18, 28, 230));
                panel.setOutlineColor(sf::Color(255, 75, 90));
            }
            panel.setOutlineThickness(4.f);
            window.draw(panel);

            if (fontLoaded)
            {
                sf::Text title(font, victory ? "EPIC VICTORY ACHIEVED!" : "YOU HAVE FALLEN", 36);
                title.setFillColor(victory ? sf::Color(110, 255, 190) : sf::Color(255, 95, 110));
                sf::FloatRect b = title.getLocalBounds();
                title.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
                title.setPosition({550.f, 235.f});
                window.draw(title);

                if (endIsNewHigh)
                {
                    sf::Text recordText(font, "NEW HIGH SCORE!", 20);
                    recordText.setFillColor(sf::Color(255, 230, 120));
                    sf::FloatRect rb2 = recordText.getLocalBounds();
                    recordText.setOrigin({rb2.position.x + rb2.size.x / 2.f, rb2.position.y + rb2.size.y / 2.f});
                    recordText.setPosition({550.f, 276.f});
                    window.draw(recordText);
                }

                sf::Text bdHeader(font, "SCORE BREAKDOWN", 13);
                bdHeader.setFillColor(sf::Color(212, 175, 90));
                sf::FloatRect hb2 = bdHeader.getLocalBounds();
                bdHeader.setOrigin({hb2.position.x + hb2.size.x / 2.f, hb2.position.y + hb2.size.y / 2.f});
                bdHeader.setPosition({550.f, 305.f});
                window.draw(bdHeader);

                // Reveal animation: one line every 0.3 s, then the total counts up over 0.6 s.
                constexpr float endLineInterval = 0.3f;
                constexpr float endTotalDuration = 0.6f;
                constexpr int endLineCount = 7;

                int minutes = static_cast<int>(runTime) / 60;
                int seconds = static_cast<int>(runTime) % 60;
                std::string timeLabel = "Time Bonus (" + std::to_string(minutes) + ":" +
                                        (seconds < 10 ? "0" : "") + std::to_string(seconds) + ")";

                const char* endLabels[endLineCount] = {
                    "Crystals",
                    "Enemy Kills",
                    "Combo Bonus",
                    "Perfect Dodges",
                    "Victory Bonus",
                    "No-Damage Bonus",
                    timeLabel.c_str(),
                };
                const int endValues[endLineCount] = {
                    breakdownCrystals,
                    breakdownKills,
                    breakdownCombo,
                    breakdownDodgePts,
                    breakdownVictory,
                    breakdownNoDamage,
                    breakdownTime,
                };

                constexpr float startY = 330.f;
                for (int i = 0; i < endLineCount; ++i)
                {
                    if (endAnimTime < i * endLineInterval) break;

                    bool earned = endValues[i] > 0;
                    sf::Color lineCol = earned ? sf::Color(230, 245, 255) : sf::Color(125, 135, 145);
                    sf::Text labelText(font, endLabels[i], 17);
                    labelText.setFillColor(lineCol);
                    labelText.setPosition({305.f, startY + i * 30.f});
                    window.draw(labelText);

                    std::string valStr = earned ? "+" + std::to_string(endValues[i]) : "0";
                    sf::Text valText(font, valStr, 17);
                    valText.setFillColor(lineCol);
                    sf::FloatRect vb = valText.getLocalBounds();
                    valText.setOrigin({vb.position.x + vb.size.x, 0.f});
                    valText.setPosition({805.f, startY + i * 30.f});
                    window.draw(valText);
                }

                // TOTAL appears once all lines are in; its value counts up quickly.
                if (endAnimTime >= endLineCount * endLineInterval)
                {
                    float p = std::clamp((endAnimTime - endLineCount * endLineInterval) / endTotalDuration, 0.f, 1.f);
                    int shownTotal = static_cast<int>(std::floor(p * static_cast<float>(score)));

                    sf::Text totalText(font, "TOTAL", 24);
                    totalText.setFillColor(sf::Color(255, 222, 95));
                    totalText.setPosition({305.f, 550.f});
                    window.draw(totalText);

                    sf::Text totalVal(font, std::to_string(shownTotal), 24);
                    totalVal.setFillColor(sf::Color(255, 222, 95));
                    sf::FloatRect tvb = totalVal.getLocalBounds();
                    totalVal.setOrigin({tvb.position.x + tvb.size.x, 0.f});
                    totalVal.setPosition({805.f, 550.f});
                    window.draw(totalVal);
                }

                sf::Text prompt(font, "[R] Play Again   |   [H] Dashboard   |   [L] Login   |   [ENTER]/[SPACE] Skip", 13);
                prompt.setFillColor(sf::Color(210, 225, 235));
                sf::FloatRect pb = prompt.getLocalBounds();
                prompt.setOrigin({pb.position.x + pb.size.x / 2.f, pb.position.y + pb.size.y / 2.f});
                prompt.setPosition({550.f, 600.f});
                window.draw(prompt);
            }
        }

        if (showHistoryDashboard)
        {
            drawHistoryDashboard(window, font, fontLoaded, matchHistory, highScore, totalTime);
        }

        // PAUSE MENU OVERLAY
        if (paused && fontLoaded)
        {
            sf::RectangleShape pauseDim({1100.f, 750.f});
            pauseDim.setFillColor(sf::Color(8, 12, 20, 205));
            window.draw(pauseDim);

            const sf::FloatRect panel({330.f, 130.f}, {440.f, 520.f});
            drawRoundedRect(panel, uiStyle.hudPanelCorner, uiStyle.hudPanelBorder);
            sf::FloatRect panelInner({panel.position.x + 3.f, panel.position.y + 3.f},
                                     {panel.size.x - 6.f, panel.size.y - 6.f});
            drawRoundedRect(panelInner, uiStyle.hudPanelCorner - 1.f, sf::Color(13, 20, 34, 250));

            sf::Text title(font, "PAUSED", 32);
            title.setFillColor(sf::Color(255, 222, 95));
            sf::FloatRect tb = title.getLocalBounds();
            title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
            title.setPosition({550.f, 168.f});
            window.draw(title);

            std::string shakeStr = (gameSettings.screenShakeMode == 0) ? "On"
                                 : (gameSettings.screenShakeMode == 1) ? "Reduced" : "Off";

            const std::string labels[5] = {
                "Resume",
                "Volume: " + std::to_string(gameSettings.volume),
                "Screen Shake: " + shakeStr,
                std::string("Show Hints: ") + (gameSettings.showHints == 1 ? "On" : "Off"),
                "Quit to Login"
            };

            const float rowX = 360.f;
            const float rowY[5] = {235.f, 295.f, 355.f, 415.f, 475.f};

            for (int i = 0; i < 5; ++i)
            {
                bool sel = (i == pauseSelection);

                if (sel)
                {
                    sf::FloatRect hi({rowX - 16.f, rowY[i] - 22.f}, {368.f, 42.f});
                    drawRoundedRect(hi, 6.f, sf::Color(212, 175, 90, 55));
                }

                sf::Text item(font, labels[i], 24);
                item.setFillColor(sel ? sf::Color(255, 236, 150) : sf::Color(205, 220, 235));
                item.setPosition({rowX, rowY[i]});
                window.draw(item);

                if (sel)
                {
                    sf::Text marker(font, ">", 24);
                    marker.setFillColor(sf::Color(255, 222, 95));
                    marker.setPosition({rowX - 30.f, rowY[i]});
                    window.draw(marker);
                }
            }

            // Master volume slider (track, gold fill, knob)
            {
                const float sx = rowX;
                const float sy = 327.f;
                const float sw = 340.f;
                sf::RectangleShape track({sw, 8.f});
                track.setPosition({sx, sy});
                track.setFillColor(sf::Color(60, 74, 92, 220));
                window.draw(track);

                float frac = static_cast<float>(gameSettings.volume) / 100.f;
                sf::RectangleShape fill({sw * frac, 8.f});
                fill.setPosition({sx, sy});
                fill.setFillColor(sf::Color(212, 175, 90));
                window.draw(fill);

                sf::CircleShape knob(6.f, 16);
                knob.setOrigin({6.f, 6.f});
                knob.setPosition({sx + sw * frac, sy + 4.f});
                knob.setFillColor(sf::Color(255, 236, 150));
                window.draw(knob);
            }

            sf::Text hint(font, "Up/Down or W/S: navigate    Left/Right: adjust    Enter: confirm    Esc: resume", 12);
            hint.setFillColor(sf::Color(150, 170, 190));
            sf::FloatRect hb = hint.getLocalBounds();
            hint.setOrigin({hb.position.x + hb.size.x / 2.f, hb.position.y + hb.size.y / 2.f});
            hint.setPosition({550.f, 625.f});
            window.draw(hint);
        }

        window.display();
    }

    bgmSound.stop();
    window.close();
    std::exit(0);
    return 0;
}
