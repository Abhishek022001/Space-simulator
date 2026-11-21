/**
 * SpaceSim — Authentic Microgravity 3D Space Simulator
 *
 * Enhanced Spaceflight Navigation & Control System:
 *   • Flight Assist Modes: FA-ON (Fly-By-Wire stabilized) & FA-OFF (Pure Newtonian)
 *   • Cruise Control Throttle: Dial in forward speed (m/s) with Mouse Wheel / Keys
 *   • Afterburner Boost: 3.2x thrust with energy reservoir (Left Shift)
 *   • Autopilot Alignment Computer: Target Track (U), Prograde (P), Retrograde (O)
 *   • Target Navigation System: Cycle targets (T/G), Target Nearest (Y)
 *   • Off-Screen Target Pointer: Directional perimeter arrows guide your aim
 *   • 3D Space Station Docking Approach Corridor
 *   • Vector HUD Stroke Font & Telemetry Display
 *   • Toggleable In-Cockpit Flight Manual (H / F1)
 */

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/Shader.h"
#include "rendering/Mesh.h"
#include "camera/Camera.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
// Global window & navigation state
// ─────────────────────────────────────────────────────────────────────────────
static int    g_width         = 1280;
static int    g_height        = 720;
static double g_mouseDX       = 0.0;
static double g_mouseDY       = 0.0;
static double g_lastMouseX    = 640.0;
static double g_lastMouseY    = 360.0;
static bool   g_firstMouse    = true;
static bool   g_mouseCaptured = false;
static bool   g_resetCamera   = false;
static bool   g_showHelp      = false;

static Camera g_camera;
static int    g_selectedTarget = 0; // Index into g_navTargets

struct NavTarget {
    const char* name;
    glm::vec3   pos;
    glm::vec3   color;
    float       radius;
};

static std::vector<NavTarget> g_navTargets = {
    { "SPACE STATION ALPHA", {   0.0f,   0.0f,   0.0f }, { 0.25f, 0.90f, 1.00f }, 18.0f },
    { "TERRA PLANET",        { -100.0f, -60.0f, -360.0f}, { 0.35f, 0.75f, 1.00f }, 130.0f },
    { "COMSAT-1 SATELLITE",  {  42.0f,  18.0f,  -48.0f }, { 1.00f, 0.82f, 0.25f },  3.0f },
    { "ASTEROID 2026-SG1",   { -26.0f,  12.0f,  -28.0f }, { 1.00f, 0.55f, 0.20f },  4.2f },
    { "ASTEROID 2026-SG2",   {  34.0f, -16.0f,  -65.0f }, { 1.00f, 0.55f, 0.20f },  7.5f },
    { "ASTEROID 2026-SG3",   { -18.0f, -12.0f,   32.0f }, { 1.00f, 0.55f, 0.20f },  2.8f },
    { "ASTEROID 2026-SG4",   {  68.0f,  32.0f,  -95.0f }, { 1.00f, 0.55f, 0.20f }, 11.0f },
    { "ASTEROID 2026-SG5",   { -52.0f,  40.0f,  -75.0f }, { 1.00f, 0.55f, 0.20f },  5.5f }
};

// ─────────────────────────────────────────────────────────────────────────────
// Callbacks
// ─────────────────────────────────────────────────────────────────────────────
static void captureMouse(GLFWwindow* win) {
    g_mouseCaptured = true;
    g_firstMouse    = true;
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    std::cout << "[SpaceSim] Flight Mode Engaged (Mouse captured) — Tab/ESC to release\n";
}

static void releaseMouse(GLFWwindow* win) {
    g_mouseCaptured = false;
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    std::cout << "[SpaceSim] Flight Mode Suspended (Mouse free) — click window to re-engage\n";
}

static void glfwErrorCB(int e, const char* msg) {
    std::cerr << "[GLFW Error] " << e << ": " << msg << "\n";
}

static void framebufferCB(GLFWwindow*, int w, int h) {
    g_width = w; g_height = h;
    glViewport(0, 0, w, h);
}

static void windowFocusCB(GLFWwindow* win, int focused) {
    if (!focused && g_mouseCaptured)
        releaseMouse(win);
}

static void mouseButtonCB(GLFWwindow* win, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !g_mouseCaptured)
        captureMouse(win);
}

static void cursorPosCB(GLFWwindow*, double x, double y) {
    if (!g_mouseCaptured) return;
    if (g_firstMouse) { g_lastMouseX = x; g_lastMouseY = y; g_firstMouse = false; return; }
    g_mouseDX    += x - g_lastMouseX;
    g_mouseDY    += y - g_lastMouseY;
    g_lastMouseX  = x;
    g_lastMouseY  = y;
}

static void scrollCB(GLFWwindow*, double, double yoffset) {
    // Mouse wheel controls cruise throttle (+/- 2.5 m/s per click)
    g_camera.adjustCruiseThrottle((float)yoffset * 2.5f);
}

