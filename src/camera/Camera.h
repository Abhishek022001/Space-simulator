#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Camera — Advanced 6DOF Microgravity Spacecraft Controller
 *
 * Features:
 *   • Flight Assist ON (Fly-By-Wire): auto-stabilizes drift & stops spin
 *   • Flight Assist OFF (Pure Newtonian): true zero-drag momentum conservation
 *   • Afterburner Boost: 3.2x thrust with energy reservoir
 *   • Cruise Control Throttle: dials in target forward speed (m/s)
 *   • Autopilot Alignment Computer: auto-slews nose to Target, Prograde, or Retrograde
 *   • Smooth virtual flight yoke with rate damping
 */
class Camera {
public:
    // ── Physical State ───────────────────────────────────────────────────────
    glm::vec3 position    = { 0.f, 2.5f, 22.f };
    glm::quat orientation = { 1.f, 0.f,  0.f,  0.f }; // Identity = looking -Z

    glm::vec3 linearVel   = { 0.f, 0.f, 0.f };        // World-space (m/s)
    glm::vec3 angularVel  = { 0.f, 0.f, 0.f };        // Body-space  (rad/s)

    // ── Flight Assist, Boost & Throttle ──────────────────────────────────────
    bool  flightAssist    = true;   // Toggle with Z (True = Fly-By-Wire, False = Raw Newtonian)
    bool  invertPitch     = false;  // Toggle with I (Normal vs Inverted Pitch)
    bool  boostActive     = false;  // Engaged with Left Shift
    float boostEnergy     = 100.0f; // 0 to 100%
    float cruiseThrottle  = 0.0f;   // Target cruise velocity in m/s (adjusted by mouse wheel or +/-)

    // ── Real-time RCS Thruster Annunciator State ─────────────────────────────
    struct ThrusterState {
        bool fwd       = false;
        bool back      = false;
        bool left      = false;
        bool right     = false;
        bool up        = false;
        bool down      = false;
        bool rollLeft  = false;
        bool rollRight = false;
        bool pitchUp   = false;
        bool pitchDown = false;
        bool yawLeft   = false;
        bool yawRight  = false;
        bool boost     = false;
        bool brake     = false;
        bool fullStop  = false;
        bool autoAlign = false;
    } thrusters;

    // ── Engine Tuning ────────────────────────────────────────────────────────
    float baseThrustAccel = 6.5f;   // m/s² standard RCS acceleration
    float boostMultiplier = 3.2f;   // Multiplier when afterburner is firing (20.8 m/s²)
    float keyTurnSpeed    = 1.6f;   // rad/s for Arrow Key / Numpad steering (~92°/s)
    float mouseSensX      = 0.0016f;// rad / pixel yaw
    float mouseSensY      = 0.0016f;// rad / pixel pitch
    float rollSpeed       = 1.5f;   // rad/s when Q/E held
    float brakeStrength   = 4.2f;   // Exponential decay rate (s⁻¹)

    // ── Local-axis accessors (world-space vectors) ────────────────────────────
    glm::vec3 forward() const;   // Local -Z rotated into world space
    glm::vec3 right()   const;   // Local +X rotated into world space
    glm::vec3 up()      const;   // Local +Y rotated into world space

    // ── View matrix ──────────────────────────────────────────────────────────
    glm::mat4 getViewMatrix() const;

    // ── Input responders ─────────────────────────────────────────────────────
    void applyThrust(glm::vec3 localDir, float dt);
    void applyAngularImpulse(float pitch, float yaw, float roll);
    void applyKeyboardRotation(float pitchInput, float yawInput, float rollInput, float dt);
    void applyMouseDelta(double dx, double dy);

    // ── Autopilot Alignment Computer ─────────────────────────────────────────
    void alignWithDirection(const glm::vec3& targetWorldDir, float dt, float turnRate = 3.5f);

    // ── Cruise Throttle & Damping ────────────────────────────────────────────
    void setCruiseThrottle(float targetSpeed);
    void adjustCruiseThrottle(float deltaSpeed);
    void brakeLinear(float dt);
    void brakeAngular(float dt);
    void fullStop();

    // ── Integration (call once per physics frame) ────────────────────────────
    void integrate(float dt);
};
