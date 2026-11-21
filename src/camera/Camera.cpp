#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Local-axis accessors
// ─────────────────────────────────────────────────────────────────────────────
glm::vec3 Camera::forward() const {
    // Local -Z axis rotated into world space
    return orientation * glm::vec3(0.f, 0.f, -1.f);
}

glm::vec3 Camera::right() const {
    // Local +X axis rotated into world space
    return orientation * glm::vec3(1.f, 0.f, 0.f);
}

glm::vec3 Camera::up() const {
    // Local +Y axis rotated into world space
    return orientation * glm::vec3(0.f, 1.f, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// View matrix: V = R^T * T(-position)
// ─────────────────────────────────────────────────────────────────────────────
glm::mat4 Camera::getViewMatrix() const {
    glm::mat4 R = glm::mat4_cast(glm::conjugate(orientation));
    glm::mat4 T = glm::translate(glm::mat4(1.f), -position);
    return R * T;
}

// ─────────────────────────────────────────────────────────────────────────────
// applyThrust
// ─────────────────────────────────────────────────────────────────────────────
void Camera::applyThrust(glm::vec3 localDir, float dt) {
    float accel = baseThrustAccel;
    if (boostActive && boostEnergy > 5.0f) {
        accel *= boostMultiplier;
        boostEnergy = std::max(0.0f, boostEnergy - dt * 28.0f);
        thrusters.boost = true;
    }
    glm::vec3 worldDir = orientation * localDir;
    linearVel += worldDir * (accel * dt);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyAngularImpulse
// ─────────────────────────────────────────────────────────────────────────────
void Camera::applyAngularImpulse(float pitch, float yaw, float roll) {
    angularVel.x += pitch;
    angularVel.y += yaw;
    angularVel.z += roll;
}

// ─────────────────────────────────────────────────────────────────────────────
// applyKeyboardRotation — Pure keyboard flight steering
//
// pitchInput: +1.0 = pitch down, -1.0 = pitch up (or inverted if invertPitch)
// yawInput:   +1.0 = turn left,  -1.0 = turn right
// rollInput:  +1.0 = roll left,  -1.0 = roll right
// ─────────────────────────────────────────────────────────────────────────────
void Camera::applyKeyboardRotation(float pitchInput, float yawInput, float rollInput, float dt) {
    float pSign = invertPitch ? -1.0f : 1.0f;
    float targetPitch = pitchInput * pSign * keyTurnSpeed;
    float targetYaw   = yawInput * keyTurnSpeed;
    float targetRoll  = rollInput * rollSpeed;

    if (flightAssist) {
        // Fly-By-Wire: smoothly set commanded angular rates
        if (std::abs(pitchInput) > 0.01f) {
            angularVel.x = targetPitch;
            if (targetPitch > 0) thrusters.pitchDown = true; else thrusters.pitchUp = true;
        }
        if (std::abs(yawInput) > 0.01f) {
            angularVel.y = targetYaw;
            if (targetYaw > 0) thrusters.yawLeft = true; else thrusters.yawRight = true;
        }
        if (std::abs(rollInput) > 0.01f) {
            angularVel.z = targetRoll;
            if (targetRoll > 0) thrusters.rollLeft = true; else thrusters.rollRight = true;
        }
    } else {
        // Pure Newtonian: torque acceleration impulse
        float rotAccel = 3.8f;
        angularVel.x += targetPitch * rotAccel * dt;
        angularVel.y += targetYaw   * rotAccel * dt;
        angularVel.z += targetRoll  * rotAccel * dt;

        if (targetPitch > 0.01f) thrusters.pitchDown = true; else if (targetPitch < -0.01f) thrusters.pitchUp = true;
        if (targetYaw   > 0.01f) thrusters.yawLeft   = true; else if (targetYaw   < -0.01f) thrusters.yawRight = true;
        if (targetRoll  > 0.01f) thrusters.rollLeft  = true; else if (targetRoll  < -0.01f) thrusters.rollRight = true;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// applyMouseDelta
// ─────────────────────────────────────────────────────────────────────────────
void Camera::applyMouseDelta(double dx, double dy) {
    float yaw   = (float)(-dx) * mouseSensX;
    float pitch = (float)(-dy) * mouseSensY;

    if (std::abs(yaw) > 1e-6f) {
        glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
        orientation = glm::normalize(orientation * qYaw);
    }
    if (std::abs(pitch) > 1e-6f) {
        glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.f, 0.f, 0.f));
        orientation = glm::normalize(orientation * qPitch);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Autopilot Vector Alignment Computer
// ─────────────────────────────────────────────────────────────────────────────
void Camera::alignWithDirection(const glm::vec3& targetWorldDir, float dt, float turnRate) {
    if (glm::length(targetWorldDir) < 1e-4f) return;

    glm::vec3 fwd = forward();
    glm::vec3 target = glm::normalize(targetWorldDir);

    float dotVal = glm::clamp(glm::dot(fwd, target), -1.0f, 1.0f);
    float angle  = std::acos(dotVal);

    if (angle > 0.005f) {
        glm::vec3 axis = glm::cross(fwd, target);
        if (glm::length(axis) < 1e-5f) axis = up();
        axis = glm::normalize(axis);

        float step = std::min(angle, turnRate * dt);
        glm::quat rot = glm::angleAxis(step, axis);
        orientation = glm::normalize(rot * orientation);
        thrusters.autoAlign = true;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Throttle & Braking
// ─────────────────────────────────────────────────────────────────────────────
void Camera::setCruiseThrottle(float targetSpeed) {
    cruiseThrottle = glm::clamp(targetSpeed, -20.0f, 160.0f);
}

void Camera::adjustCruiseThrottle(float deltaSpeed) {
    cruiseThrottle = glm::clamp(cruiseThrottle + deltaSpeed, -20.0f, 160.0f);
}

void Camera::brakeLinear(float dt) {
    float factor = std::exp(-brakeStrength * dt);
    linearVel *= factor;
    if (glm::length(linearVel) < 0.005f) linearVel = glm::vec3(0.f);
}

void Camera::brakeAngular(float dt) {
    float factor = std::exp(-brakeStrength * dt);
    angularVel *= factor;
    if (glm::length(angularVel) < 0.001f) angularVel = glm::vec3(0.f);
}

void Camera::fullStop() {
    linearVel      = glm::vec3(0.f);
    angularVel     = glm::vec3(0.f);
    cruiseThrottle = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// integrate — Semi-implicit Euler & Flight Assist Autostabilization
// ─────────────────────────────────────────────────────────────────────────────
void Camera::integrate(float dt) {
    // 1. Boost energy recharge when not boosting
    if (!thrusters.boost) {
        boostEnergy = std::min(100.0f, boostEnergy + dt * 18.0f);
    }

    // 2. Cruise Control Throttle (auto-maintains forward velocity)
    if (std::abs(cruiseThrottle) > 0.1f) {
        float forwardSpeed = glm::dot(linearVel, forward());
        float speedDiff = cruiseThrottle - forwardSpeed;
        if (std::abs(speedDiff) > 0.25f) {
            float thrustSign = (speedDiff > 0.0f) ? -1.0f : 1.0f; // Local -Z is forward
            applyThrust(glm::vec3(0, 0, thrustSign), dt * 0.9f);
            if (thrustSign < 0) thrusters.fwd = true; else thrusters.back = true;
        }
    }

    // 3. Flight Assist (FA-ON: fly-by-wire drift & rotation stabilization)
    if (flightAssist) {
        bool noTranslationKeys = !thrusters.fwd && !thrusters.back &&
                                 !thrusters.left && !thrusters.right &&
                                 !thrusters.up && !thrusters.down;

        // If no translation input and cruise throttle is zero, damp drift
        if (noTranslationKeys && std::abs(cruiseThrottle) < 0.1f) {
            brakeLinear(dt * 0.65f);
        }

        // SAS: Stop residual spin when rotation inputs are idle
        bool rotating = thrusters.rollLeft || thrusters.rollRight ||
                        thrusters.pitchUp || thrusters.pitchDown ||
                        thrusters.yawLeft || thrusters.yawRight ||
                        thrusters.autoAlign;
        if (!rotating) {
            brakeAngular(dt * 2.5f);
        }
    }

    // 4. Integrate linear motion
    position += linearVel * dt;

    // 5. Integrate angular motion
    float angSpeed = glm::length(angularVel);
    if (angSpeed > 1e-6f) {
        float     angle    = angSpeed * dt;
        glm::vec3 axis     = angularVel / angSpeed;
        glm::quat localRot = glm::angleAxis(angle, axis);
        orientation = glm::normalize(orientation * localRot);
    }
}