static void keyCB(GLFWwindow* win, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        if (g_mouseCaptured) { releaseMouse(win); return; }
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
    if (key == GLFW_KEY_TAB) {
        if (g_mouseCaptured) releaseMouse(win);
        else                 captureMouse(win);
    }

    // Toggle Flight Assist (FA-ON / FA-OFF)
    if (key == GLFW_KEY_Z) {
        g_camera.flightAssist = !g_camera.flightAssist;
        std::cout << "[SpaceSim] Flight Assist: "
                  << (g_camera.flightAssist ? "ON (Fly-By-Wire)" : "OFF (Newtonian Drift)") << "\n";
    }

    // Toggle Pitch Inversion (Normal / Inverted stick)
    if (key == GLFW_KEY_I) {
        g_camera.invertPitch = !g_camera.invertPitch;
        std::cout << "[SpaceSim] Pitch Inversion: " << (g_camera.invertPitch ? "INVERTED (Flight Stick)" : "NORMAL") << "\n";
    }

    // Keyboard cruise throttle adjustments
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        g_camera.adjustCruiseThrottle(+2.5f);
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        g_camera.adjustCruiseThrottle(-2.5f);
    }

    // Numpad 5: Level / Stop all rotation immediately
    if (key == GLFW_KEY_KP_5) {
        g_camera.angularVel = glm::vec3(0.0f);
    }

    // Steering turn sensitivity adjustments
    if (key == GLFW_KEY_RIGHT_BRACKET) {
        g_camera.keyTurnSpeed = std::min(3.5f, g_camera.keyTurnSpeed + 0.3f);
    }
    if (key == GLFW_KEY_LEFT_BRACKET) {
        g_camera.keyTurnSpeed = std::max(0.6f, g_camera.keyTurnSpeed - 0.3f);
    }

    // Toggle Help Manual Overlay
    if (key == GLFW_KEY_H || key == GLFW_KEY_F1) {
        g_showHelp = !g_showHelp;
    }

    // Target Selection
    if (key == GLFW_KEY_T) {
        g_selectedTarget = (g_selectedTarget + 1) % (int)g_navTargets.size();
    }
    if (key == GLFW_KEY_G) {
        g_selectedTarget = (g_selectedTarget + (int)g_navTargets.size() - 1) % (int)g_navTargets.size();
    }
    if (key == GLFW_KEY_Y) {
        // Lock closest object
        float bestDist = 1e9f;
        int bestIdx = 0;
        for (int i = 0; i < (int)g_navTargets.size(); ++i) {
            float d = glm::length(g_navTargets[i].pos - g_camera.position);
            if (d < bestDist) { bestDist = d; bestIdx = i; }
        }
        g_selectedTarget = bestIdx;
    }

    // Zero cruise throttle
    if (key == GLFW_KEY_0 || key == GLFW_KEY_BACKSPACE) {
        g_camera.setCruiseThrottle(0.0f);
    }

    // Retrograde brake stops throttle
    if (key == GLFW_KEY_X || key == GLFW_KEY_B) {
        g_camera.setCruiseThrottle(0.0f);
    }

    // Reset spacecraft to origin
    if (key == GLFW_KEY_R) {
        g_resetCamera = true;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// HUD Vector Graphics Engine (Stroke Font & Tactical Overlays)
// ─────────────────────────────────────────────────────────────────────────────
namespace {
    void addHudLine(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                    glm::vec2 p0, glm::vec2 p1, glm::vec3 col) {
        unsigned int base = static_cast<unsigned int>(verts.size());
        verts.push_back({glm::vec3(p0, 0.0f), glm::vec3(0,0,1), col});
        verts.push_back({glm::vec3(p1, 0.0f), glm::vec3(0,0,1), col});
        indices.push_back(base);
        indices.push_back(base + 1);
    }

    void addHudCircle(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                      glm::vec2 center, float radius, float aspect, glm::vec3 col, int segs = 16) {
        for (int i = 0; i < segs; ++i) {
            float th0 = (float)i * 6.2831853f / (float)segs;
            float th1 = (float)(i + 1) * 6.2831853f / (float)segs;
            glm::vec2 pt0 = center + glm::vec2(std::cos(th0) * radius / aspect, std::sin(th0) * radius);
            glm::vec2 pt1 = center + glm::vec2(std::cos(th1) * radius / aspect, std::sin(th1) * radius);
            addHudLine(verts, indices, pt0, pt1, col);
        }
    }

    // Stroke Font character segment definitions
    void drawStrokeChar(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                        char c, glm::vec2 pos, float size, float aspect, glm::vec3 col) {
        float w = size * 0.65f / aspect;
        float h = size;

        auto seg = [&](float x0, float y0, float x1, float y1) {
            addHudLine(verts, indices, pos + glm::vec2(x0 * w, y0 * h), pos + glm::vec2(x1 * w, y1 * h), col);
        };

        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

        switch (c) {
            case '0': seg(0,0,1,0); seg(1,0,1,1); seg(1,1,0,1); seg(0,1,0,0); seg(0,0,1,1); break;
            case '1': seg(0.2f,0.8f,0.5f,1.f); seg(0.5f,1.f,0.5f,0.f); seg(0.2f,0.f,0.8f,0.f); break;
            case '2': seg(0,1,1,1); seg(1,1,1,0.5f); seg(1,0.5f,0,0.5f); seg(0,0.5f,0,0); seg(0,0,1,0); break;
            case '3': seg(0,1,1,1); seg(1,1,1,0); seg(0,0,1,0); seg(0.3f,0.5f,1,0.5f); break;
            case '4': seg(0,1,0,0.5f); seg(0,0.5f,1,0.5f); seg(1,1,1,0); break;
            case '5': seg(1,1,0,1); seg(0,1,0,0.5f); seg(0,0.5f,1,0.5f); seg(1,0.5f,1,0); seg(1,0,0,0); break;
            case '6': seg(1,1,0,1); seg(0,1,0,0); seg(0,0,1,0); seg(1,0,1,0.5f); seg(1,0.5f,0,0.5f); break;
            case '7': seg(0,1,1,1); seg(1,1,0.3f,0); break;
            case '8': seg(0,0,1,0); seg(1,0,1,1); seg(1,1,0,1); seg(0,1,0,0); seg(0,0.5f,1,0.5f); break;
            case '9': seg(0,0,1,0); seg(1,0,1,1); seg(1,1,0,1); seg(0,1,0,0.5f); seg(0,0.5f,1,0.5f); break;
            case '.': seg(0.3f,0,0.7f,0); seg(0.7f,0,0.7f,0.15f); seg(0.7f,0.15f,0.3f,0.15f); seg(0.3f,0.15f,0.3f,0); break;
            case ':': seg(0.4f,0.25f,0.6f,0.25f); seg(0.4f,0.75f,0.6f,0.75f); break;
            case '-': seg(0.15f,0.5f,0.85f,0.5f); break;
            case '+': seg(0.15f,0.5f,0.85f,0.5f); seg(0.5f,0.15f,0.5f,0.85f); break;
            case '%': seg(0,0,1,1); seg(0.2f,0.8f,0.35f,0.8f); seg(0.65f,0.2f,0.8f,0.2f); break;
            case 'A': seg(0,0,0.5f,1); seg(0.5f,1,1,0); seg(0.25f,0.45f,0.75f,0.45f); break;
            case 'B': seg(0,0,0,1); seg(0,1,0.7f,1); seg(0.7f,1,1,0.75f); seg(1,0.75f,0.7f,0.5f); seg(0.7f,0.5f,0,0.5f); seg(0.7f,0.5f,1,0.25f); seg(1,0.25f,0.7f,0); seg(0.7f,0,0,0); break;
            case 'C': seg(1,1,0,1); seg(0,1,0,0); seg(0,0,1,0); break;
            case 'D': seg(0,0,0,1); seg(0,1,0.7f,1); seg(0.7f,1,1,0.5f); seg(1,0.5f,0.7f,0); seg(0.7f,0,0,0); break;
            case 'E': seg(1,1,0,1); seg(0,1,0,0); seg(0,0,1,0); seg(0,0.5f,0.7f,0.5f); break;
            case 'F': seg(0,0,0,1); seg(0,1,1,1); seg(0,0.5f,0.7f,0.5f); break;
            case 'G': seg(1,1,0,1); seg(0,1,0,0); seg(0,0,1,0); seg(1,0,1,0.5f); seg(1,0.5f,0.5f,0.5f); break;
            case 'H': seg(0,0,0,1); seg(1,0,1,1); seg(0,0.5f,1,0.5f); break;
            case 'I': seg(0.2f,1,0.8f,1); seg(0.5f,1,0.5f,0); seg(0.2f,0,0.8f,0); break;
            case 'K': seg(0,0,0,1); seg(1,1,0,0.5f); seg(0,0.5f,1,0); break;
            case 'L': seg(0,1,0,0); seg(0,0,1,0); break;
            case 'M': seg(0,0,0,1); seg(0,1,0.5f,0.4f); seg(0.5f,0.4f,1,1); seg(1,1,1,0); break;
            case 'N': seg(0,0,0,1); seg(0,1,1,0); seg(1,0,1,1); break;
            case 'O': seg(0,0,1,0); seg(1,0,1,1); seg(1,1,0,1); seg(0,1,0,0); break;
            case 'P': seg(0,0,0,1); seg(0,1,1,1); seg(1,1,1,0.5f); seg(1,0.5f,0,0.5f); break;
            case 'R': seg(0,0,0,1); seg(0,1,1,1); seg(1,1,1,0.5f); seg(1,0.5f,0,0.5f); seg(0.4f,0.5f,1,0); break;
            case 'S': seg(1,1,0,1); seg(0,1,0,0.5f); seg(0,0.5f,1,0.5f); seg(1,0.5f,1,0); seg(1,0,0,0); break;
            case 'T': seg(0,1,1,1); seg(0.5f,1,0.5f,0); break;
            case 'U': seg(0,1,0,0); seg(0,0,1,0); seg(1,0,1,1); break;
            case 'V': seg(0,1,0.5f,0); seg(0.5f,0,1,1); break;
            case 'W': seg(0,1,0.2f,0); seg(0.2f,0,0.5f,0.6f); seg(0.5f,0.6f,0.8f,0); seg(0.8f,0,1,1); break;
            case 'X': seg(0,0,1,1); seg(0,1,1,0); break;
            case 'Y': seg(0,1,0.5f,0.5f); seg(1,1,0.5f,0.5f); seg(0.5f,0.5f,0.5f,0); break;
            case 'Z': seg(0,1,1,1); seg(1,1,0,0); seg(0,0,1,0); break;
            case '[': seg(0.8f,1,0.2f,1); seg(0.2f,1,0.2f,0); seg(0.2f,0,0.8f,0); break;
            case ']': seg(0.2f,1,0.8f,1); seg(0.8f,1,0.8f,0); seg(0.8f,0,0.2f,0); break;
            case '/': seg(0,0,1,1); break;
            default: break;
        }
    }

    void drawStrokeText(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                        const std::string& text, glm::vec2 pos, float size, float aspect, glm::vec3 col) {
        float stepX = size * 0.85f / aspect;
        for (char c : text) {
            if (c != ' ') {
                drawStrokeChar(verts, indices, c, pos, size, aspect, col);
            }
            pos.x += stepX;
        }
    }

    void addTargetBracket(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                          glm::vec2 pos, float size, float aspect, glm::vec3 col, bool isLocked) {
        float sx = size / aspect;
        float sy = size;
        float arm = 0.35f;

        // Corner brackets
        addHudLine(verts, indices, pos + glm::vec2(-sx, sy), pos + glm::vec2(-sx * (1.0f - arm), sy), col);
        addHudLine(verts, indices, pos + glm::vec2(-sx, sy), pos + glm::vec2(-sx, sy * (1.0f - arm)), col);

        addHudLine(verts, indices, pos + glm::vec2(sx, sy), pos + glm::vec2(sx * (1.0f - arm), sy), col);
        addHudLine(verts, indices, pos + glm::vec2(sx, sy), pos + glm::vec2(sx, sy * (1.0f - arm)), col);

        addHudLine(verts, indices, pos + glm::vec2(-sx, -sy), pos + glm::vec2(-sx * (1.0f - arm), -sy), col);
        addHudLine(verts, indices, pos + glm::vec2(-sx, -sy), pos + glm::vec2(-sx, -sy * (1.0f - arm)), col);

        addHudLine(verts, indices, pos + glm::vec2(sx, -sy), pos + glm::vec2(sx * (1.0f - arm), -sy), col);
        addHudLine(verts, indices, pos + glm::vec2(sx, -sy), pos + glm::vec2(sx, -sy * (1.0f - arm)), col);

        // If currently locked target, add inner Diamond reticle
        if (isLocked) {
            float dx = (size * 0.5f) / aspect;
            float dy = size * 0.5f;
            addHudLine(verts, indices, pos + glm::vec2(0, dy),  pos + glm::vec2(dx, 0),  col);
            addHudLine(verts, indices, pos + glm::vec2(dx, 0),  pos + glm::vec2(0, -dy), col);
            addHudLine(verts, indices, pos + glm::vec2(0, -dy), pos + glm::vec2(-dx, 0), col);
            addHudLine(verts, indices, pos + glm::vec2(-dx, 0), pos + glm::vec2(0, dy),  col);
        }
    }

    // Off-screen indicator arrow pointing to targets outside the viewport
    void addOffScreenPointer(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                             glm::vec2 screenDir, float aspect, glm::vec3 col) {
        if (glm::length(screenDir) < 1e-4f) return;
        glm::vec2 dir = glm::normalize(screenDir);

        // Intersect with screen margin box [-0.85, 0.85]
        float maxExtent = 0.82f;
        float tX = (dir.x != 0.0f) ? std::abs(maxExtent / dir.x) : 1e9f;
        float tY = (dir.y != 0.0f) ? std::abs(maxExtent / dir.y) : 1e9f;
        float t = std::min(tX, tY);

        glm::vec2 pos = dir * t;
        float s = 0.035f;
        glm::vec2 perp(-dir.y / aspect, dir.x);

        glm::vec2 tip  = pos + dir * (s * 1.4f);
        glm::vec2 base1= pos - dir * s + perp * s;
        glm::vec2 base2= pos - dir * s - perp * s;

        addHudLine(verts, indices, tip, base1, col);
        addHudLine(verts, indices, base1, pos, col);
        addHudLine(verts, indices, pos, base2, col);
        addHudLine(verts, indices, base2, tip, col);
    }
}

static void updateHUDMesh(Mesh& hudMesh, const Camera& cam, const glm::mat4& view,
                          const glm::mat4& proj, int width, int height) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;

    float aspect = (height > 0) ? (float)width / (float)height : 1.777f;

    const glm::vec3 hudCyan  (0.25f, 0.88f, 1.00f);
    const glm::vec3 hudGreen (0.20f, 1.00f, 0.45f);
    const glm::vec3 hudAmber (1.00f, 0.65f, 0.15f);
    const glm::vec3 hudActive(1.00f, 0.95f, 0.20f);
    const glm::vec3 hudDim   (0.20f, 0.35f, 0.45f);
    const glm::vec3 hudWhite (0.95f, 0.95f, 0.98f);

    // ── 1. Center Boresight Reticle & Artificial Horizon ────────────────────
    float reticleSize = 0.025f;
    float gap = 0.008f;
    addHudLine(verts, indices, {-reticleSize / aspect, 0}, {-gap / aspect, 0}, hudCyan);
    addHudLine(verts, indices, { gap / aspect, 0}, { reticleSize / aspect, 0}, hudCyan);
    addHudLine(verts, indices, {0, -reticleSize}, {0, -gap}, hudCyan);
    addHudLine(verts, indices, {0,  gap}, {0,  reticleSize}, hudCyan);
    addHudCircle(verts, indices, {0, 0}, 0.06f, aspect, hudCyan * 0.4f, 24);

    // Roll indicator / Pitch ladder
    glm::vec3 camRight = cam.right();
    float rollAngle = std::atan2(camRight.y, camRight.x);
    glm::vec2 rDir(std::cos(rollAngle) / aspect, std::sin(rollAngle));
    addHudLine(verts, indices, -rDir * 0.14f, -rDir * 0.08f, hudCyan * 0.6f);
    addHudLine(verts, indices,  rDir * 0.08f,  rDir * 0.14f, hudCyan * 0.6f);

    // ── 2. Velocity Vectors (Prograde & Retrograde) ──────────────────────────
    float speed = glm::length(cam.linearVel);
    if (speed > 0.08f) {
        glm::vec3 vDir = glm::normalize(cam.linearVel);

        // Prograde
        glm::vec4 clipProg = proj * view * glm::vec4(cam.position + vDir * 20.0f, 1.0f);
        if (clipProg.w > 0.05f) {
            glm::vec2 ndc(clipProg.x / clipProg.w, clipProg.y / clipProg.w);
            if (std::abs(ndc.x) < 1.1f && std::abs(ndc.y) < 1.1f) {
                float r = 0.022f;
                addHudCircle(verts, indices, ndc, r, aspect, hudGreen, 16);
                addHudLine(verts, indices, ndc + glm::vec2(0, r), ndc + glm::vec2(0, r * 1.7f), hudGreen);
                addHudLine(verts, indices, ndc - glm::vec2(r / aspect, 0), ndc - glm::vec2(r * 1.7f / aspect, 0), hudGreen);
                addHudLine(verts, indices, ndc + glm::vec2(r / aspect, 0), ndc + glm::vec2(r * 1.7f / aspect, 0), hudGreen);
            }
        }

        // Retrograde
        glm::vec4 clipRetro = proj * view * glm::vec4(cam.position - vDir * 20.0f, 1.0f);
        if (clipRetro.w > 0.05f) {
            glm::vec2 ndc(clipRetro.x / clipRetro.w, clipRetro.y / clipRetro.w);
            if (std::abs(ndc.x) < 1.1f && std::abs(ndc.y) < 1.1f) {
                float r = 0.022f;
                addHudCircle(verts, indices, ndc, r, aspect, hudAmber, 16);
                float d = r * 0.707f;
                addHudLine(verts, indices, ndc + glm::vec2(-d / aspect, -d), ndc + glm::vec2(d / aspect, d), hudAmber);
                addHudLine(verts, indices, ndc + glm::vec2(-d / aspect,  d), ndc + glm::vec2(d / aspect, -d), hudAmber);
            }
        }
    }

    // ── 3. Target Navigation & Tracking ─────────────────────────────────────
    NavTarget& lockedTarget = g_navTargets[g_selectedTarget];
    glm::vec3 relPos = lockedTarget.pos - cam.position;
    float distToTarget = glm::length(relPos);
    float closingVel = (distToTarget > 1e-4f) ? -glm::dot(cam.linearVel, relPos / distToTarget) : 0.0f;

    for (int i = 0; i < (int)g_navTargets.size(); ++i) {
        bool isLocked = (i == g_selectedTarget);
        glm::vec3 tPos = g_navTargets[i].pos;
        glm::vec4 clipT = proj * view * glm::vec4(tPos, 1.0f);

        if (clipT.w > 0.1f) {
            glm::vec2 ndc(clipT.x / clipT.w, clipT.y / clipT.w);
            if (std::abs(ndc.x) < 1.05f && std::abs(ndc.y) < 1.05f) {
                float dist = glm::length(tPos - cam.position);
                float bSize = glm::clamp(20.0f / (dist + 5.0f), 0.035f, 0.09f);
                glm::vec3 col = isLocked ? hudCyan : (g_navTargets[i].color * 0.75f);
                addTargetBracket(verts, indices, ndc, bSize, aspect, col, isLocked);
            }
        } else if (isLocked) {
            // Target is behind camera: invert XY for off-screen pointer
            addOffScreenPointer(verts, indices, glm::vec2(-clipT.x, -clipT.y), aspect, hudCyan);
        }
    }

    // If locked target is in front but off-screen, show perimeter arrow
    glm::vec4 clipLocked = proj * view * glm::vec4(lockedTarget.pos, 1.0f);
    if (clipLocked.w > 0.1f) {
        glm::vec2 ndc(clipLocked.x / clipLocked.w, clipLocked.y / clipLocked.w);
        if (std::abs(ndc.x) >= 1.0f || std::abs(ndc.y) >= 1.0f) {
            addOffScreenPointer(verts, indices, ndc, aspect, hudCyan);
        }
    }

    // ── 4. Space Station Docking Approach Corridor ───────────────────────────
    // If within 150m of Space Station Alpha, draw 3D docking gateway gates
    float distToStation = glm::length(g_navTargets[0].pos - cam.position);
    if (distToStation < 150.0f) {
        float gateDistances[4] = { 15.0f, 35.0f, 65.0f, 105.0f };
        for (float gz : gateDistances) {
            float gw = 2.5f;
            float gh = 2.5f;
            glm::vec3 gCorners[4] = {
                { -gw, -gh, gz }, {  gw, -gh, gz },
                {  gw,  gh, gz }, { -gw,  gh, gz }
            };
            for (int k = 0; k < 4; ++k) {
                glm::vec4 c0 = proj * view * glm::vec4(gCorners[k], 1.0f);
                glm::vec4 c1 = proj * view * glm::vec4(gCorners[(k + 1) % 4], 1.0f);
                if (c0.w > 0.1f && c1.w > 0.1f) {
                    addHudLine(verts, indices, {c0.x / c0.w, c0.y / c0.w}, {c1.x / c1.w, c1.y / c1.w}, hudGreen * 0.7f);
                }
            }
        }
    }

    // ── 5. Vector HUD Stroke Text Telemetry Panels ───────────────────────────
    float txtSize = 0.024f;

    // Top-Left: Flight Status & Engine Modes
    drawStrokeText(verts, indices, cam.flightAssist ? "FA: ON  [FLY-BY-WIRE]" : "FA: OFF [NEWTONIAN]",
                   {-0.94f, 0.90f}, txtSize, aspect, cam.flightAssist ? hudGreen : hudAmber);

    std::ostringstream ssSpd;
    ssSpd << "SPD: " << std::fixed << std::setprecision(1) << speed << " M/S";
    drawStrokeText(verts, indices, ssSpd.str(), {-0.94f, 0.85f}, txtSize, aspect, hudWhite);

    if (std::abs(cam.cruiseThrottle) > 0.1f) {
        std::ostringstream ssThr;
        ssThr << "THR: " << std::fixed << std::setprecision(1) << cam.cruiseThrottle << " M/S [CRUISE]";
        drawStrokeText(verts, indices, ssThr.str(), {-0.94f, 0.80f}, txtSize, aspect, hudCyan);
    }

    std::ostringstream ssBst;
    ssBst << "BST: " << (int)cam.boostEnergy << "%";
    drawStrokeText(verts, indices, ssBst.str(), {-0.94f, 0.75f}, txtSize, aspect,
                   (cam.boostEnergy > 20.0f) ? hudCyan : hudAmber);

    // Top-Right: Target Navigation Telemetry
    drawStrokeText(verts, indices, std::string("TGT: ") + lockedTarget.name,
                   {0.25f, 0.90f}, txtSize, aspect, hudCyan);

    std::ostringstream ssDst;
    ssDst << "DST: " << std::fixed << std::setprecision(1) << distToTarget << " M";
    drawStrokeText(verts, indices, ssDst.str(), {0.25f, 0.85f}, txtSize, aspect, hudWhite);

    std::ostringstream ssRel;
    ssRel << "VEL: " << std::fixed << std::setprecision(1) << closingVel << " M/S";
    drawStrokeText(verts, indices, ssRel.str(), {0.25f, 0.80f}, txtSize, aspect,
                   (closingVel > 0.1f) ? hudGreen : ((closingVel < -0.1f) ? hudAmber : hudDim));

    if (closingVel > 0.5f) {
        float eta = distToTarget / closingVel;
        std::ostringstream ssEta;
        ssEta << "ETA: " << (int)eta << " S";
        drawStrokeText(verts, indices, ssEta.str(), {0.25f, 0.75f}, txtSize, aspect, hudGreen);
    }

    // Bottom-Center: Autopilot Active Indicator
    if (cam.thrusters.autoAlign) {
        drawStrokeText(verts, indices, "[ AUTOPILOT: ALIGNING ]", {-0.24f, -0.65f}, txtSize, aspect, hudGreen);
    }
    if (cam.thrusters.boost) {
        drawStrokeText(verts, indices, "[ AFTERBURNER ACTIVE ]", {-0.24f, -0.70f}, txtSize, aspect, hudCyan);
    }

    // ── 6. RCS Thruster Annunciator Matrix (Bottom Left) ─────────────────────
    float bx = -0.92f;
    float by = -0.80f;
    float bw = 0.045f;
    float bh = 0.030f;

    auto drawBoxIndicator = [&](float x, float y, bool active, glm::vec3 activeCol) {
        glm::vec3 col = active ? activeCol : hudDim;
        addHudLine(verts, indices, {x, y}, {x + bw, y}, col);
        addHudLine(verts, indices, {x + bw, y}, {x + bw, y + bh}, col);
        addHudLine(verts, indices, {x + bw, y + bh}, {x, y + bh}, col);
        addHudLine(verts, indices, {x, y + bh}, {x, y}, col);
        if (active) {
            addHudLine(verts, indices, {x, y}, {x + bw, y + bh}, col);
            addHudLine(verts, indices, {x, y + bh}, {x + bw, y}, col);
        }
    };

    drawBoxIndicator(bx + 0.00f, by + 0.10f, cam.thrusters.fwd,       hudActive);
    drawBoxIndicator(bx + 0.06f, by + 0.10f, cam.thrusters.up,        hudActive);
    drawBoxIndicator(bx + 0.12f, by + 0.10f, cam.thrusters.rollLeft,  hudCyan);

    drawBoxIndicator(bx + 0.00f, by + 0.05f, cam.thrusters.back,      hudActive);
    drawBoxIndicator(bx + 0.06f, by + 0.05f, cam.thrusters.down,      hudActive);
    drawBoxIndicator(bx + 0.12f, by + 0.05f, cam.thrusters.rollRight, hudCyan);

    drawBoxIndicator(bx + 0.00f, by + 0.00f, cam.thrusters.left,      hudActive);
    drawBoxIndicator(bx + 0.06f, by + 0.00f, cam.thrusters.right,     hudActive);
    drawBoxIndicator(bx + 0.12f, by + 0.00f, cam.thrusters.brake || cam.thrusters.fullStop, hudAmber);

    // ── 7. In-Cockpit Flight Manual Overlay (Toggleable with H) ──────────────
    if (g_showHelp) {
        float hx = -0.65f;
        float hy =  0.52f;
        float hs =  0.021f;
        drawStrokeText(verts, indices, "=== PURE KEYBOARD & FLIGHT NAVIGATION MANUAL ===", {hx, hy}, hs, aspect, hudCyan);
        drawStrokeText(verts, indices, "ARROW KEYS / NUMPAD - PITCH & YAW (NO MOUSE NEEDED)", {hx, hy - 0.045f}, hs, aspect, hudActive);
        drawStrokeText(verts, indices, "W / S               - MAIN ENGINES FORWARD / REVERSE", {hx, hy - 0.090f}, hs, aspect, hudWhite);
        drawStrokeText(verts, indices, "A / D               - RCS LATERAL STRAFE LEFT / RIGHT", {hx, hy - 0.135f}, hs, aspect, hudWhite);
        drawStrokeText(verts, indices, "SPACE / L-CTRL      - RCS VERTICAL THRUST UP / DOWN", {hx, hy - 0.180f}, hs, aspect, hudWhite);
        drawStrokeText(verts, indices, "Q / E               - ROLL LEFT / ROLL RIGHT", {hx, hy - 0.225f}, hs, aspect, hudWhite);
        drawStrokeText(verts, indices, "I                   - TOGGLE PITCH INVERSION (NORMAL/STICK)", {hx, hy - 0.270f}, hs, aspect, hudCyan);
        drawStrokeText(verts, indices, "+ / - (OR NUM +/-)  - CRUISE THROTTLE SPEED (+/-)", {hx, hy - 0.315f}, hs, aspect, hudCyan);
        drawStrokeText(verts, indices, "0 / BACKSPACE       - ZERO CRUISE THROTTLE", {hx, hy - 0.360f}, hs, aspect, hudWhite);
        drawStrokeText(verts, indices, "LEFT SHIFT          - AFTERBURNER BOOST (3.2X THRUST)", {hx, hy - 0.405f}, hs, aspect, hudActive);
        drawStrokeText(verts, indices, "Z                   - TOGGLE FLIGHT ASSIST (FA ON/OFF)", {hx, hy - 0.450f}, hs, aspect, hudGreen);
        drawStrokeText(verts, indices, "X / B               - RETRO-BRAKE (KILLS DRIFT)", {hx, hy - 0.495f}, hs, aspect, hudAmber);
        drawStrokeText(verts, indices, "F                   - EMERGENCY ALL-STOP", {hx, hy - 0.540f}, hs, aspect, hudAmber);
        drawStrokeText(verts, indices, "T / G / Y           - NEXT / PREV / CLOSEST TARGET", {hx, hy - 0.585f}, hs, aspect, hudCyan);
        drawStrokeText(verts, indices, "U / P / O (HOLD)    - AUTOPILOT: TARGET / PROGRADE / RETRO", {hx, hy - 0.630f}, hs, aspect, hudGreen);
        drawStrokeText(verts, indices, "NUMPAD 5            - STABILIZE / LEVEL ROTATION", {hx, hy - 0.675f}, hs, aspect, hudWhite);
        drawStrokeText(verts, indices, "MOUSE LOOK (OPT)    - 3DOF LOOK (CLICK WINDOW TO ENGAGE)", {hx, hy - 0.720f}, hs, aspect, hudDim);
        drawStrokeText(verts, indices, "H                   - TOGGLE THIS FLIGHT MANUAL", {hx, hy - 0.765f}, hs, aspect, hudCyan);
    } else {
        drawStrokeText(verts, indices, "[ARROWS: STEER | WASD: THRUST | Q/E: ROLL | H: HELP]", {-0.94f, -0.92f}, txtSize * 0.82f, aspect, hudDim);
    }

    hudMesh.updateLines(verts, indices);
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    glfwSetErrorCallback(glfwErrorCB);
    if (!glfwInit()) {
        std::cerr << "[FATAL] Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    GLFWwindow* win = glfwCreateWindow(g_width, g_height,
        "SpaceSim — Realistic Microgravity Space Simulator", nullptr, nullptr);
    if (!win) {
        std::cerr << "[FATAL] Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwShowWindow(win);
    glfwFocusWindow(win);

#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(win);
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOWNORMAL);
        SetForegroundWindow(hwnd);
        SetActiveWindow(hwnd);
    }
