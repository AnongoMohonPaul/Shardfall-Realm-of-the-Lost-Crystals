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
enum class GameState { HomeMenu, LoginScreen, Playing, Settings, HowToPlay };

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

    // Hero drawing/animation (visuals only, no gameplay effect)
    float heroSwingAngle = 32.f;      // limb rotation in degrees at full walk
    float heroWalkSpeedFactor = 11.f; // walk-phase advance per second while moving
    float heroWalkEase = 10.f;        // how fast the walk pose eases in/out
    float heroBreathAmount = 1.6f;    // px torso rise/fall while standing still
    float heroBreathRate = 2.4f;      // idle breathing speed in rad/s

    // Regular-enemy drawing/animation (visuals only, no gameplay effect)
    float enemySwingAngle = 26.f;       // limb swing in degrees at full walk
    float enemyWalkSpeedFactor = 0.08f; // walk-phase radians per pixel travelled
    float enemyWalkEase = 8.f;          // how fast the walk pose eases in/out
    float enemyBreathAmount = 1.4f;     // px idle breathing / hover
    float enemyBreathRate = 2.6f;       // idle breathing speed in rad/s

    // Hero sword-attack animation (visuals only; follows the existing attack timer)
    float heroAttackAnimDuration = 0.22f; // matches attackTimer set in performAttack()
    float heroAttackAnticFrac = 0.30f;    // anticipation ends at this fraction (0..1)
    float heroAttackStrikeFrac = 0.62f;   // strike ends / recovery begins at this fraction
    float heroAnticipationAngle = 38.f;   // pull-back angle before the strike (deg)
    float heroStrikeAngle = 62.f;         // swing-through angle past neutral (deg)
    float heroLeanAmount = 0.18f;         // torso lean/twist per swing degree
    float heroAttackPlant = 0.7f;         // how much the legs plant during a swing
    int heroTrailLength = 7;              // max sword-trail samples drawn
    float heroTrailDuration = 0.16f;      // seconds a trail sample persists

    // Giant Fireball Hazard (Craggy Mountain Peaks only)
    float fireballMinInterval = 7.f;      // random interval range between events (s)
    float fireballMaxInterval = 14.f;     // random interval range between events (s)
    float fireballDoubleChance = 0.15f;   // chance an event launches a pair of fireballs
    float fireballDoubleGap = 0.6f;       // launch gap between the two fireballs (s)
    float fireballGraceTime = 5.f;        // no fireballs for this long after entering the realm
    float fireballWarningTime = 0.8f;     // telegraph shown before each launch (s)
    float fireballSpeed = 480.f;          // travel speed (px/s)
    float fireballSize = 20.f;            // visual radius of the ball (px)
    float fireballPlayerDamage = 30.f;    // large hit vs. normal enemy damage
    float fireballEnemyDamage = 3.f;      // regular-enemy damage on touch
    float fireballBossDamage = 0.f;       // boss takes a small amount or none
    float fireballShakeAmount = 10.f;     // launch shake intensity

    // Enemy Huts (solid world obstacles in both realms)
    float hutHalfWidth = 22.f;        // half-width of a hut footprint used for drawing (px)
    float hutHalfHeight = 15.f;       // half-height of a hut footprint used for drawing (px)
    float hutCollisionRadius = 20.f;  // collision radius (smaller than the sprite, feels fair)
    int hutsPerRealm = 3;             // fixed number of hand-placed huts per realm

    // Wizard Enemies (spawned by huts over time; ranged attack comes later)
    float wizardSpawnMinInterval = 4.f; // hut spawn interval random min (s)
    float wizardSpawnMaxInterval = 5.f; // hut spawn interval random max (s)
    float wizardSpawnGap = 1.2f;        // minimum gap between any two wizard spawns (s)
    float wizardStartOffsetMax = 2.f;   // random initial delay per hut (s)
    float wizardGraceTime = 3.f;        // no spawns for this long after entering a realm
    int wizardMaxAlive = 8;             // max wizards alive at once (pool/cap)
    float wizardHealth = 2.f;           // low: one or two sword hits kill it
    float wizardSpeed = 105.f;          // walk speed toward the player (px/s)
    float wizardPreferredDist = 260.f;  // stops and faces the player at this distance
    float wizardMinDist = 140.f;        // backs away slowly when the player comes closer
    float wizardEmergeTime = 0.5f;      // fade/scale-in at the door (can't be hit/move)
    int wizardScoreValue = 5;           // flat score per kill (no combo contribution)
    float wizardFadeTime = 0.35f;       // victory fade-out duration (s)

    // Wizard ranged attack (mini fireballs)
    float wizardAttackMinCd = 2.5f;     // min random delay between a wizard's shots (s)
    float wizardAttackMaxCd = 4.0f;     // max random delay between a wizard's shots (s)
    float wizardTelegraph = 0.5f;       // wind-up before the shot is fired (s)
    int wizardMaxShooting = 3;          // max wizards charging a shot at any moment
    float wizardShotGap = 0.8f;         // min gap between any two mini fireball launches (s)
    float miniFireballSpeed = 300.f;    // travel speed (px/s)
    float miniFireballSize = 5.f;       // visual radius of the mini ball (px)
    float miniFireballDamage = 8.f;     // damage on player hit
    float miniFireballShake = 4.f;      // screen shake on player hit
    bool miniBlockOnObstacles = true;   // mini fireballs burst on huts/trees/etc.
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