#endif

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); // 60 FPS VSync

    glfwSetFramebufferSizeCallback(win, framebufferCB);
    glfwSetWindowFocusCallback(win,     windowFocusCB);
    glfwSetMouseButtonCallback(win,     mouseButtonCB);
    glfwSetCursorPosCallback(win,       cursorPosCB);
    glfwSetScrollCallback(win,          scrollCB);
    glfwSetKeyCallback(win,             keyCB);

    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "[FATAL] Failed to load OpenGL via GLAD\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "[SpaceSim] GPU      : " << glGetString(GL_RENDERER) << "\n"
              << "[SpaceSim] OpenGL   : " << glGetString(GL_VERSION) << "\n"
              << "[SpaceSim] Shading  : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";

    // ── OpenGL Render State ──────────────────────────────────────────────────
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ── Compile Shaders ──────────────────────────────────────────────────────
    Shader starShader  ("shaders/starfield.vert", "shaders/starfield.frag");
    Shader planetShader("shaders/planet.vert",    "shaders/planet.frag");
    Shader sunShader   ("shaders/sun.vert",       "shaders/sun.frag");
    Shader objectShader("shaders/object.vert",    "shaders/object.frag");
    Shader dustShader  ("shaders/dust.vert",      "shaders/dust.frag");
    Shader hudShader   ("shaders/hud.vert",       "shaders/hud.frag");

    if (!starShader.id || !planetShader.id || !sunShader.id ||
        !objectShader.id || !dustShader.id || !hudShader.id) {
        std::cerr << "[FATAL] Failed to compile one or more shaders.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // ── Build Scene Geometries ───────────────────────────────────────────────
    Mesh starfield    = Mesh::createStarfield(6000, 850.0f);
    Mesh sunMesh      = Mesh::createSphere(42.0f, 48, 24, glm::vec3(1.0f));
    Mesh planetMesh   = Mesh::createSphere(130.0f, 96, 48, glm::vec3(1.0f));
    Mesh spaceStation = Mesh::createSpaceStation();
    Mesh satellite    = Mesh::createSatellite();

    Mesh asteroids[5] = {
        Mesh::createAsteroid(4.2f, 36, 20, 101),
        Mesh::createAsteroid(7.5f, 36, 20, 202),
        Mesh::createAsteroid(2.8f, 32, 18, 303),
        Mesh::createAsteroid(11.0f, 40, 24, 404),
        Mesh::createAsteroid(5.5f, 36, 20, 505)
    };

    Mesh spaceDust = Mesh::createSpaceDust(1600, 42.0f);
    Mesh hudMesh;

    // ── Lighting & Scene Bodies ──────────────────────────────────────────────
    const glm::vec3 lightDir = glm::normalize(glm::vec3(-0.55f, -0.40f, -0.73f));
    const glm::vec3 sunPos   = -lightDir * 650.0f;

    struct AsteroidBody {
        int targetIdx;
        glm::vec3 rotAxis;
        float rotSpeed;
        Mesh* mesh;
    };
    AsteroidBody astBodies[5] = {
        { 3, glm::normalize(glm::vec3(0.4f, 1.0f, 0.2f)),  8.5f, &asteroids[0] },
        { 4, glm::normalize(glm::vec3(0.8f, 0.2f, 0.5f)),  5.2f, &asteroids[1] },
        { 5, glm::normalize(glm::vec3(0.1f, 0.4f, 1.0f)), 14.0f, &asteroids[2] },
        { 6, glm::normalize(glm::vec3(0.6f, 0.7f, 0.3f)),  3.6f, &asteroids[3] },
        { 7, glm::normalize(glm::vec3(0.2f, 1.0f, 0.8f)),  7.8f, &asteroids[4] }
    };

    // Initial spacecraft position: docked 22m out from Space Station Alpha
    g_camera.position = glm::vec3(0.0f, 2.5f, 22.0f);

    std::cout << "[SpaceSim] Flight engine initialized.\n"
              << "[SpaceSim] Press 'H' in simulator for on-screen Flight Manual.\n\n";

    // ── Simulation & Render Loop ─────────────────────────────────────────────
    double lastTime  = glfwGetTime();
    double hudTimer  = 0.0;
    int    fpsCount  = 0;
    float  totalTime = 0.0f;

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        float dt   = (float)(now - lastTime);
        if (dt > 0.05f) dt = 0.05f;
        lastTime   = now;
        totalTime += dt;

        // Spacecraft reset
        if (g_resetCamera) {
            g_camera.position       = glm::vec3(0.0f, 2.5f, 22.0f);
            g_camera.orientation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            g_camera.linearVel      = glm::vec3(0.0f);
            g_camera.angularVel     = glm::vec3(0.0f);
            g_camera.cruiseThrottle = 0.0f;
            g_resetCamera           = false;
        }

        // Apply mouse rotation look
        if (g_mouseCaptured) {
            g_camera.applyMouseDelta(g_mouseDX, g_mouseDY);
        }
        g_mouseDX = g_mouseDY = 0.0;

        // Reset real-time thruster annunciator flags
        g_camera.thrusters = Camera::ThrusterState();

        // Check if Afterburner Boost is engaged (Left Shift)
        g_camera.boostActive = (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

        // Process continuous flight & thruster inputs (ALWAYS active for pure keyboard control)
        // ── 1. Keyboard Pitch, Yaw & Roll Steering (Arrow Keys & Numpad) ────────
        float pitchInput = 0.0f;
        float yawInput   = 0.0f;
        float rollInput  = 0.0f;

        // Pitch (Up Arrow = Nose Down, Down Arrow = Nose Up; toggleable via 'I')
        if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_KP_8) == GLFW_PRESS) {
            pitchInput += 1.0f;
        }
        if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_KP_2) == GLFW_PRESS) {
            pitchInput -= 1.0f;
        }

        // Yaw (Left Arrow = Turn Left, Right Arrow = Turn Right)
        if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_KP_4) == GLFW_PRESS) {
            yawInput += 1.0f;
        }
        if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_KP_6) == GLFW_PRESS) {
            yawInput -= 1.0f;
        }

        // Roll (Q / Numpad 7 = Roll Left, E / Numpad 9 = Roll Right)
        if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_KP_7) == GLFW_PRESS) {
            rollInput += 1.0f;
        }
        if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_KP_9) == GLFW_PRESS) {
            rollInput -= 1.0f;
        }

        if (std::abs(pitchInput) > 0.01f || std::abs(yawInput) > 0.01f || std::abs(rollInput) > 0.01f) {
            g_camera.applyKeyboardRotation(pitchInput, yawInput, rollInput, dt);
        }

        // ── 2. Translational Thrusters (W/A/S/D + Space/Ctrl) ────────────────────
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
            g_camera.applyThrust({0, 0, -1}, dt);
            g_camera.thrusters.fwd = true;
        }
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
            g_camera.applyThrust({0, 0,  1}, dt);
            g_camera.thrusters.back = true;
        }
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
            g_camera.applyThrust({-1, 0, 0}, dt);
            g_camera.thrusters.left = true;
        }
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
            g_camera.applyThrust({ 1, 0, 0}, dt);
            g_camera.thrusters.right = true;
        }
        if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {
            g_camera.applyThrust({0,  1, 0}, dt);
            g_camera.thrusters.up = true;
        }
        if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            g_camera.applyThrust({0, -1, 0}, dt);
            g_camera.thrusters.down = true;
        }

        // ── 3. Braking (X / B = Retro-Brake, F = Full Stop) ──────────────────────
        if (glfwGetKey(win, GLFW_KEY_B) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS) {
            g_camera.brakeLinear(dt);
            g_camera.brakeAngular(dt);
            g_camera.thrusters.brake = true;
        }
        if (glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS) {
            g_camera.fullStop();
            g_camera.thrusters.fullStop = true;
        }

        // ── 4. Autopilot Alignment Computers (U = Target, P = Prograde, O = Retro) ──
        if (glfwGetKey(win, GLFW_KEY_U) == GLFW_PRESS) {
            glm::vec3 dirToTarget = g_navTargets[g_selectedTarget].pos - g_camera.position;
            g_camera.alignWithDirection(dirToTarget, dt, 4.0f);
        }
        if (glfwGetKey(win, GLFW_KEY_P) == GLFW_PRESS && glm::length(g_camera.linearVel) > 0.1f) {
            g_camera.alignWithDirection(g_camera.linearVel, dt, 4.0f);
        }
        if (glfwGetKey(win, GLFW_KEY_O) == GLFW_PRESS && glm::length(g_camera.linearVel) > 0.1f) {
            g_camera.alignWithDirection(-g_camera.linearVel, dt, 4.0f);
        }

        // Integrate flight physics (Semi-implicit Euler + Flight Assist)
        g_camera.integrate(dt);

        // ── Render Scene ─────────────────────────────────────────────────────
        glClearColor(0.001f, 0.001f, 0.005f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float ar = (g_height > 0) ? (float)g_width / (float)g_height : 1.777f;
        glm::mat4 proj = glm::perspective(glm::radians(72.0f), ar, 0.05f, 1400.0f);
        glm::mat4 view = g_camera.getViewMatrix();

        // 1. Starfield
        glDepthMask(GL_FALSE);
        starShader.use();
        starShader.setMat4("uView", view);
        starShader.setMat4("uProj", proj);
        starShader.setFloat("uTime", totalTime);
        starfield.drawPoints();
        glDepthMask(GL_TRUE);

        // 2. The Sun
        sunShader.use();
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos);
        sunShader.setMat4("uModel", sunModel);
        sunShader.setMat4("uView", view);
        sunShader.setMat4("uProj", proj);
        sunShader.setVec3("uViewPos", g_camera.position);
        sunShader.setFloat("uTime", totalTime);
        sunMesh.draw();

        // 3. Terra Planet
        planetShader.use();
        glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), g_navTargets[1].pos);
        planetShader.setMat4("uModel", planetModel);
        planetShader.setMat4("uView", view);
        planetShader.setMat4("uProj", proj);
        planetShader.setVec3("uLightDir", lightDir);
        planetShader.setVec3("uViewPos", g_camera.position);
        planetShader.setFloat("uTime", totalTime);
        planetMesh.draw();

        // 4. Spacecraft & Celestial Objects
        objectShader.use();
        objectShader.setMat4("uView", view);
        objectShader.setMat4("uProj", proj);
        objectShader.setVec3("uLightDir", lightDir);
        objectShader.setVec3("uViewPos", g_camera.position);
        objectShader.setFloat("uTime", totalTime);

        // Space Station Alpha
        glm::mat4 stModel = glm::translate(glm::mat4(1.0f), g_navTargets[0].pos);
        stModel = glm::rotate(stModel, glm::radians(totalTime * 1.8f), glm::vec3(0, 0, 1));
        objectShader.setMat4("uModel", stModel);
        spaceStation.draw();

        // ComSat-1 Satellite
        glm::mat4 satModel = glm::translate(glm::mat4(1.0f), g_navTargets[2].pos);
        satModel = glm::rotate(satModel, glm::radians(totalTime * 4.2f), glm::vec3(0.3f, 1.0f, 0.2f));
        objectShader.setMat4("uModel", satModel);
        satellite.draw();

        // Asteroids
        for (auto& ast : astBodies) {
            glm::mat4 aModel = glm::translate(glm::mat4(1.0f), g_navTargets[ast.targetIdx].pos);
            aModel = glm::rotate(aModel, glm::radians(totalTime * ast.rotSpeed), ast.rotAxis);
            objectShader.setMat4("uModel", aModel);
            ast.mesh->draw();
        }

        // 5. Space Dust Particles
        glDepthMask(GL_FALSE);
        dustShader.use();
        dustShader.setMat4("uView", view);
        dustShader.setMat4("uProj", proj);
        dustShader.setVec3("uCamPos", g_camera.position);
        dustShader.setFloat("uBoxExtent", 42.0f);
        spaceDust.drawPoints();
        glDepthMask(GL_TRUE);

        // 6. Vector Flight HUD Overlay
        glDisable(GL_DEPTH_TEST);
        updateHUDMesh(hudMesh, g_camera, view, proj, g_width, g_height);
        hudShader.use();
        hudMesh.drawLines();
        glEnable(GL_DEPTH_TEST);

        // Console Telemetry
        hudTimer += dt;
        fpsCount++;
        if (hudTimer >= 1.0) {
            std::cout << std::fixed << std::setprecision(1)
                      << "FPS: " << fpsCount
                      << " | Speed: " << glm::length(g_camera.linearVel) << " m/s"
                      << " | Target: " << g_navTargets[g_selectedTarget].name
                      << " (" << glm::length(g_navTargets[g_selectedTarget].pos - g_camera.position) << "m)"
                      << (g_camera.flightAssist ? " [FA: ON]" : " [FA: OFF]")
                      << (g_mouseCaptured ? " [FLIGHT]" : " [FREE CURSOR]")
                      << "\n";
            hudTimer = 0.0;
            fpsCount = 0;
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    std::cout << "[SpaceSim] Simulation stopped.\n";
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