inline sf::Color colorLerp(const sf::Color& a, const sf::Color& b, float t)
{
    t = std::clamp(t, 0.f, 1.f);
    return sf::Color(
        static_cast<std::uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<std::uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<std::uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<std::uint8_t>(a.a + (b.a - a.a) * t));
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

// Giant Fireball Hazard (Mountain Peaks) - fixed pool
enum class FireballState { Inactive, Warn, Flying };

struct Fireball
{
    FireballState state = FireballState::Inactive;
    int id = -1;                    // unique id so each fireball hits each enemy at most once
    sf::Vector2f pos{0.f, 0.f};
    sf::Vector2f dir{1.f, 0.f};
    sf::Vector2f aimPos{0.f, 0.f};  // player position the ball was aimed at (telegraph target)
    float warnDelay = 0.f;          // seconds before this ball's telegraph begins
    float warnTimer = 0.f;          // counts down once the telegraph has started
    float warnTime = 0.8f;          // total telegraph duration
    float speed = 480.f;
    float emberAccum = 0.f;         // trail ember spawn accumulator
    bool perfectDodged = false;     // avoids repeated perfect-dodge awards from one ball
    bool mini = false;              // true = wizard mini fireball (shares this pool)
};

// Fixed pool for fireball embers / sparks / smoke puffs
struct FireParticle
{
    bool active = false;
    int kind = 0;                   // 0 = ember/spark, 1 = smoke puff
    sf::Vector2f pos{0.f, 0.f};
    sf::Vector2f vel{0.f, 0.f};
    sf::Color color{255, 200, 80, 255};
    float size = 3.f;
    float life = 0.f;
    float maxLife = 0.5f;
};

// Expanding shock ring for the fireball launch flash / impact explosion
struct ExplosionRing
{
    bool active = false;
    sf::Vector2f pos{0.f, 0.f};
    float radius = 10.f;
    float maxRadius = 100.f;
    float life = 0.f;
    float maxLife = 0.5f;
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
    float animPhase = 0.f;               // visual walk-cycle phase (radians)
    float animMove = 0.f;                // 0..1 eased walk-pose blend (visual only)
    int fireballDamageId = -1;           // id of the last fireball that damaged this enemy
};

// Wizard Enemy (spawned by huts; ranged attack comes in a later step)
struct Wizard
{
    bool active = false;
    RealmLocation realm = RealmLocation::ForestValley;
    sf::Vector2f pos{0.f, 0.f};
    sf::Vector2f facing{0.f, 1.f};
    float health = 0.f;
    float maxHealth = 0.f;
    float speed = 0.f;
    float hitTimer = 0.f;
    float emergeTimer = 0.f; // >0: fading/scaling in at the door; can't be hit or move
    bool fading = false;     // victory fade-out (no score)
    float fadeTimer = 0.f;
    float animPhase = 0.f;   // visual walk-bob phase
    float animMove = 0.f;    // 0..1 eased walk-pose blend (visual only)
    float attackTimer = 0.f;     // counts down to the next shot attempt
    float telegraphTimer = 0.f;  // >0: charging a shot (holds position)
    sf::Vector2f aimDir{0.f, 1.f}; // direction the current shot will fly
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

// Enemy Hut (solid world obstacle in both realms)
struct Hut
{
    sf::Vector2f pos;     // ground-center of the hut
    float size;           // half-width used for drawing scale and the collision footprint
    RealmLocation realm;
    sf::Vector2f doorPos; // where a character could later come out (data only for now)
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
    const std::vector<Boulder>& mountainCrags,
    const std::vector<Hut>& huts)
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

    // Enemy Huts are solid world obstacles in both realms.
    for (const auto& h : huts)
    {
        if (h.realm != realm) continue;
        float dx = pos.x - h.pos.x;
        float dy = pos.y - h.pos.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float minDist = balance.hutCollisionRadius + bodyRadius;
        if (dist < minDist && dist > 0.f)
            pos = h.pos + sf::Vector2f(dx / dist * minDist, dy / dist * minDist);
    }

    pos.x = std::clamp(pos.x, 30.f, 1070.f);
    pos.y = std::clamp(pos.y, 30.f, 720.f);
    return pos;
}

// Returns true if a moving circle overlaps any solid obstacle in the given
// realm (boulders, trees, crags or huts). Used so mini fireballs burst on
// impact. No allocations.
bool circleHitsObstacle(
    sf::Vector2f pos,
    float radius,
    RealmLocation realm,
    const std::vector<Boulder>& forestBoulders,
    const std::vector<Tree>& forestTrees,
    const std::vector<Boulder>& mountainCrags,
    const std::vector<Hut>& huts)
{
    if (realm == RealmLocation::ForestValley)
    {
        for (const auto& b : forestBoulders)
        {
            float dx = pos.x - b.pos.x;
            float dy = pos.y - b.pos.y;
            float m = b.radius + radius;
            if (dx * dx + dy * dy < m * m) return true;
        }
        for (const auto& t : forestTrees)
        {
            float dx = pos.x - t.pos.x;
            float dy = pos.y - (t.pos.y + 20.f);
            float m = 22.f * t.scale + radius;
            if (m < 2.f) m = 2.f;
            if (dx * dx + dy * dy < m * m) return true;
        }
    }
    else
    {
        for (const auto& c : mountainCrags)
        {
            float dx = pos.x - c.pos.x;
            float dy = pos.y - c.pos.y;
            float m = c.radius + radius;
            if (dx * dx + dy * dy < m * m) return true;
        }
    }

    for (const auto& h : huts)
    {
        if (h.realm != realm) continue;
        float dx = pos.x - h.pos.x;
        float dy = pos.y - h.pos.y;
        float m = balance.hutCollisionRadius + radius;
        if (dx * dx + dy * dy < m * m) return true;
    }
    return false;
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
    const std::vector<Boulder>& mountainCrags,
    const std::vector<Hut>& huts)
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
                                                 forestBoulders, forestTrees, mountainCrags, huts);
    sf::Vector2f best = full;
    float bestProgress = (full.x - start.x) * desired.x + (full.y - start.y) * desired.y;

    // If the full move was adjusted by a collision, try axis-only slides.
    float resX = full.x - (start.x + step.x);
    float resY = full.y - (start.y + step.y);
    if (resX * resX + resY * resY > 0.0001f)
    {
        sf::Vector2f xCand = resolveObstacleCollision(start + sf::Vector2f(step.x, 0.f), bodyRadius, realm,
                                                      forestBoulders, forestTrees, mountainCrags, huts);
        sf::Vector2f yCand = resolveObstacleCollision(start + sf::Vector2f(0.f, step.y), bodyRadius, realm,
                                                      forestBoulders, forestTrees, mountainCrags, huts);

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
    sf::SoundBuffer& warnBuf,
    sf::SoundBuffer& fbWarnBuf,
    sf::SoundBuffer& fbWhooshBuf,
    sf::SoundBuffer& fbBoomBuf,
    sf::SoundBuffer& miniWarnBuf,
    sf::SoundBuffer& miniShotBuf
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

    // 9. Fireball Warning (low rumbling tone that rises in pitch)
    {
        const std::uint32_t n = (std::uint32_t)(sampleRate * 0.8f);
        std::vector<std::int16_t> s(n);
        for (std::uint32_t i = 0; i < n; ++i) {
            float t = (float)i / sampleRate;
            float freq = 72.f + t * 130.f;
            float noise = ((rand() % 200 - 100) / 100.f);
            float v = std::sin(2.f * 3.14159f * freq * t) * 0.55f + noise * 0.45f;
            float env = 0.35f + 0.65f * (float)i / n;
            s[i] = static_cast<std::int16_t>(v * env * 16000.f);
        }
        (void)fbWarnBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 10. Fireball Launch Whoosh (roaring filtered noise with a pitch sweep)
    {
        const std::uint32_t n = (std::uint32_t)(sampleRate * 0.45f);
        std::vector<std::int16_t> s(n);
        for (std::uint32_t i = 0; i < n; ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin((float)i / n * 3.14159f);
            float noise = ((rand() % 200 - 100) / 100.f);
            float freq = 300.f + t * 1600.f;
            float v = std::sin(2.f * 3.14159f * freq * t) * 0.35f + noise * 0.60f;
            s[i] = static_cast<std::int16_t>(v * env * 20000.f);
        }
        (void)fbWhooshBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 11. Fireball Impact (deep boom with a noise tail)
    {
        const std::uint32_t n = (std::uint32_t)(sampleRate * 1.0f);
        std::vector<std::int16_t> s(n);
        for (std::uint32_t i = 0; i < n; ++i) {
            float t = (float)i / sampleRate;
            float env = std::exp(-t * 3.5f);
            float noise = ((rand() % 200 - 100) / 100.f) * std::exp(-t * 12.f);
            float v = std::sin(2.f * 3.14159f * (70.f - t * 30.f) * t) * 0.8f + noise * 0.5f;
            s[i] = static_cast<std::int16_t>(v * env * 26000.f);
        }
        (void)fbBoomBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 12. Wizard mini-fireball Warning (short, soft magical chirp)
    {
        const std::uint32_t n = (std::uint32_t)(sampleRate * 0.3f);
        std::vector<std::int16_t> s(n);
        for (std::uint32_t i = 0; i < n; ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin((float)i / n * 3.14159f);
            float freq = 680.f + std::sin(t * 40.f) * 90.f;
            float v = std::sin(2.f * 3.14159f * freq * t) * 0.7f
                    + std::sin(2.f * 3.14159f * freq * 1.5f * t) * 0.3f;
            s[i] = static_cast<std::int16_t>(v * env * 11000.f);
        }
        (void)miniWarnBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
    }

    // 13. Wizard mini-fireball Launch (soft, short whoosh)
    {
        const std::uint32_t n = (std::uint32_t)(sampleRate * 0.4f);
        std::vector<std::int16_t> s(n);
        for (std::uint32_t i = 0; i < n; ++i) {
            float t = (float)i / sampleRate;
            float env = std::sin((float)i / n * 3.14159f);
            float noise = ((rand() % 200 - 100) / 100.f);
            float freq = 260.f + t * 900.f;
            float v = std::sin(2.f * 3.14159f * freq * t) * 0.30f + noise * 0.45f;
            s[i] = static_cast<std::int16_t>(v * env * 13000.f);
        }
        (void)miniShotBuf.loadFromSamples(s.data(), s.size(), 1, sampleRate, { sf::SoundChannel::Mono });
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
    sf::SoundBuffer swingBuf, hitBuf, crystalBuf, dashBuf, slamBuf, teleportBuf, bgmBuf, warnBuf,
                     fbWarnBuf, fbWhooshBuf, fbBoomBuf, miniWarnBuf, miniShotBuf;
    generateProceduralAudio(swingBuf, hitBuf, crystalBuf, dashBuf, slamBuf, teleportBuf, bgmBuf, warnBuf,
                            fbWarnBuf, fbWhooshBuf, fbBoomBuf, miniWarnBuf, miniShotBuf);

    sf::Sound swingSound(swingBuf);
    sf::Sound hitSound(hitBuf);
    sf::Sound crystalSound(crystalBuf);
    sf::Sound dashSound(dashBuf);
    sf::Sound slamSound(slamBuf);
    sf::Sound teleportSound(teleportBuf);
    sf::Sound warnSound(warnBuf);
    warnSound.setVolume(55.f);
    sf::Sound fbWarnSound(fbWarnBuf);
    sf::Sound fbWhooshSound(fbWhooshBuf);
    sf::Sound fbBoomSound(fbBoomBuf);
    sf::Sound miniWarnSound(miniWarnBuf);
    sf::Sound miniShotSound(miniShotBuf);
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
        fbWarnSound.setVolume(0.55f * v);
        fbWhooshSound.setVolume(0.8f * v);
        fbBoomSound.setVolume(v);
        miniWarnSound.setVolume(0.5f * v);
        miniShotSound.setVolume(0.55f * v);
        bgmSound.setVolume(v);
    };
    applyVolume();

    sf::Clock clock;
    float totalTime = 0.f;

    // Game State & User Profile
    GameState gameState = GameState::HomeMenu;
    std::string playerName = (!matchHistory.empty() && !matchHistory.back().heroName.empty())
                             ? matchHistory.back().heroName : "Hero";
    int homeSelection = 0;

    // Home menu button geometry (5 stacked buttons) and shared nav helpers.
    auto homeButtonRect = [](int i) {
        return sf::FloatRect({380.f, 300.f + static_cast<float>(i) * 58.f}, {340.f, 46.f});
    };
    auto placeholderBackRect = []() {
        return sf::FloatRect({380.f, 470.f}, {340.f, 50.f});
    };
    auto confirmHomeSelection = [&]() {
        if (homeSelection == 0)
        {
            gameState = GameState::LoginScreen; // Play -> existing hero-name login
        }
        else if (homeSelection == 1)
        {
            showHistoryDashboard = true; // Hall of Heroes
        }
        else if (homeSelection == 2)
        {
            gameState = GameState::Settings;
        }
        else if (homeSelection == 3)
        {
            gameState = GameState::HowToPlay;
        }
        else if (homeSelection == 4)
        {
            bgmSound.stop();
            window.close();
            std::exit(0); // Quit
        }
    };

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

    // Hero visual animation state (kept across frames; visuals only)
    float heroWalkPhase = 0.f;   // advances with movement; drives limb swing (radians)
    float heroWalkAmount = 0.f;  // 0..1 eased blend of the walk pose

    // Hero sword-trail buffer (fixed size, reused every frame; visuals only)
    constexpr int heroTrailMax = 12;
    std::array<sf::Vector2f, heroTrailMax> heroTrailBase{};
    std::array<sf::Vector2f, heroTrailMax> heroTrailTip{};
    std::array<float, heroTrailMax> heroTrailTime{};
    int heroTrailHead = 0;
    sf::VertexArray heroTrailStrip(sf::PrimitiveType::TriangleStrip, heroTrailMax * 2);
    heroTrailTime.fill(-1.f);

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

    // Enemy Huts (solid world obstacles, fixed hand-picked placements)
    std::vector<Hut> huts = {
        // Mystic Forest Valley: small wooden cottages
        {{170.f, 300.f}, balance.hutHalfWidth, RealmLocation::ForestValley, {170.f, 316.f}},
        {{430.f, 655.f}, balance.hutHalfWidth, RealmLocation::ForestValley, {430.f, 671.f}},
        {{905.f, 205.f}, balance.hutHalfWidth, RealmLocation::ForestValley, {905.f, 221.f}},
        // Craggy Mountain Peaks: stone huts
        {{150.f, 280.f}, balance.hutHalfWidth, RealmLocation::MountainPeaks, {150.f, 296.f}},
        {{950.f, 240.f}, balance.hutHalfWidth, RealmLocation::MountainPeaks, {950.f, 256.f}},
        {{500.f, 560.f}, balance.hutHalfWidth, RealmLocation::MountainPeaks, {500.f, 576.f}}
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

    // Fireball pool (fixed-size; zero per-frame heap allocations). Shared by the
    // giant Mountain hazard and the wizards' mini fireballs.
    static constexpr int MAX_FIREBALLS = 16;
    std::array<Fireball, MAX_FIREBALLS> fireballs{};
    static constexpr int MAX_FIRE_PARTICLES = 64;
    std::array<FireParticle, MAX_FIRE_PARTICLES> fireParticles{};
    static constexpr int MAX_EXPLOSION_RINGS = 8;
    std::array<ExplosionRing, MAX_EXPLOSION_RINGS> explosionRings{};
    int fireballIdCounter = 1;
    float fireballCountdown = balance.fireballGraceTime; // grace period before the first event
    std::uniform_real_distribution<float> fireIntervalDist(balance.fireballMinInterval, balance.fireballMaxInterval);
    std::uniform_real_distribution<float> fireDoubleDist(0.f, 1.f);
    std::uniform_real_distribution<float> fireSpawnXDist(140.f, 960.f);

    // Wizard enemy pool (fixed-size; cap == balance.wizardMaxAlive, max 8)
    static constexpr int MAX_WIZARDS = 8;
    std::array<Wizard, MAX_WIZARDS> wizards{};
    static constexpr int MAX_HUTS = 6; // matches the hand-placed hut list
    std::array<float, MAX_HUTS> wizardHutTimers{};
    std::uniform_real_distribution<float> wizardSpawnIntervalDist(balance.wizardSpawnMinInterval, balance.wizardSpawnMaxInterval);
    std::uniform_real_distribution<float> wizardStartOffsetDist(0.f, balance.wizardStartOffsetMax);
    std::uniform_real_distribution<float> wizardAttackCdDist(balance.wizardAttackMinCd, balance.wizardAttackMaxCd);
    const int wizardCap = std::min<int>(balance.wizardMaxAlive, MAX_WIZARDS);
    float wizardGlobalGapTimer = 0.f;
    float wizardShotGapTimer = 0.f; // min gap between any two mini fireball launches

    // Clears any in-flight mini fireballs (giant Mountain balls are untouched).
    auto clearMiniFireballs = [&]() {
        for (auto& fb : fireballs)
            if (fb.mini) fb.state = FireballState::Inactive;
    };

    auto resetWizardSpawnTimers = [&]() {
        for (auto& t : wizardHutTimers)
            t = balance.wizardGraceTime + wizardStartOffsetDist(rng);
        wizardGlobalGapTimer = 0.f;
        wizardShotGapTimer = 0.f;
    };
    resetWizardSpawnTimers();

    auto activeWizardCount = [&]() {
        int c = 0;
        for (const auto& w : wizards) if (w.active) ++c;
        return c;
    };

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

    // Fixed-pool fire particle (ember/spark or smoke puff); no per-frame heap allocation.
    auto spawnFireParticle = [&](sf::Vector2f pos, sf::Vector2f vel, sf::Color color, float size, float maxLife, int kind) {
        for (auto& fp : fireParticles)
        {
            if (!fp.active)
            {
                fp.active = true;
                fp.kind = kind;
                fp.pos = pos;
                fp.vel = vel;
                fp.color = color;
                fp.size = size;
                fp.life = 0.f;
                fp.maxLife = maxLife;
                break;
            }
        }
    };

    // Fireball explosion: flash ring + flying sparks + a short-lived smoke cloud.
    auto spawnExplosion = [&](sf::Vector2f pos, float scale) {
        fbBoomSound.play();
        for (auto& r : explosionRings)
        {
            if (!r.active)
            {
                r.active = true;
                r.pos = pos;
                r.radius = 10.f * scale;
                r.maxRadius = 105.f * scale;
                r.life = 0.f;
                r.maxLife = 0.5f;
                break;
            }
        }
        for (int i = 0; i < 14; ++i)
        {
            float a = (float(rand() % 360) * 3.14159f) / 180.f;
            float sp = 90.f + float(rand() % 220);
            sf::Color c(255, 180 + rand() % 75, 60 + rand() % 70);
            spawnFireParticle(pos, {std::cos(a) * sp, std::sin(a) * sp - 40.f}, c,
                              2.5f + float(rand() % 3), 0.3f + float(rand() % 30) / 100.f, 0);
        }
        for (int i = 0; i < 6; ++i)
        {
            float a = (float(rand() % 360) * 3.14159f) / 180.f;
            float sp = 15.f + float(rand() % 45);
            spawnFireParticle(pos, {std::cos(a) * sp, -12.f - float(rand() % 45)}, sf::Color(85, 72, 64),
                              8.f + float(rand() % 6), 0.7f + float(rand() % 60) / 100.f, 1);
        }
    };

    // Fireball launch: whoosh + expanding ring + sparks + screen shake.
    auto spawnLaunchFx = [&](sf::Vector2f pos) {
        fbWhooshSound.play();
        for (auto& r : explosionRings)
        {
            if (!r.active)
            {
                r.active = true;
                r.pos = pos;
                r.radius = 6.f;
                r.maxRadius = 62.f;
                r.life = 0.f;
                r.maxLife = 0.3f;
                break;
            }
        }
        for (int i = 0; i < 8; ++i)
        {
            float a = (float(rand() % 360) * 3.14159f) / 180.f;
            float sp = 120.f + float(rand() % 160);
            spawnFireParticle(pos, {std::cos(a) * sp, std::sin(a) * sp}, sf::Color(255, 190 + rand() % 65, 60),
                              2.f + float(rand() % 3), 0.25f + float(rand() % 20) / 100.f, 0);
        }
        addScreenShake(balance.fireballShakeAmount, 0.22f);
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
                    else if (gameState == GameState::LoginScreen)
                    {
                        gameState = GameState::HomeMenu; // Esc backs out of the login screen
                    }
                    else if (gameState == GameState::Settings || gameState == GameState::HowToPlay)
                    {
                        gameState = GameState::HomeMenu; // Esc backs out of the placeholders
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
                else if (gameState == GameState::HomeMenu)
                {
                    if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
                    {
                        homeSelection = (homeSelection + 4) % 5;
                    }
                    else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
                    {
                        homeSelection = (homeSelection + 1) % 5;
                    }
                    else if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space)
                    {
                        confirmHomeSelection();
                    }
                }
                else if (gameState == GameState::Settings || gameState == GameState::HowToPlay)
                {
                    if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space)
                    {
                        gameState = GameState::HomeMenu; // Back
                    }
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

                        for (auto& w : wizards) w.active = false;
                        clearMiniFireballs();
                        resetWizardSpawnTimers();

                        shockwaves.clear();
                        ghostTrails.clear();
                        for (auto& fn : floatingNumbers) fn.active = false;
                        for (auto& dh : damageHistory) { dh.timestamp = -100.f; dh.amount = 0.f; }
                        damageHistoryHead = 0;
                        clock.restart();
                    }

                    // Return to the Home menu on 'L' (Logout after a run)
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

                        for (auto& w : wizards) w.active = false;
                        clearMiniFireballs();
                        resetWizardSpawnTimers();

                        shockwaves.clear();
                        ghostTrails.clear();
                        for (auto& fn : floatingNumbers) fn.active = false;
                        for (auto& dh : damageHistory) { dh.timestamp = -100.f; dh.amount = 0.f; }
                        damageHistoryHead = 0;
                        gameState = GameState::HomeMenu;
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
                    else if (gameState == GameState::HomeMenu)
                    {
                        // Left click confirms the button under the cursor.
                        for (int i = 0; i < 5; ++i)
                        {
                            if (homeButtonRect(i).contains(mousePos))
                            {
                                homeSelection = i;
                                confirmHomeSelection();
                                break;
                            }
                        }
                    }
                    else if (gameState == GameState::Settings || gameState == GameState::HowToPlay)
                    {
                        if (placeholderBackRect().contains(mousePos))
                        {
                            gameState = GameState::HomeMenu; // Back
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
        // HOME MENU RENDERING
        // ===============================================
        if (gameState == GameState::HomeMenu)
        {
            // Ambient particles floating smoothly (shared with the login screen)
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

            drawMountainRidge(window, {200.f, 440.f}, 260.f, 310.f, sf::Color(22, 26, 42, 160), sf::Color(14, 16, 28, 160));
            drawMountainRidge(window, {900.f, 420.f}, 280.f, 330.f, sf::Color(24, 28, 44, 160), sf::Color(15, 18, 30, 160));
            drawMountainRidge(window, {550.f, 490.f}, 350.f, 260.f, sf::Color(18, 22, 36, 200), sf::Color(11, 14, 24, 200));

            for (const auto& ff : weatherMotes)
            {
                sf::CircleShape dot(ff.size * 0.9f, 8);
                dot.setFillColor(sf::Color(140, 230, 255, 140));
                dot.setPosition(ff.pos);
                window.draw(dot);
            }

            drawRadialGlow(window, {550.f, 360.f}, 400.f, sf::Color(0, 140, 220, 30));
            drawRadialGlow(window, {550.f, 360.f}, 230.f, sf::Color(70, 200, 255, 25));

            if (fontLoaded)
            {
                sf::Text title(font, "Shardfall", 40);
                title.setFillColor(sf::Color(255, 222, 95));
                sf::FloatRect tb = title.getLocalBounds();
                title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
                title.setPosition({550.f, 172.f});
                window.draw(title);

                sf::Text subtitle(font, "Realm of the Lost Crystals", 14);
                subtitle.setFillColor(sf::Color(140, 230, 255));
                sf::FloatRect sb = subtitle.getLocalBounds();
                subtitle.setOrigin({sb.position.x + sb.size.x / 2.f, sb.position.y + sb.size.y / 2.f});
                subtitle.setPosition({550.f, 214.f});
                window.draw(subtitle);
            }

            const char* labels[5] = { "Play", "Hall of Heroes", "Settings", "How to Play", "Quit" };

            // Mouse hover selects the button under the cursor.
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mPos = window.mapPixelToCoords(pixelPos);
            for (int i = 0; i < 5; ++i)
            {
                if (homeButtonRect(i).contains(mPos)) { homeSelection = i; break; }
            }

            for (int i = 0; i < 5; ++i)
            {
                bool sel = (i == homeSelection);
                sf::FloatRect r = homeButtonRect(i);
                float cx = r.position.x + r.size.x / 2.f;
                float cy = r.position.y + r.size.y / 2.f;

                if (sel)
                    drawRadialGlow(window, {cx, cy}, 150.f, sf::Color(255, 205, 80, 55));

                // Gold border + dark panel, matching the existing UI style.
                drawRoundedRect(r, uiStyle.hudPanelCorner, sel ? uiStyle.hudPanelBorder : sf::Color(90, 100, 115, 210));
                sf::FloatRect inner({r.position.x + 3.f, r.position.y + 3.f}, {r.size.x - 6.f, r.size.y - 6.f});
                drawRoundedRect(inner, uiStyle.hudPanelCorner - 1.f, sel ? sf::Color(26, 34, 48, 250) : uiStyle.hudPanelBg);

                if (fontLoaded)
                {
                    std::string label = (sel ? "> " : "") + std::string(labels[i]) + (sel ? " <" : "");
                    sf::Text item(font, label, 20);
                    item.setFillColor(sel ? sf::Color(255, 236, 150) : sf::Color(205, 220, 235));
                    sf::FloatRect ib = item.getLocalBounds();
                    item.setOrigin({ib.position.x + ib.size.x / 2.f, ib.position.y + ib.size.y / 2.f});
                    item.setPosition({cx, cy});
                    window.draw(item);
                }
            }

            if (fontLoaded)
            {
                sf::Text best(font, "Best: " + std::to_string(highScore), 14);
                best.setFillColor(sf::Color(255, 215, 105));
                best.setPosition({24.f, 22.f});
                window.draw(best);

                sf::Text version(font, "v1.0", 12);
                version.setFillColor(sf::Color(135, 160, 185, 180));
                sf::FloatRect vb = version.getLocalBounds();
                version.setPosition({1100.f - vb.size.x - 24.f, 750.f - vb.size.y - 34.f});
                window.draw(version);

                sf::Text hint(font, "Up/Down or W/S: navigate    Enter/Space or click: confirm", 11);
                hint.setFillColor(sf::Color(165, 205, 230, 180));
                sf::FloatRect hb = hint.getLocalBounds();
                hint.setOrigin({hb.position.x + hb.size.x / 2.f, hb.position.y + hb.size.y / 2.f});
                hint.setPosition({550.f, 618.f});
                window.draw(hint);
            }

            if (showHistoryDashboard)
            {
                drawHistoryDashboard(window, font, fontLoaded, matchHistory, highScore, totalTime);
            }

            window.display();
            continue;
        }

        // ===============================================
        // PLACEHOLDER SCREENS (Settings / How to Play)
        // ===============================================
        if (gameState == GameState::Settings || gameState == GameState::HowToPlay)
        {
            window.setView(window.getDefaultView());
            window.clear(sf::Color(10, 14, 22));

            drawRadialGlow(window, {550.f, 340.f}, 380.f, sf::Color(0, 140, 220, 30));
            drawRadialGlow(window, {550.f, 340.f}, 210.f, sf::Color(70, 200, 255, 25));

            if (fontLoaded)
            {
                const char* titleStr = (gameState == GameState::Settings) ? "Settings" : "How to Play";
                sf::Text title(font, titleStr, 34);
                title.setFillColor(sf::Color(255, 222, 95));
                sf::FloatRect tb = title.getLocalBounds();
                title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
                title.setPosition({550.f, 260.f});
                window.draw(title);

                sf::Text soon(font, "Coming soon", 20);
                soon.setFillColor(sf::Color(180, 205, 230));
                sf::FloatRect sob = soon.getLocalBounds();
                soon.setOrigin({sob.position.x + sob.size.x / 2.f, sob.position.y + sob.size.y / 2.f});
                soon.setPosition({550.f, 330.f});
                window.draw(soon);
            }

            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mPos = window.mapPixelToCoords(pixelPos);
            sf::FloatRect backR = placeholderBackRect();
            bool backHover = backR.contains(mPos);
            float bcx = backR.position.x + backR.size.x / 2.f;
            float bcy = backR.position.y + backR.size.y / 2.f;

            if (backHover)
                drawRadialGlow(window, {bcx, bcy}, 150.f, sf::Color(255, 205, 80, 55));

            drawRoundedRect(backR, uiStyle.hudPanelCorner, backHover ? uiStyle.hudPanelBorder : sf::Color(90, 100, 115, 210));
            sf::FloatRect backInner({backR.position.x + 3.f, backR.position.y + 3.f}, {backR.size.x - 6.f, backR.size.y - 6.f});
            drawRoundedRect(backInner, uiStyle.hudPanelCorner - 1.f, backHover ? sf::Color(26, 34, 48, 250) : uiStyle.hudPanelBg);

            if (fontLoaded)
            {
                sf::Text back(font, "Back", 20);
                back.setFillColor(backHover ? sf::Color(255, 236, 150) : sf::Color(205, 220, 235));
                sf::FloatRect bb = back.getLocalBounds();
                back.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
                back.setPosition({bcx, bcy});
                window.draw(back);

                sf::Text hint(font, "Esc or click Back to return", 11);
                hint.setFillColor(sf::Color(165, 205, 230, 180));
                sf::FloatRect hb = hint.getLocalBounds();
                hint.setOrigin({hb.position.x + hb.size.x / 2.f, hb.position.y + hb.size.y / 2.f});
                hint.setPosition({550.f, 560.f});
                window.draw(hint);
            }

            window.display();
            continue;
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

        // Fireball embers, sparks & smoke (fixed pools)
        for (auto& fp : fireParticles)
        {
            if (!fp.active) continue;
            fp.life += dt;
            fp.vel.x *= (1.f - 1.6f * dt);
            fp.vel.y = fp.vel.y * (1.f - 1.2f * dt) + ((fp.kind == 1) ? -46.f : -28.f) * dt; // heat rise
            fp.pos += fp.vel * dt;
            if (fp.kind == 1) fp.size += 15.f * dt; // smoke puffs spread as they rise
            if (fp.life >= fp.maxLife) fp.active = false;
        }

        // Fireball impact / launch shock rings
        for (auto& r : explosionRings)
        {
            if (!r.active) continue;
            r.life += dt;
            r.radius += (r.maxRadius - r.radius) * std::min(1.f, dt * 6.f);
            if (r.life >= r.maxLife) r.active = false;
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
                                               forestBoulders, forestTrees, mountainCrags, huts);
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

                    // Giant fireball hazard: start a fresh grace period, clear any leftovers.
                    fireballCountdown = balance.fireballGraceTime;
                    for (auto& fb : fireballs) fb.state = FireballState::Inactive;
                    for (auto& e : enemies) e.fireballDamageId = -1;
                }
                else
                {
                    currentRealm = RealmLocation::ForestValley;
                    playerPos = {940.f, 360.f};
                    spawnSparks(playerPos, sf::Color(100, 240, 255), 30);
                }

                // Wizards: remove any living wizards (no score) and reset hut spawn timers.
                for (auto& w : wizards) w.active = false;
                clearMiniFireballs(); // drop any in-flight mini fireballs with the wizards
                resetWizardSpawnTimers();

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
                                                                 forestBoulders, forestTrees, mountainCrags, huts);

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
                            balance.enemyCollisionRadius, forestBoulders, forestTrees, mountainCrags, huts);
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
                                                                         forestBoulders, forestTrees, mountainCrags, huts);
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
                                balance.enemyCollisionRadius, forestBoulders, forestTrees, mountainCrags, huts);
                        }
                    }
                }
                else
                {
                    // Crawlers: chase the player while sliding around obstacles
                    moveEnemyWithCollision(enemy, playerPos, dt, currentRealm,
                        balance.enemyCollisionRadius, forestBoulders, forestTrees, mountainCrags, huts);

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

            // =============================================
            // GIANT FIREBALL HAZARD (Craggy Mountain Peaks)
            // =============================================
            if (currentRealm == RealmLocation::MountainPeaks && portalCooldown <= 0.f)
            {
                // The interval timer only counts while in the Mountain realm; it pauses
                // during the boss's phase-change invulnerability window.
                bool bossInvuln = false;
                for (const auto& foe : enemies)
                {
                    if (foe.type == EnemyType::Boss && foe.realm == currentRealm && foe.isAlive && foe.phaseInvulnTimer > 0.f)
                    {
                        bossInvuln = true;
                        break;
                    }
                }

                if (!bossInvuln)
                {
                    fireballCountdown -= dt;
                    if (fireballCountdown <= 0.f)
                    {
                        // Re-roll the interval after every event.
                        fireballCountdown = fireIntervalDist(rng);

                        // Double fireball chance: two balls, staggered by the double gap.
                        int target = (fireDoubleDist(rng) < balance.fireballDoubleChance) ? 2 : 1;
                        int placed = 0;
                        for (auto& fb : fireballs)
                        {
                            if (placed >= target) break;
                            if (fb.state == FireballState::Inactive)
                            {
                                ++placed;
                                fb.state = FireballState::Warn;
                                fb.mini = false; // reclaim the slot from any former mini ball
                                fb.id = fireballIdCounter++;
                                fb.pos = {fireSpawnXDist(rng), 366.f}; // erupts from the magma gorge
                                fb.dir = {0.f, 0.f};
                                fb.aimPos = playerPos;
                                fb.perfectDodged = false;
                                fb.emberAccum = 0.f;
                                fb.speed = balance.fireballSpeed;
                                fb.warnTime = balance.fireballWarningTime;
                                fb.warnTimer = 0.f;
                                fb.warnDelay = (placed == 2) ? balance.fireballDoubleGap : 0.f;
                            }
                        }
                    }
                }

                for (auto& fb : fireballs)
                {
                    if (fb.mini) continue; // wizard mini fireballs use their own update loop
                    if (fb.state == FireballState::Warn)
                    {
                        if (fb.warnDelay > 0.f)
                        {
                            fb.warnDelay -= dt;
                            if (fb.warnDelay <= 0.f)
                            {
                                fb.warnTimer = fb.warnTime;
                                fbWarnSound.play();
                            }
                        }
                        else
                        {
                            // Live telegraph: the path tracks the player until launch locks it.
                            fb.aimPos = playerPos;
                            sf::Vector2f toWarn = playerPos - fb.pos;
                            float dlWarn = std::sqrt(toWarn.x * toWarn.x + toWarn.y * toWarn.y);
                            fb.dir = (dlWarn > 0.0001f) ? sf::Vector2f(toWarn.x / dlWarn, toWarn.y / dlWarn)
                                                        : sf::Vector2f(0.f, 1.f);

                            fb.warnTimer -= dt;
                            if (fb.warnTimer <= 0.f)
                            {
                                fb.state = FireballState::Flying;
                                fb.aimPos = playerPos; // locked: aimed at the player's launch position
                                spawnLaunchFx(fb.pos);
                            }
                        }
                    }
                    else if (fb.state == FireballState::Flying)
                    {
                        fb.pos += fb.dir * fb.speed * dt;

                        // Continuous ember trail (fixed pool, no per-frame allocation).
                        fb.emberAccum += dt * 55.f;
                        while (fb.emberAccum >= 1.f)
                        {
                            fb.emberAccum -= 1.f;
                            float spread = (float(rand() % 100 - 50) / 100.f);
                            sf::Vector2f perp(-fb.dir.y, fb.dir.x);
                            if (rand() % 5 == 0)
                            {
                                // Occasional smoke puff drifting behind the trail.
                                spawnFireParticle(fb.pos + perp * spread * 16.f,
                                                  -fb.dir * 18.f + sf::Vector2f(spread * 30.f, -18.f),
                                                  sf::Color(90, 78, 70), 7.f + float(rand() % 6),
                                                  0.8f + float(rand() % 50) / 100.f, 1);
                            }
                            else
                            {
                                sf::Vector2f spur = -fb.dir * (50.f + float(rand() % 70)) + perp * spread * 90.f;
                                spawnFireParticle(fb.pos + perp * spread * 12.f,
                                                  spur + sf::Vector2f(0.f, -55.f),
                                                  sf::Color(255, 200 + rand() % 55, 60 + rand() % 80),
                                                  2.f + float(rand() % 3),
                                                  0.25f + float(rand() % 25) / 100.f, 0);
                            }
                        }

                        // Vanishes (with an explosion) once it leaves the play area.
                        if (fb.pos.x < -80.f || fb.pos.x > 1180.f || fb.pos.y < -80.f || fb.pos.y > 830.f)
                        {
                            sf::Vector2f exitPos(std::clamp(fb.pos.x, -80.f, 1180.f),
                                                 std::clamp(fb.pos.y, -80.f, 830.f));
                            spawnExplosion(exitPos, 0.9f);
                            fb.state = FireballState::Inactive;
                            continue;
                        }

                        // Damages enemies it touches; each fireball hits each enemy once.
                        for (auto& enemy : enemies)
                        {
                            if (enemy.realm != currentRealm || !enemy.isAlive) continue;
                            float ex = enemy.pos.x - fb.pos.x;
                            float ey = enemy.pos.y - fb.pos.y;
                            float ed = std::sqrt(ex * ex + ey * ey);
                            float enemyHitR = balance.fireballSize + ((enemy.type == EnemyType::Boss) ? 42.f : 26.f);
                            if (ed > enemyHitR) continue;
                            if (enemy.type == EnemyType::Boss && enemy.phaseInvulnTimer > 0.f) continue;
                            if (enemy.fireballDamageId == fb.id) continue;
                            float dmg = (enemy.type == EnemyType::Boss) ? balance.fireballBossDamage
                                                                        : balance.fireballEnemyDamage;
                            if (dmg <= 0.f) continue;
                            enemy.health = std::max(0.f, enemy.health - dmg);
                            enemy.fireballDamageId = fb.id;
                            spawnFloatingNumber(enemy.pos, static_cast<int>(std::round(dmg)), sf::Color(255, 140, 60));
                            spawnSparks(enemy.pos, sf::Color(255, 150, 60), 10);
                            if (enemy.health <= 0.f)
                            {
                                enemy.isAlive = false;
                                spawnSparks(enemy.pos, sf::Color(255, 90, 40), 30);
                                spawnExplosion(enemy.pos, 0.8f);
                            }
                        }

                        // Hits the player (or is dashed through / escaped by a perfect dodge).
                        float px = playerPos.x - fb.pos.x;
                        float py = playerPos.y - fb.pos.y;
                        float pd = std::sqrt(px * px + py * py);
                        const float hitR = balance.fireballSize + 14.f;
                        const float nearR = balance.fireballSize + 28.f;
                        if (pd < nearR && dashIFrameTimer > 0.f)
                        {
                            // Dashed through or very near the fireball: perfect dodge.
                            if (perfectDodgeCooldown <= 0.f && !fb.perfectDodged)
                                awardPerfectDodge();
                            fb.perfectDodged = true;
                            if (pd < hitR) fb.state = FireballState::Inactive; // consumed by the dodge
                        }
                        else if (pd < hitR && damageTimer <= 0.f)
                        {
                            registerPlayerDamage(std::min(health, balance.fireballPlayerDamage));
                            health = std::max(0.f, health - balance.fireballPlayerDamage);
                            healBlockTimer = balance.healDelayAfterDamage;
                            hitstopTimer = std::min(hitstopTimer + balance.hitstopPlayerHit, maxHitstop);
                            damageTimer = 0.65f;
                            int dmg = static_cast<int>(std::round(balance.fireballPlayerDamage));
                            spawnFloatingNumber(playerPos, dmg, sf::Color(255, 90, 40));
                            recordPlayerDamage(static_cast<float>(dmg));

                            hitSound.play();
                            addScreenShake(12.f, 0.35f);
                            spawnExplosion(fb.pos, 1.25f);
                            fb.state = FireballState::Inactive;

                            if (health <= 0.f)
                            {
                                gameOver = true;
                                recordMatch("DEFEATED");
                            }
                        }
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

        // =============================================
        // WIZARD ENEMIES (spawned by huts; ranged mini fireball attacks)
        // =============================================
        if (!paused)
        {
            // End screens: drop any in-flight mini fireballs so they don't linger.
            if (gameOver || victory) clearMiniFireballs();

            // Victory: every living wizard fades out quickly with a smoke puff (no score).
            if (victory)
            {
                for (auto& w : wizards)
                {
                    if (!w.active) continue;
                    if (!w.fading) { w.fading = true; w.fadeTimer = 0.f; }
                    w.fadeTimer += dt;
                    if (w.fadeTimer >= balance.wizardFadeTime)
                    {
                        w.active = false;
                        w.fading = false;
                        w.fadeTimer = 0.f;
                    }
                }
            }

            if (!gameOver && !victory)
            {
                // Spawning is paused during the teleport transition and while the boss
                // is in its phase-change invulnerability.
                bool wizardBossInvuln = false;
                for (const auto& foe : enemies)
                {
                    if (foe.type == EnemyType::Boss && foe.realm == currentRealm && foe.isAlive && foe.phaseInvulnTimer > 0.f)
                    {
                        wizardBossInvuln = true;
                        break;
                    }
                }

                bool spawnAllowed = (portalCooldown <= 0.f) && !wizardBossInvuln
                                    && (activeWizardCount() < wizardCap);

                // Each hut of the current realm runs its own timer; the global gap timer
                // enforces that huts never spawn two wizards at the same moment.
                if (spawnAllowed && wizardGlobalGapTimer <= 0.f)
                {
                    for (size_t i = 0; i < huts.size(); ++i)
                    {
                        if (i >= MAX_HUTS || huts[i].realm != currentRealm) continue;
                        wizardHutTimers[i] -= dt;
                        if (wizardHutTimers[i] <= 0.f)
                        {
                            Wizard* slot = nullptr;
                            for (auto& w : wizards)
                            {
                                if (!w.active) { slot = &w; break; }
                            }
                            if (slot)
                            {
                                slot->active = true;
                                slot->realm = currentRealm;
                                slot->pos = huts[i].doorPos;
                                slot->facing = {0.f, 1.f};
                                slot->health = balance.wizardHealth;
                                slot->maxHealth = balance.wizardHealth;
                                slot->speed = balance.wizardSpeed;
                                slot->hitTimer = 0.f;
                                slot->emergeTimer = balance.wizardEmergeTime;
                                slot->fading = false;
                                slot->fadeTimer = 0.f;
                                slot->animPhase = float(rand() % 100) / 100.f * 6.28318f;
                                slot->animMove = 0.f;
                                slot->attackTimer = wizardAttackCdDist(rng);
                                slot->telegraphTimer = 0.f;
                                slot->aimDir = {0.f, 1.f};
                                spawnSparks(huts[i].doorPos, sf::Color(150, 120, 95), 10); // dust puff
                            }
                            wizardHutTimers[i] = wizardSpawnIntervalDist(rng);
                            wizardGlobalGapTimer = balance.wizardSpawnGap;
                            break; // global gap: at most one wizard spawns per frame
                        }
                    }
                }
                if (wizardGlobalGapTimer > 0.f)
                    wizardGlobalGapTimer = std::max(0.f, wizardGlobalGapTimer - dt);
                if (wizardShotGapTimer > 0.f)
                    wizardShotGapTimer = std::max(0.f, wizardShotGapTimer - dt);

                // Wizard behaviour: approach to a preferred distance, stop and face the
                // player, back away when too close, and lob mini fireballs from range.
                for (auto& w : wizards)
                {
                    if (!w.active || w.realm != currentRealm) continue;
                    w.hitTimer = std::max(0.f, w.hitTimer - dt);

                    float dx = playerPos.x - w.pos.x;
                    float dy = playerPos.y - w.pos.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    float udx = (dist > 0.0001f) ? dx / dist : 0.f;
                    float udy = (dist > 0.0001f) ? dy / dist : 1.f;
                    w.facing = {udx, udy};

                    if (w.emergeTimer > 0.f)
                    {
                        w.emergeTimer = std::max(0.f, w.emergeTimer - dt);
                        continue; // still emerging at the door: no damage, no movement
                    }

                    // Sword hit (light enemy: one or two hits; flat score, no combo).
                    if (attackTimer > 0.f && dist < 85.f && dist > 0.f)
                    {
                        float dot = (facing.x * dx + facing.y * dy) / dist;
                        if (dot > 0.15f && w.hitTimer <= 0.f)
                        {
                            w.health -= 1.f;
                            w.hitTimer = 0.28f;
                            hitstopTimer = std::min(hitstopTimer + balance.hitstopEnemyHit, maxHitstop);
                            spawnFloatingNumber(w.pos, 1, sf::Color(255, 255, 255));
                            hitSound.play();
                            spawnSparks(w.pos, (w.realm == RealmLocation::ForestValley)
                                                   ? sf::Color(175, 95, 255)
                                                   : sf::Color(255, 95, 60), 16);
                            addScreenShake(6.f, 0.2f);

                            // Knockback that respects obstacle collision.
                            float kbDist = 45.f;
                            w.pos += sf::Vector2f(dx / dist * kbDist, dy / dist * kbDist);
                            w.pos = resolveObstacleCollision(w.pos, 12.f, w.realm,
                                                             forestBoulders, forestTrees, mountainCrags, huts);

                            if (w.health <= 0.f)
                            {
                                // Wizard death: flat score, no combo / no extra bonuses.
                                score += balance.wizardScoreValue;
                                spawnSparks(w.pos, (w.realm == RealmLocation::ForestValley)
                                                        ? sf::Color(175, 95, 255)
                                                        : sf::Color(255, 80, 50), 40);
                                addScreenShake(12.f, 0.3f);
                                w.active = false;
                                continue;
                            }
                        }
                    }

                    // Ranged attack: charge a mini fireball, then launch it at the player.
                    if (w.telegraphTimer > 0.f)
                    {
                        // Wind-up: keep the staff tracking the player, hold position.
                        w.aimDir = {udx, udy};
                        w.telegraphTimer -= dt;
                        if (w.telegraphTimer <= 0.f)
                        {
                            Fireball* slot = nullptr;
                            for (auto& fb : fireballs)
                            {
                                if (fb.state == FireballState::Inactive) { slot = &fb; break; }
                            }
                            if (slot)
                            {
                                slot->state = FireballState::Flying;
                                slot->mini = true;
                                slot->id = fireballIdCounter++;
                                slot->pos = w.pos + sf::Vector2f(udx, udy) * 14.f + sf::Vector2f(0.f, -10.f);
                                slot->dir = {udx, udy};
                                slot->aimPos = playerPos;
                                slot->warnDelay = 0.f;
                                slot->warnTimer = 0.f;
                                slot->warnTime = 0.f;
                                slot->speed = balance.miniFireballSpeed;
                                slot->emberAccum = 0.f;
                                slot->perfectDodged = false;
                                miniShotSound.play();
                                wizardShotGapTimer = balance.wizardShotGap;
                            }
                            w.telegraphTimer = 0.f;
                            w.attackTimer = wizardAttackCdDist(rng);
                        }
                        w.animMove = std::max(0.f, w.animMove - 8.f * dt);
                        continue; // charging: skip movement this frame
                    }

                    w.attackTimer = std::max(0.f, w.attackTimer - dt);
                    if (w.attackTimer <= 0.f && wizardShotGapTimer <= 0.f && dist <= balance.wizardPreferredDist + 60.f)
                    {
                        int shooters = 0;
                        for (const auto& o : wizards)
                        {
                            if (o.active && o.realm == currentRealm && o.telegraphTimer > 0.f) ++shooters;
                        }
                        if (shooters < balance.wizardMaxShooting)
                        {
                            w.telegraphTimer = balance.wizardTelegraph;
                            w.aimDir = {udx, udy};
                            miniWarnSound.play();
                        }
                        else
                        {
                            w.attackTimer = 0.25f; // too many shooters: retry shortly
                        }
                    }

                    // Move: approach until preferred distance, back away past the minimum.
                    bool moving = false;
                    if (dist > balance.wizardPreferredDist) moving = true;
                    else if (dist < balance.wizardMinDist) moving = true;

                    if (moving && dist > 0.0001f)
                    {
                        float dirSign = (dist > balance.wizardPreferredDist) ? 1.f : -0.45f;
                        w.animMove = std::min(1.f, w.animMove + 8.f * dt);
                        w.animPhase += w.speed * balance.enemyWalkSpeedFactor * dt;

                        sf::Vector2f desired(udx * dirSign * w.speed, udy * dirSign * w.speed);
                        sf::Vector2f start = w.pos;
                        sf::Vector2f step = desired * dt;
                        sf::Vector2f full = resolveObstacleCollision(start + step, 12.f, w.realm,
                                                                     forestBoulders, forestTrees, mountainCrags, huts);
                        sf::Vector2f best = full;
                        float bestProg = (full.x - start.x) * desired.x + (full.y - start.y) * desired.y;
                        float rX = full.x - (start.x + step.x);
                        float rY = full.y - (start.y + step.y);
                        if (rX * rX + rY * rY > 0.0001f)
                        {
                            sf::Vector2f xC = resolveObstacleCollision(start + sf::Vector2f(step.x, 0.f), 12.f, w.realm,
                                                                       forestBoulders, forestTrees, mountainCrags, huts);
                            sf::Vector2f yC = resolveObstacleCollision(start + sf::Vector2f(0.f, step.y), 12.f, w.realm,
                                                                       forestBoulders, forestTrees, mountainCrags, huts);
                            float xP = (xC.x - start.x) * desired.x + (xC.y - start.y) * desired.y;
                            float yP = (yC.x - start.x) * desired.x + (yC.y - start.y) * desired.y;
                            if (xP > bestProg) { bestProg = xP; best = xC; }
                            if (yP > bestProg) { best = yC; }
                        }
                        w.pos = best;
                    }
                    else
                    {
                        w.animMove = std::max(0.f, w.animMove - 8.f * dt);
                    }
                }

                // MINI FIREBALLS (wizard ranged attacks; active in both realms).
                for (auto& fb : fireballs)
                {
                    if (!fb.mini || fb.state != FireballState::Flying) continue;

                    fb.pos += fb.dir * fb.speed * dt;

                    // Short ember trail.
                    fb.emberAccum += dt * 40.f;
                    while (fb.emberAccum >= 1.f)
                    {
                        fb.emberAccum -= 1.f;
                        float spread = (float(rand() % 100 - 50) / 100.f);
                        sf::Vector2f perp(-fb.dir.y, fb.dir.x);
                        sf::Vector2f spur = -fb.dir * (35.f + float(rand() % 40)) + perp * spread * 60.f;
                        spawnFireParticle(fb.pos + perp * spread * 6.f,
                                          spur + sf::Vector2f(0.f, -30.f),
                                          sf::Color(255, 190 + rand() % 55, 70 + rand() % 70),
                                          1.5f + float(rand() % 2),
                                          0.2f + float(rand() % 20) / 100.f, 0);
                    }

                    // Fizzles out off-screen.
                    if (fb.pos.x < -60.f || fb.pos.x > 1160.f || fb.pos.y < -60.f || fb.pos.y > 810.f)
                    {
                        fb.state = FireballState::Inactive;
                        continue;
                    }

                    // Bursts on world obstacles (huts / trees / boulders / crags).
                    if (balance.miniBlockOnObstacles &&
                        circleHitsObstacle(fb.pos, balance.miniFireballSize, currentRealm,
                                           forestBoulders, forestTrees, mountainCrags, huts))
                    {
                        for (int i = 0; i < 6; ++i)
                        {
                            float a = (float(rand() % 360) * 3.14159f) / 180.f;
                            float sp = 60.f + float(rand() % 120);
                            spawnFireParticle(fb.pos, {std::cos(a) * sp, std::sin(a) * sp - 20.f},
                                              sf::Color(255, 170 + rand() % 60, 60 + rand() % 60),
                                              2.f + float(rand() % 2), 0.22f + float(rand() % 20) / 100.f, 0);
                        }
                        fb.state = FireballState::Inactive;
                        continue;
                    }

                    // Hits the player (or is dashed through for a perfect dodge).
                    float px = playerPos.x - fb.pos.x;
                    float py = playerPos.y - fb.pos.y;
                    float pd = std::sqrt(px * px + py * py);
                    const float hitR = balance.miniFireballSize + 12.f;
                    const float nearR = balance.miniFireballSize + 26.f;
                    if (pd < nearR && dashIFrameTimer > 0.f)
                    {
                        if (perfectDodgeCooldown <= 0.f && !fb.perfectDodged)
                            awardPerfectDodge();
                        fb.perfectDodged = true;
                        if (pd < hitR) fb.state = FireballState::Inactive; // consumed by the dodge
                    }
                    else if (pd < hitR && damageTimer <= 0.f)
                    {
                        registerPlayerDamage(std::min(health, balance.miniFireballDamage));
                        health = std::max(0.f, health - balance.miniFireballDamage);
                        healBlockTimer = balance.healDelayAfterDamage;
                        hitstopTimer = std::min(hitstopTimer + balance.hitstopPlayerHit, maxHitstop);
                        damageTimer = 0.5f;
                        int dmg = static_cast<int>(std::round(balance.miniFireballDamage));
                        spawnFloatingNumber(playerPos, dmg, sf::Color(255, 90, 40));
                        recordPlayerDamage(static_cast<float>(dmg));
                        hitSound.play();
                        addScreenShake(balance.miniFireballShake, 0.25f);
                        spawnSparks(fb.pos, sf::Color(255, 150, 60), 12);
                        fb.state = FireballState::Inactive;

                        if (health <= 0.f)
                        {
                            gameOver = true;
                            recordMatch("DEFEATED");
                        }
                    }
                }
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

        // Giant fireball ground telegraphs, danger markers & heat glow (Mountain Peaks)
        if (currentRealm == RealmLocation::MountainPeaks && !paused && !gameOver && !victory)
        {
            for (const auto& fb : fireballs)
            {
                if (fb.state != FireballState::Warn || fb.warnDelay > 0.f) continue;

                // Pulsing red line along the ball's path; the pulse speeds up as launch nears.
                float urgency = std::clamp(1.f - fb.warnTimer / std::max(0.0001f, fb.warnTime), 0.f, 1.f);
                float pulse = (std::sin(totalTime * (6.f + urgency * 16.f)) + 1.f) * 0.5f;
                float alphaMul = 0.4f + urgency * 0.6f;
                sf::Vector2f pathVec = fb.aimPos - fb.pos;
                float pathLen = std::sqrt(pathVec.x * pathVec.x + pathVec.y * pathVec.y);
                if (pathLen > 3.f)
                {
                    sf::Vector2f mid = fb.pos + pathVec * 0.5f;
                    float pathAng = std::atan2(pathVec.y, pathVec.x) * 180.f / 3.14159265f;

                    sf::RectangleShape warnLine({pathLen, 3.f + pulse * 4.f});
                    warnLine.setOrigin({pathLen * 0.5f, warnLine.getSize().y * 0.5f});
                    warnLine.setPosition(mid);
                    warnLine.setRotation(sf::degrees(pathAng));
                    warnLine.setFillColor(sf::Color(255, 30, 25, static_cast<std::uint8_t>(85 + pulse * 120 * alphaMul)));
                    window.draw(warnLine);

                    sf::RectangleShape coreLine({pathLen * (0.7f + pulse * 0.3f), 2.f});
                    coreLine.setOrigin({coreLine.getSize().x * 0.5f, 1.f});
                    coreLine.setPosition(mid);
                    coreLine.setRotation(sf::degrees(pathAng));
                    coreLine.setFillColor(sf::Color(255, 150, 70, static_cast<std::uint8_t>(110 + pulse * 130 * alphaMul)));
                    window.draw(coreLine);

                    // Pulsing danger marker at the end of the path.
                    float markerPulse = (std::sin(totalTime * (7.f + urgency * 18.f)) + 1.f) * 0.5f;
                    float md = 14.f + markerPulse * 6.f;
                    sf::CircleShape marker(md, 26);
                    marker.setOrigin({md, md});
                    marker.setPosition(fb.aimPos);
                    marker.setFillColor(sf::Color(255, 30, 20, static_cast<std::uint8_t>(25 + markerPulse * 40)));
                    marker.setOutlineThickness(2.5f + markerPulse * 2.5f);
                    marker.setOutlineColor(sf::Color(255, 60, 45, static_cast<std::uint8_t>(160 + markerPulse * 95)));
                    window.draw(marker);
                }
            }

            // Soft large heat halo that lights up the ground around each flying fireball.
            for (const auto& fb : fireballs)
            {
                if (fb.state != FireballState::Flying || fb.mini) continue;
                drawRadialGlow(window, fb.pos, balance.fireballSize * 4.f, sf::Color(255, 140, 40, 42), 24);
            }
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

        // Enemy Huts in the current realm (solid world obstacles, Y-sorted)
        for (const auto& hut : huts)
        {
            if (hut.realm != currentRealm) continue;
            renderables.push_back({
                hut.pos.y + hut.size * 0.5f,
                [&window, hut, totalTime]() {
                    // Soft ground shadow
                    sf::CircleShape hs(26.f, 20);
                    hs.setFillColor(sf::Color(0, 0, 0, 80));
                    hs.setOrigin({26.f, 26.f});
                    if (hut.realm == RealmLocation::ForestValley)
                        hs.setPosition({hut.pos.x, hut.pos.y + 10.f});
                    else
                        hs.setPosition({hut.pos.x, hut.pos.y + 12.f});
                    hs.setScale({1.7f, 0.5f});
                    window.draw(hs);

                    if (hut.realm == RealmLocation::ForestValley)
                    {
                        // Wooden body
                        sf::RectangleShape body({44.f, 30.f});
                        body.setFillColor(sf::Color(96, 66, 40));
                        body.setOutlineColor(sf::Color(54, 36, 20));
                        body.setOutlineThickness(2.f);
                        body.setOrigin({22.f, 15.f});
                        body.setPosition({hut.pos.x, hut.pos.y - 8.f});
                        window.draw(body);

                        // Timber planks
                        sf::RectangleShape plank1({44.f, 2.f});
                        plank1.setFillColor(sf::Color(74, 50, 30));
                        plank1.setPosition({hut.pos.x - 22.f, hut.pos.y - 14.f});
                        window.draw(plank1);
                        sf::RectangleShape plank2({44.f, 2.f});
                        plank2.setFillColor(sf::Color(74, 50, 30));
                        plank2.setPosition({hut.pos.x - 22.f, hut.pos.y - 4.f});
                        window.draw(plank2);

                        // Mossy sloped roof
                        sf::ConvexShape roof(4);
                        roof.setPoint(0, {-30.f, -6.f});
                        roof.setPoint(1, {30.f, -6.f});
                        roof.setPoint(2, {12.f, -24.f});
                        roof.setPoint(3, {-12.f, -24.f});
                        roof.setFillColor(sf::Color(40, 72, 44));
                        roof.setOutlineColor(sf::Color(24, 48, 28));
                        roof.setOutlineThickness(2.f);
                        roof.setPosition({hut.pos.x, hut.pos.y - 16.f});
                        window.draw(roof);

                        // Dark door (front side, where a character could later come out)
                        sf::RectangleShape door({12.f, 16.f});
                        door.setFillColor(sf::Color(38, 28, 22));
                        door.setOutlineColor(sf::Color(20, 14, 10));
                        door.setOutlineThickness(1.5f);
                        door.setOrigin({6.f, 16.f});
                        door.setPosition({hut.pos.x + 4.f, hut.pos.y + 7.f});
                        window.draw(door);

                        // Tiny warm window glow (subtle pulse)
                        float winPulse = (std::sin(totalTime * 3.f + hut.pos.x * 0.05f) + 1.f) * 0.5f;
                        sf::RectangleShape win({7.f, 9.f});
                        win.setFillColor(sf::Color(255, 205, 120, static_cast<std::uint8_t>(150 + 105 * winPulse)));
                        win.setOutlineColor(sf::Color(120, 80, 30));
                        win.setOutlineThickness(1.f);
                        win.setPosition({hut.pos.x - 10.f, hut.pos.y - 11.f});
                        window.draw(win);
                    }
                    else
                    {
                        // Stone body
                        sf::RectangleShape body({46.f, 28.f});
                        body.setFillColor(sf::Color(108, 106, 118));
                        body.setOutlineColor(sf::Color(58, 56, 66));
                        body.setOutlineThickness(2.f);
                        body.setOrigin({23.f, 14.f});
                        body.setPosition({hut.pos.x, hut.pos.y - 8.f});
                        window.draw(body);

                        // Stone seams
                        sf::RectangleShape seam1({46.f, 2.f});
                        seam1.setFillColor(sf::Color(82, 80, 92));
                        seam1.setPosition({hut.pos.x - 23.f, hut.pos.y - 12.f});
                        window.draw(seam1);
                        sf::RectangleShape seam2({46.f, 2.f});
                        seam2.setFillColor(sf::Color(82, 80, 92));
                        seam2.setPosition({hut.pos.x - 23.f, hut.pos.y - 2.f});
                        window.draw(seam2);

                        // Dark roof slab
                        sf::ConvexShape roof(4);
                        roof.setPoint(0, {-28.f, -4.f});
                        roof.setPoint(1, {28.f, -4.f});
                        roof.setPoint(2, {14.f, -16.f});
                        roof.setPoint(3, {-14.f, -16.f});
                        roof.setFillColor(sf::Color(42, 42, 50));
                        roof.setOutlineColor(sf::Color(24, 24, 30));
                        roof.setOutlineThickness(2.f);
                        roof.setPosition({hut.pos.x, hut.pos.y - 16.f});
                        window.draw(roof);

                        // Dark doorway with a faint orange glow
                        sf::RectangleShape door({12.f, 16.f});
                        door.setFillColor(sf::Color(30, 26, 24));
                        door.setOutlineColor(sf::Color(18, 15, 13));
                        door.setOutlineThickness(1.5f);
                        door.setOrigin({6.f, 16.f});
                        door.setPosition({hut.pos.x - 2.f, hut.pos.y + 7.f});
                        window.draw(door);
                        sf::CircleShape glow(8.f, 16);
                        glow.setFillColor(sf::Color(255, 150, 70, 26));
                        glow.setOrigin({8.f, 8.f});
                        glow.setPosition({hut.pos.x - 2.f, hut.pos.y + 2.f});
                        window.draw(glow);
                    }
                }
            });
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
        for (auto& enemy : enemies) {
            if (enemy.realm != currentRealm || !enemy.isAlive) continue;

            renderables.push_back({
                enemy.pos.y + ((enemy.type == EnemyType::Boss) ? 50.f : 20.f),
                [&window, &enemy, totalTime, dt]() {
                    sf::Vector2f epos = enemy.pos;
                    sf::Color baseColor = (enemy.type == EnemyType::Boss) ? sf::Color(255, 40, 60) : (enemy.type == EnemyType::Crawler ? sf::Color(80, 220, 120) : sf::Color(200, 35, 65));

                    // Visual-only walk / idle animation (no gameplay effect). The phase
                    // advances with the enemy's speed and the walk blend eases back to a
                    // neutral pose while the enemy stands still.
                    float limbSwing = 0.f;
                    float idleBreath = 0.f;
                    if (enemy.type != EnemyType::Boss)
                    {
                        bool enemyMoving = true;
                        float enemySpeedNow = enemy.speed;
                        if (enemy.type == EnemyType::Charger)
                        {
                            enemyMoving = (enemy.chargePhase == 0 || enemy.chargePhase == 2);
                            enemySpeedNow = (enemy.chargePhase == 2) ? balance.chargerChargeSpeed : enemy.speed;
                        }
                        else if (enemy.type == EnemyType::Demon)
                        {
                            enemyMoving = !enemy.isWindingUp;
                        }

                        enemy.animMove += ((enemyMoving ? 1.f : 0.f) - enemy.animMove)
                                          * std::min(1.f, dt * balance.enemyWalkEase);
                        enemy.animPhase += (enemyMoving ? enemySpeedNow : 0.f)
                                           * balance.enemyWalkSpeedFactor * dt;

                        limbSwing = std::sin(enemy.animPhase) * balance.enemySwingAngle * enemy.animMove;
                        idleBreath = std::sin(totalTime * balance.enemyBreathRate + epos.x * 0.01f)
                                     * balance.enemyBreathAmount * (1.f - enemy.animMove);
                    }

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
                        // Crawler: low, wide body with several small scuttling legs.
                        sf::Color bodyFill = (enemy.realm == RealmLocation::ForestValley) ? sf::Color(20, 45, 35) : sf::Color(55, 25, 20);
                        sf::Color outlineCol = enemy.hitTimer > 0.f ? sf::Color::White : baseColor;
                        float cy = epos.y + idleBreath;

                        // Three legs per side, drawn behind the body and scuttling out of phase.
                        const float legOffsets[3] = { -7.f, 0.f, 7.f };
                        for (int i = 0; i < 3; ++i)
                        {
                            float sw = std::sin(enemy.animPhase + static_cast<float>(i) * 1.1f) * limbSwing * 0.7f;
                            float lx = epos.x + legOffsets[i];

                            sf::RectangleShape leg({2.5f, 11.f});
                            leg.setFillColor(sf::Color(25, 25, 20));
                            leg.setOutlineThickness(1.f);
                            leg.setOutlineColor(outlineCol);
                            leg.setOrigin({1.25f, 0.f});

                            leg.setPosition({lx - 6.f, cy + 2.f});
                            leg.setRotation(sf::degrees(28.f + sw));
                            window.draw(leg);

                            leg.setPosition({lx + 6.f, cy + 2.f});
                            leg.setRotation(sf::degrees(-28.f - sw));
                            window.draw(leg);
                        }

                        // Low, wide carapace.
                        sf::CircleShape body(16.f, 20);
                        body.setScale({1.05f, 0.6f});
                        body.setFillColor(bodyFill);
                        body.setOutlineThickness(2.5f);
                        body.setOutlineColor(outlineCol);
                        body.setOrigin({16.f, 16.f});
                        body.setPosition({epos.x, cy});
                        window.draw(body);

                        // Eyes at the front of the carapace.
                        sf::CircleShape eye(2.8f, 10);
                        eye.setFillColor(sf::Color(255, 210, 70));
                        eye.setPosition({epos.x - 5.f, cy - 3.f});
                        window.draw(eye);
                        eye.setPosition({epos.x + 5.f, cy - 3.f});
                        window.draw(eye);
                    }
                    else if (enemy.type == EnemyType::Charger)
                    {
                        // Charger: heavy, forward-leaning body with a horn and thick legs.
                        bool isFlashing = enemy.isWindingUp && (std::fmod(totalTime * 20.f, 2.f) < 1.f);
                        bool isStunned = (enemy.chargePhase == 3);
                        float ang = std::atan2(enemy.chargeDir.y, enemy.chargeDir.x) * 180.f / 3.14159265f;

                        sf::Color chargerFill = isStunned ? sf::Color(45, 40, 55)
                                               : (enemy.isWindingUp ? (isFlashing ? sf::Color(255, 235, 150) : sf::Color(120, 50, 10))
                                                                    : sf::Color(95, 45, 15));
                        sf::Color chargerOutline = enemy.hitTimer > 0.f ? sf::Color::White : sf::Color(255, 150, 40);

                        float rad = ang * 3.14159265f / 180.f;
                        sf::Vector2f fwd(std::cos(rad), std::sin(rad));
                        sf::Vector2f perp(-fwd.y, fwd.x);

                        // Thick legs (two per side), swinging out of phase.
                        const float legPos[2] = { -4.f, 14.f };
                        for (int i = 0; i < 2; ++i)
                        {
                            float sw = std::sin(enemy.animPhase + static_cast<float>(i) * 1.6f) * limbSwing * 0.6f;
                            sf::Vector2f base = epos + fwd * legPos[i];

                            sf::RectangleShape leg({5.f, 13.f});
                            leg.setFillColor(chargerFill);
                            leg.setOutlineThickness(1.5f);
                            leg.setOutlineColor(chargerOutline);
                            leg.setOrigin({2.5f, 0.f});

                            leg.setPosition(base - perp * 8.f);
                            leg.setRotation(sf::degrees(ang + 90.f + sw));
                            window.draw(leg);

                            leg.setPosition(base + perp * 8.f);
                            leg.setRotation(sf::degrees(ang - 90.f - sw));
                            window.draw(leg);
                        }

                        // Heavy body (drawn after the legs so it overlaps them).
                        sf::ConvexShape body(4);
                        body.setPoint(0, {22.f, 2.f});
                        body.setPoint(1, {9.f, -14.f});
                        body.setPoint(2, {-13.f, -16.f});
                        body.setPoint(3, {-13.f, 14.f});
                        body.setFillColor(chargerFill);
                        body.setOutlineThickness(3.f);
                        body.setOutlineColor(chargerOutline);
                        body.setPosition(epos);
                        body.setRotation(sf::degrees(ang));
                        window.draw(body);

                        // Head / horn at the front.
                        sf::ConvexShape horn(3);
                        horn.setPoint(0, {18.f, -8.f});
                        horn.setPoint(1, {30.f, -1.f});
                        horn.setPoint(2, {17.f, 3.f});
                        horn.setFillColor(chargerFill);
                        horn.setOutlineThickness(2.f);
                        horn.setOutlineColor(chargerOutline);
                        horn.setPosition(epos);
                        horn.setRotation(sf::degrees(ang));
                        window.draw(horn);
                    }
                    else
                    {
                        // Demon: upright body with small flapping wings, horns and arms.
                        bool isFlashing = enemy.isWindingUp && (std::fmod(totalTime * 20.f, 2.f) < 1.f);
                        sf::Color flashColor = isFlashing ? sf::Color::White : sf::Color(255, 35, 50);

                        sf::Color demonFill = enemy.isWindingUp ? (isFlashing ? sf::Color(160, 40, 60) : sf::Color(65, 15, 25)) : sf::Color(35, 12, 45);
                        sf::Color demonOutline = enemy.hitTimer > 0.f ? sf::Color::White : (enemy.isWindingUp ? flashColor : baseColor);
                        float dy = epos.y + idleBreath;
                        float flap = std::sin(enemy.animPhase) * limbSwing * 0.5f;

                        // Wings (behind the body).
                        sf::ConvexShape wing(3);
                        wing.setFillColor(demonFill);
                        wing.setOutlineThickness(2.f);
                        wing.setOutlineColor(demonOutline);
                        wing.setPoint(0, {0.f, 0.f});
                        wing.setPoint(1, {-20.f, -10.f});
                        wing.setPoint(2, {-6.f, 12.f});
                        wing.setPosition({epos.x - 8.f, dy - 6.f});
                        wing.setRotation(sf::degrees(18.f + flap));
                        window.draw(wing);

                        sf::ConvexShape wingR(3);
                        wingR.setFillColor(demonFill);
                        wingR.setOutlineThickness(2.f);
                        wingR.setOutlineColor(demonOutline);
                        wingR.setPoint(0, {0.f, 0.f});
                        wingR.setPoint(1, {20.f, -10.f});
                        wingR.setPoint(2, {6.f, 12.f});
                        wingR.setPosition({epos.x + 8.f, dy - 6.f});
                        wingR.setRotation(sf::degrees(-18.f - flap));
                        window.draw(wingR);

                        // Arms (in front, swinging with the walk).
                        sf::RectangleShape arm({5.f, 15.f});
                        arm.setFillColor(demonFill);
                        arm.setOutlineThickness(2.f);
                        arm.setOutlineColor(demonOutline);
                        arm.setOrigin({2.5f, 0.f});
                        arm.setPosition({epos.x - 17.f, dy - 10.f});
                        arm.setRotation(sf::degrees(-limbSwing));
                        window.draw(arm);
                        arm.setPosition({epos.x + 17.f, dy - 10.f});
                        arm.setRotation(sf::degrees(limbSwing));
                        window.draw(arm);

                        // Upright body.
                        sf::CircleShape body(24.f, 24);
                        body.setScale({0.85f, 1.1f});
                        body.setFillColor(demonFill);
                        body.setOutlineThickness(3.f);
                        body.setOutlineColor(demonOutline);
                        body.setOrigin({24.f, 24.f});
                        body.setPosition({epos.x, dy - 2.f});
                        window.draw(body);

                        // Horns on top.
                        sf::ConvexShape horn(3);
                        horn.setFillColor(demonFill);
                        horn.setOutlineThickness(1.5f);
                        horn.setOutlineColor(demonOutline);
                        horn.setPoint(0, {-6.f, 0.f});
                        horn.setPoint(1, {0.f, -13.f});
                        horn.setPoint(2, {6.f, 0.f});
                        horn.setPosition({epos.x - 9.f, dy - 24.f});
                        window.draw(horn);
                        horn.setPosition({epos.x + 9.f, dy - 24.f});
                        window.draw(horn);

                        // Glowing eyes.
                        sf::CircleShape eye(3.5f, 12);
                        eye.setFillColor(sf::Color(255, 220, 70));
                        eye.setPosition({epos.x - 7.f, dy - 8.f});
                        window.draw(eye);
                        eye.setPosition({epos.x + 7.f, dy - 8.f});
                        window.draw(eye);
                    }
                }
            });
        }

        // Wizards in the current realm (spawned by huts)
        for (auto& w : wizards)
        {
            if (!w.active || w.realm != currentRealm) continue;
            renderables.push_back({
                w.pos.y + 15.f,
                [&window, &w, totalTime]() {
                    bool isForest = (w.realm == RealmLocation::ForestValley);
                    sf::Color robeCol = isForest ? sf::Color(104, 58, 168) : sf::Color(120, 32, 34);
                    sf::Color robeDark = isForest ? sf::Color(70, 36, 118) : sf::Color(82, 20, 22);
                    sf::Color outlineCol = (w.hitTimer > 0.f) ? sf::Color::White : robeDark;

                    // Emerge (fade/scale in) and victory fade-out progress.
                    float emergeFrac = (w.emergeTimer > 0.f)
                        ? (1.f - w.emergeTimer / std::max(0.01f, balance.wizardEmergeTime))
                        : 1.f;
                    float fadeProg = w.fading ? std::min(1.f, w.fadeTimer / std::max(0.01f, balance.wizardFadeTime)) : 0.f;
                    float scale = std::max(0.1f, emergeFrac) * (1.f - fadeProg);
                    float alpha = std::min(1.f, emergeFrac * 1.8f) * (1.f - fadeProg);

                    auto colFade = [&](sf::Color c, float mul) {
                        std::uint8_t a = static_cast<std::uint8_t>(std::clamp(int(c.a * mul), 0, 255));
                        return sf::Color(c.r, c.g, c.b, a);
                    };

                    // Simple walk bob (up/down while moving, gentle idle breathing).
                    float bob = (w.animMove > 0.01f)
                              ? std::abs(std::sin(w.animPhase * 2.f)) * 2.5f
                              : std::sin(totalTime * balance.enemyBreathRate + w.pos.x * 0.01f) * balance.enemyBreathAmount;

                    float gx = w.pos.x;
                    float gy = w.pos.y - bob * scale;
                    bool faceRight = (w.facing.x >= 0.f);

                    // Faint aura matching the robe while emerging / active.
                    drawRadialGlow(window, {gx, gy - 8.f}, 34.f * scale,
                                   colFade(isForest ? sf::Color(150, 95, 235, 70) : sf::Color(240, 95, 70, 70), alpha));

                    // Short staff with a glowing tip (side flipped toward travel).
                    float staffSide = faceRight ? 1.f : -1.f;
                    sf::RectangleShape staff({2.5f, 26.f});
                    staff.setFillColor(sf::Color(52, 40, 34));
                    staff.setOrigin({1.25f, 26.f});
                    staff.setPosition({gx + staffSide * 6.f * scale, gy - 2.f});
                    staff.setRotation(sf::degrees(faceRight ? -14.f : 14.f));
                    window.draw(staff);

                    sf::Vector2f tipPos(gx + staffSide * 11.f * scale, gy - 27.f);
                    drawRadialGlow(window, tipPos, 12.f * scale,
                                   colFade(isForest ? sf::Color(255, 205, 120, 210) : sf::Color(255, 170, 80, 210), alpha));
                    sf::CircleShape tip(3.f * scale, 12);
                    tip.setFillColor(colFade(sf::Color(255, 235, 150), alpha));
                    tip.setOrigin({3.f * scale, 3.f * scale});
                    tip.setPosition(tipPos);
                    window.draw(tip);

                    // Charging a shot: flare the staff tip and trace a faint aim line.
                    if (w.telegraphTimer > 0.f)
                    {
                        float chg = std::clamp(1.f - w.telegraphTimer / std::max(0.01f, balance.wizardTelegraph), 0.f, 1.f);
                        float pulse = (std::sin(totalTime * (10.f + chg * 22.f)) + 1.f) * 0.5f;
                        drawRadialGlow(window, tipPos, (14.f + pulse * 8.f) * scale,
                                       colFade(isForest ? sf::Color(255, 120, 240, 150) : sf::Color(255, 90, 60, 150),
                                               alpha * (0.5f + chg * 0.5f)));

                        float aimAng = std::atan2(w.aimDir.y, w.aimDir.x);
                        float aimLen = 110.f + chg * 70.f;
                        sf::RectangleShape aimLine({aimLen, 2.f});
                        aimLine.setOrigin({0.f, 1.f});
                        aimLine.setPosition(tipPos);
                        aimLine.setRotation(sf::degrees(aimAng * 180.f / 3.14159265f));
                        aimLine.setFillColor(colFade(isForest ? sf::Color(255, 90, 220) : sf::Color(255, 70, 40),
                                                     alpha * (0.25f + chg * 0.4f)));
                        window.draw(aimLine);
                    }

                    // Robed body (flared hem).
                    sf::ConvexShape robe(4);
                    robe.setPoint(0, {-7.f * scale, 2.f});
                    robe.setPoint(1, {7.f * scale, 2.f});
                    robe.setPoint(2, {9.5f * scale, -13.f * scale});
                    robe.setPoint(3, {-9.5f * scale, -13.f * scale});
                    robe.setFillColor(colFade(robeCol, alpha));
                    robe.setOutlineThickness(2.f);
                    robe.setOutlineColor(colFade(outlineCol, alpha));
                    robe.setPosition({gx, gy});
                    window.draw(robe);

                    // Belt at the waist.
                    sf::RectangleShape belt({15.f * scale, 2.5f * scale});
                    belt.setFillColor(colFade(robeDark, alpha));
                    belt.setPosition({gx - 7.5f * scale, gy - 13.f * scale});
                    window.draw(belt);

                    // Pointed hat with a small brim.
                    sf::ConvexShape hat(3);
                    hat.setPoint(0, {-6.f * scale, 0.f});
                    hat.setPoint(1, {6.f * scale, 0.f});
                    hat.setPoint(2, {0.f, -13.f * scale});
                    hat.setFillColor(colFade(robeDark, alpha));
                    hat.setOutlineThickness(1.5f);
                    hat.setOutlineColor(colFade(outlineCol, alpha));
                    hat.setPosition({gx, gy - 14.f * scale});
                    window.draw(hat);

                    sf::RectangleShape brim({14.f * scale, 2.f * scale});
                    brim.setFillColor(colFade(robeCol, alpha));
                    brim.setPosition({gx - 7.f * scale, gy - 14.f * scale});
                    window.draw(brim);

                    // Two small eyes in the shadowed face.
                    sf::CircleShape eye(1.6f * scale, 8);
                    eye.setFillColor(colFade(sf::Color(250, 240, 190), alpha));
                    eye.setPosition({gx - 3.f * scale, gy - 9.f * scale});
                    window.draw(eye);
                    eye.setPosition({gx + 3.f * scale, gy - 9.f * scale});
                    window.draw(eye);

                    // Victory fade: a small smoke puff around the wizard.
                    if (w.fading && fadeProg > 0.25f)
                    {
                        float puffDrift = std::sin(totalTime * 7.f + w.pos.x * 0.01f) * 4.f;
                        sf::CircleShape smoke(5.f * scale + fadeProg * 14.f, 14);
                        smoke.setFillColor(colFade(sf::Color(150, 140, 150), std::max(0.f, 1.f - fadeProg) * 0.5f));
                        smoke.setOrigin({smoke.getRadius(), smoke.getRadius()});
                        smoke.setPosition({gx + puffDrift, gy - 6.f * scale});
                        window.draw(smoke);
                    }
                }
            });
        }

        // Giant Fireballs (Mountain Peaks): layered flame ball, Y-sorted with the characters
        if (currentRealm == RealmLocation::MountainPeaks && !paused && !gameOver && !victory)
        {
            for (const auto& fb : fireballs)
            {
                if (fb.state != FireballState::Flying) continue;
                renderables.push_back({
                    fb.pos.y,
                    [&window, fb, totalTime]() {
                        sf::RenderStates add;
                        add.blendMode = sf::BlendAdd;

                        const float idf = static_cast<float>(fb.id % 10);
                        float fOuter = std::sin(totalTime * 3.2f + idf);
                        float fMid   = std::sin(totalTime * 5.7f + idf * 2.1f);
                        float fCore  = std::sin(totalTime * 9.4f + idf * 3.7f + 1.3f);

                        // Layered light the fireball emits (additive glow).
                        drawRadialGlow(window, fb.pos, balance.fireballSize * 3.4f, sf::Color(255, 150, 50, 90), 32);
                        drawRadialGlow(window, fb.pos, balance.fireballSize * 2.0f, sf::Color(255, 70, 30, 70), 24);

                        // Flame tongues trailing behind the ball, opposite of travel.
                        float backAng = std::atan2(fb.dir.y, fb.dir.x) + 3.14159265f;
                        sf::Vector2f back(std::cos(backAng), std::sin(backAng));
                        sf::Vector2f perp(-back.y, back.x);
                        for (int t = 1; t <= 4; ++t)
                        {
                            float wave = std::sin(totalTime * 13.f + idf + t * 1.7f) * 5.f;
                            float backDist = balance.fireballSize * (0.7f + t * 0.55f);
                            sf::CircleShape tongue(balance.fireballSize * (0.95f - t * 0.16f), 14);
                            tongue.setOrigin({tongue.getRadius(), tongue.getRadius()});
                            tongue.setPosition(fb.pos + back * backDist + perp * wave);
                            tongue.setFillColor(sf::Color(255, 95 - t * 12, 30, 180 - t * 30));
                            window.draw(tongue, add);
                        }

                        // Outer dark-red layer.
                        sf::CircleShape outer(balance.fireballSize * (1.f + fOuter * 0.06f), 22);
                        outer.setOrigin({outer.getRadius(), outer.getRadius()});
                        outer.setPosition(fb.pos);
                        outer.setFillColor(sf::Color(205, 35, 20, 210));
                        window.draw(outer);

                        // Orange middle layer.
                        sf::CircleShape mid(balance.fireballSize * 0.72f * (1.f + fMid * 0.07f), 20);
                        mid.setOrigin({mid.getRadius(), mid.getRadius()});
                        mid.setPosition(fb.pos);
                        mid.setFillColor(sf::Color(255, 125, 30, 235));
                        window.draw(mid, add);

                        // Bright white-yellow core.
                        sf::CircleShape core(balance.fireballSize * 0.42f * (1.f + fCore * 0.09f), 18);
                        core.setOrigin({core.getRadius(), core.getRadius()});
                        core.setPosition(fb.pos);
                        core.setFillColor(sf::Color(255, 250, 185, 255));
                        window.draw(core, add);
                    }
                });
            }
        }

        // Wizard mini fireballs (both realms): small layered flame, Y-sorted.
        if (!paused && !gameOver && !victory)
        {
            for (const auto& fb : fireballs)
            {
                if (!fb.mini || fb.state != FireballState::Flying) continue;
                renderables.push_back({
                    fb.pos.y,
                    [&window, fb, totalTime]() {
                        sf::RenderStates add;
                        add.blendMode = sf::BlendAdd;

                        const float idf = static_cast<float>(fb.id % 7);
                        float fMid = std::sin(totalTime * 7.3f + idf * 2.1f);
                        float fCore = std::sin(totalTime * 11.5f + idf * 3.7f + 1.3f);

                        // Small light the mini ball emits.
                        drawRadialGlow(window, fb.pos, balance.miniFireballSize * 3.2f,
                                       sf::Color(255, 140, 50, 80), 20);

                        // Short flame trail behind the ball.
                        float backAng = std::atan2(fb.dir.y, fb.dir.x) + 3.14159265f;
                        sf::Vector2f back(std::cos(backAng), std::sin(backAng));
                        for (int t = 1; t <= 2; ++t)
                        {
                            sf::CircleShape tongue(balance.miniFireballSize * (0.9f - t * 0.22f), 12);
                            tongue.setOrigin({tongue.getRadius(), tongue.getRadius()});
                            tongue.setPosition(fb.pos + back * balance.miniFireballSize * (0.8f + t * 0.8f));
                            tongue.setFillColor(sf::Color(255, 95 - t * 15, 30, 170 - t * 40));
                            window.draw(tongue, add);
                        }

                        // Orange middle layer.
                        sf::CircleShape mid(balance.miniFireballSize * (1.f + fMid * 0.08f), 18);
                        mid.setOrigin({mid.getRadius(), mid.getRadius()});
                        mid.setPosition(fb.pos);
                        mid.setFillColor(sf::Color(255, 120, 30, 230));
                        window.draw(mid, add);

                        // Bright core.
                        sf::CircleShape core(balance.miniFireballSize * 0.5f * (1.f + fCore * 0.1f), 14);
                        core.setOrigin({core.getRadius(), core.getRadius()});
                        core.setPosition(fb.pos);
                        core.setFillColor(sf::Color(255, 245, 180, 255));
                        window.draw(core, add);
                    }
                });
            }
        }

        // Ghost Trails
        for (const auto& gt : ghostTrails) {
            sf::RectangleShape ghost({22.f, 22.f});
            ghost.setFillColor(sf::Color(100, 220, 255, static_cast<std::uint8_t>(200 * gt.alpha)));
            ghost.setOrigin({11.f, 11.f});
            ghost.setPosition(gt.pos);
            window.draw(ghost);
        }

        // Advance hero walk-cycle / idle-breathing state (visuals only, no gameplay effect)
        {
            float moveMag = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
            float targetWalk = (moveMag > 0.01f) ? 1.f : 0.f;
            heroWalkAmount += (targetWalk - heroWalkAmount) * std::min(1.f, dt * balance.heroWalkEase);
            heroWalkPhase += moveMag * balance.heroWalkSpeedFactor * dt;
        }

        // Player
        renderables.push_back({
            playerPos.y + 20.f,
            [&window, playerPos, facing, attackTimer, totalTime, gameOver, victory, dashTimer, heroWalkPhase, heroWalkAmount,
             &heroTrailBase, &heroTrailTip, &heroTrailTime, &heroTrailHead, &heroTrailStrip]() {
                float angle = std::atan2(facing.y, facing.x) * 180.f / 3.14159265f;
                const float PI = 3.14159265f;

                // ---- Attack pose: three phases driven by the existing attack timer ----
                bool attacking = (attackTimer > 0.f && !gameOver && !victory);
                float p = 0.f;           // 0..1 attack progress (anticipation -> strike -> recovery)
                float swordSwing = 0.f;  // degrees the sword sweeps around the hero
                float plant = 0.f;       // 0..1 leg-plant blend during the swing
                if (attacking)
                {
                    p = std::clamp(1.f - attackTimer / balance.heroAttackAnimDuration, 0.f, 1.f);
                    const float anticipEnd = balance.heroAttackAnticFrac;
                    const float strikeEnd  = balance.heroAttackStrikeFrac;
                    if (p < anticipEnd)
                    {
                        // Anticipation: arm and sword pull back (ease-out).
                        float t = p / std::max(0.0001f, anticipEnd);
                        float e = 1.f - (1.f - t) * (1.f - t);
                        swordSwing = -balance.heroAnticipationAngle * e;
                    }
                    else if (p < strikeEnd)
                    {
                        // Strike: fast swing through the arc (ease-out).
                        float t = (p - anticipEnd) / std::max(0.0001f, strikeEnd - anticipEnd);
                        float e = 1.f - std::pow(1.f - t, 3.f);
                        swordSwing = -balance.heroAnticipationAngle
                                   + (balance.heroAnticipationAngle + balance.heroStrikeAngle) * e;
                    }
                    else
                    {
                        // Recovery: ease back to the neutral / walking pose (ease-in-out).
                        float t = (p - strikeEnd) / std::max(0.0001f, 1.f - strikeEnd);
                        float e = (t < 0.5f) ? (4.f * t * t * t)
                                             : (1.f - std::pow(-2.f * t + 2.f, 3.f) * 0.5f);
                        swordSwing = balance.heroStrikeAngle * (1.f - e);
                    }
                    plant = std::sin(p * PI); // 0 at the ends, 1 mid-swing
                }

                // Walk cycle: limbs swing from the shared phase, arms opposite the legs.
                float swing = std::sin(heroWalkPhase) * balance.heroSwingAngle * heroWalkAmount;
                float legSwing = swing * (1.f - plant * balance.heroAttackPlant); // legs plant for the strike
                // Idle: subtle upper-body (torso) rise and fall while standing still.
                float breath = std::sin(totalTime * balance.heroBreathRate) * balance.heroBreathAmount * (1.f - heroWalkAmount);
                float bodyY = playerPos.y + breath;
                float lean = swordSwing * balance.heroLeanAmount; // torso twist toward the swing
                sf::Vector2f leanShift = facing * (swordSwing * 0.06f);

                // --- Legs (two parts, opposite swing) ---
                sf::RectangleShape leftLeg({7.f, 16.f});
                leftLeg.setFillColor(sf::Color(45, 35, 30));
                leftLeg.setOrigin({3.5f, 0.f});
                leftLeg.setPosition({playerPos.x - 6.f, playerPos.y + 4.f});
                leftLeg.setRotation(sf::degrees(legSwing));
                window.draw(leftLeg);

                sf::RectangleShape rightLeg({7.f, 16.f});
                rightLeg.setFillColor(sf::Color(45, 35, 30));
                rightLeg.setOrigin({3.5f, 0.f});
                rightLeg.setPosition({playerPos.x + 6.f, playerPos.y + 4.f});
                rightLeg.setRotation(sf::degrees(-legSwing));
                window.draw(rightLeg);

                // --- Arms (two parts; sword arm follows the attack swing) ---
                sf::RectangleShape leftArm({6.f, 14.f});
                leftArm.setFillColor(sf::Color(70, 165, 200));
                leftArm.setOutlineThickness(1.5f);
                leftArm.setOutlineColor(sf::Color(210, 245, 255));
                leftArm.setOrigin({3.f, 0.f});
                leftArm.setPosition({playerPos.x - 12.f + leanShift.x, bodyY - 9.f});
                leftArm.setRotation(sf::degrees(-swing + swordSwing * 0.25f));
                window.draw(leftArm);

                sf::RectangleShape rightArm({6.f, 14.f});
                rightArm.setFillColor(sf::Color(70, 165, 200));
                rightArm.setOutlineThickness(1.5f);
                rightArm.setOutlineColor(sf::Color(210, 245, 255));
                rightArm.setOrigin({3.f, 0.f});
                rightArm.setPosition({playerPos.x + 12.f + leanShift.x, bodyY - 9.f});
                rightArm.setRotation(sf::degrees(swing + swordSwing * 0.55f));
                window.draw(rightArm);

                // --- Torso (armor) ---
                sf::RectangleShape torso({22.f, 22.f});
                torso.setFillColor((dashTimer > 0.f) ? sf::Color(140, 240, 255) : sf::Color(70, 165, 200));
                torso.setOutlineThickness(2.5f);
                torso.setOutlineColor(sf::Color(210, 245, 255));
                torso.setOrigin({11.f, 11.f});
                torso.setPosition({playerPos.x + leanShift.x, bodyY - 2.f});
                torso.setRotation(sf::degrees(lean));
                window.draw(torso);

                // --- Head (helmet) ---
                sf::CircleShape helmet(11.f, 20);
                helmet.setFillColor(sf::Color(55, 75, 95));
                helmet.setOutlineThickness(2.f);
                helmet.setOutlineColor(sf::Color(190, 235, 255));
                helmet.setOrigin({11.f, 11.f});
                helmet.setPosition({playerPos.x + leanShift.x, bodyY - 16.f});
                window.draw(helmet);

                sf::Vector2f lanternArm = playerPos + sf::Vector2f(-facing.y * 15.f + facing.x * 6.f, facing.x * 15.f + facing.y * 6.f);
                sf::CircleShape lantern(8.f, 16);
                lantern.setFillColor(sf::Color(255, 215, 80));
                lantern.setOutlineThickness(2.5f);
                lantern.setOutlineColor(sf::Color(160, 110, 20));
                lantern.setOrigin({8.f, 8.f});
                lantern.setPosition(lanternArm);
                window.draw(lantern);

                // Sword transform for this frame (shared by the trail, blade and arc).
                bool haveSword = false;
                sf::Vector2f swordPos{0.f, 0.f};
                sf::Vector2f handDir = facing;
                sf::Vector2f bladeBase{0.f, 0.f};
                sf::Vector2f bladeTip{0.f, 0.f};
                float swordRotation = angle + 90.f;

                if (attacking)
                {
                    float swingRad = (angle + swordSwing) * PI / 180.f;
                    handDir = sf::Vector2f(std::cos(swingRad), std::sin(swingRad));
                    swordPos = playerPos + handDir * 40.f;
                    swordRotation = angle + 90.f + swordSwing;

                    float bladeRad = swordRotation * PI / 180.f;
                    sf::Vector2f bladeDir(-std::sin(bladeRad), std::cos(bladeRad));
                    bladeBase = swordPos;
                    bladeTip  = swordPos + bladeDir * 20.f;

                    // Record trail samples through the strike/recovery motion only.
                    if (p >= balance.heroAttackAnticFrac * 0.5f)
                    {
                        heroTrailBase[heroTrailHead] = bladeBase;
                        heroTrailTip[heroTrailHead]  = bladeTip;
                        heroTrailTime[heroTrailHead] = totalTime;
                        heroTrailHead = (heroTrailHead + 1) % heroTrailMax;
                    }
                    haveSword = true;
                }

                // ---- Sword trail (fixed-size reusable ribbon that fades out) ----
                {
                    int totalValid = 0;
                    for (int i = 0; i < heroTrailMax; ++i)
                    {
                        int idx = (heroTrailHead + i) % heroTrailMax;
                        float age = totalTime - heroTrailTime[idx];
                        if (heroTrailTime[idx] < 0.f || age < 0.f || age > balance.heroTrailDuration) continue;
                        ++totalValid;
                    }

                    int skip = std::max(0, totalValid - balance.heroTrailLength);
                    int v = 0;
                    sf::Vector2f lastPos(0.f, 0.f);
                    for (int i = 0; i < heroTrailMax; ++i)
                    {
                        int idx = (heroTrailHead + i) % heroTrailMax;
                        float age = totalTime - heroTrailTime[idx];
                        if (heroTrailTime[idx] < 0.f || age < 0.f || age > balance.heroTrailDuration) continue;
                        if (skip > 0) { --skip; continue; }

                        float a = 1.f - age / balance.heroTrailDuration;
                        std::uint8_t alpha = static_cast<std::uint8_t>(150.f * a);
                        heroTrailStrip[v].position = heroTrailBase[idx];
                        heroTrailStrip[v].color = sf::Color(150, 235, 255, alpha);
                        heroTrailStrip[v + 1].position = heroTrailTip[idx];
                        heroTrailStrip[v + 1].color = sf::Color(150, 235, 255, alpha);
                        lastPos = heroTrailTip[idx];
                        v += 2;
                    }

                    if (v >= 4)
                    {
                        for (int j = v; j < heroTrailMax * 2; ++j)
                        {
                            heroTrailStrip[j].position = lastPos;
                            heroTrailStrip[j].color = sf::Color(150, 235, 255, 0);
                        }
                        window.draw(heroTrailStrip);
                    }
                }

                if (haveSword)
                {
                    sf::RectangleShape sword({10.f, 44.f});
                    sword.setOrigin({5.f, 22.f});
                    sword.setFillColor(sf::Color(230, 250, 255));
                    sword.setOutlineThickness(2.5f);
                    sword.setOutlineColor(sf::Color::White);
                    sword.setPosition(swordPos);
                    sword.setRotation(sf::degrees(swordRotation));
                    window.draw(sword);

                    // Slash arc (same shape/hitbox; follows the swing and fades at the end).
                    float arcFade = std::clamp(1.f - std::max(0.f, p - balance.heroAttackStrikeFrac)
                                                    / std::max(0.0001f, 1.f - balance.heroAttackStrikeFrac), 0.f, 1.f);
                    sf::CircleShape slashArc(42.f, 24);
                    slashArc.setFillColor(sf::Color(120, 230, 255, static_cast<std::uint8_t>(130 * arcFade)));
                    slashArc.setOutlineThickness(4.f);
                    slashArc.setOutlineColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255 * arcFade)));
                    slashArc.setOrigin({42.f, 42.f});
                    slashArc.setPosition(playerPos + handDir * 22.f);
                    slashArc.setScale({1.25f, 0.5f});
                    slashArc.setRotation(sf::degrees(angle + swordSwing * 0.5f));
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

        // Fireball embers / sparks / smoke (fixed pool) - Mountain realm only
        if (currentRealm == RealmLocation::MountainPeaks && !paused && !gameOver && !victory)
        {
        for (const auto& fp : fireParticles)
        {
            if (!fp.active) continue;
            float t = std::clamp(fp.life / fp.maxLife, 0.f, 1.f);
            sf::Color col;
            if (fp.kind == 1)
            {
                // Smoke: dark grey puff that spreads as it rises.
                col = sf::Color(90, 76, 70, static_cast<std::uint8_t>(120 * (1.f - t)));
            }
            else
            {
                // Ember: yellow -> red -> dark grey as it cools.
                sf::Color y(255, 220, 90), r(255, 60, 30), g(60, 48, 48);
                sf::Color body = (t < 0.5f) ? colorLerp(y, r, t * 2.f) : colorLerp(r, g, (t - 0.5f) * 2.f);
                col = sf::Color(body.r, body.g, body.b, static_cast<std::uint8_t>(215 * (1.f - t)));
            }
            float sz = (fp.kind == 1) ? fp.size * (0.7f + 0.6f * t) : fp.size * (1.f - t * 0.45f);
            sf::CircleShape dot(sz, 8);
            dot.setOrigin({sz, sz});
            dot.setPosition(fp.pos);
            dot.setFillColor(col);
            window.draw(dot);
        }

        // Fireball launch / impact shock rings
        for (const auto& r : explosionRings)
        {
            if (!r.active) continue;
            float t = std::clamp(r.life / r.maxLife, 0.f, 1.f);
            sf::Color ringCol(255, 150 - static_cast<std::uint8_t>(90 * t), 40,
                              static_cast<std::uint8_t>(210 * (1.f - t)));
            drawRadialGlow(window, r.pos, r.radius * 0.9f,
                           sf::Color(255, 130, 40, static_cast<std::uint8_t>(70 * (1.f - t))));
            sf::CircleShape ring(r.radius, 36);
            ring.setOrigin({r.radius, r.radius});
            ring.setPosition(r.pos);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(5.f * (1.f - t) + 1.f);
            ring.setOutlineColor(ringCol);
            window.draw(ring);
        }
        } // realm gate for fireball FX

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

                sf::Text prompt(font, "[R] Play Again   |   [H] Dashboard   |   [L] Logout to Menu   |   [ENTER]/[SPACE] Skip", 13);
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
