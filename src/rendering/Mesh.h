#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Vertex layout — interleaved: pos(3) + normal(3) + color(3) = 9 floats
// Matches: layout(location=0) pos, (1) normal, (2) color in shaders
// ─────────────────────────────────────────────────────────────────────────────
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

// ─────────────────────────────────────────────────────────────────────────────
// Mesh — owns a VAO/VBO/EBO. Supports triangles, lines, and points.
// ─────────────────────────────────────────────────────────────────────────────
class Mesh {
public:
    GLuint vao         = 0;
    GLuint vbo         = 0;
    GLuint ebo         = 0;
    int    indexCount  = 0;
    int    vertexCount = 0;

    Mesh() = default;
    ~Mesh();

    // Non-copyable, movable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept;
    Mesh& operator=(Mesh&& o) noexcept;

    // Upload indexed geometry
    void upload(const std::vector<Vertex>& verts,
                const std::vector<unsigned int>& indices);

    // Upload unindexed points (e.g. starfield, dust)
    void uploadPoints(const std::vector<Vertex>& verts);

    // Dynamic line buffer for HUD (re-uploads to existing VBO/EBO or creates)
    void updateLines(const std::vector<Vertex>& verts,
                     const std::vector<unsigned int>& indices);

    // Draw methods
    void draw() const;        // GL_TRIANGLES
    void drawPoints() const;  // GL_POINTS
    void drawLines() const;   // GL_LINES

    // ── Primitive & Spacecraft Factories ──────────────────────────────────────
    static Mesh createBox(float w, float h, float d, const glm::vec3& color = {0.8f, 0.8f, 0.8f});
    static Mesh createSphere(float radius, int sectors = 64, int rings = 32, const glm::vec3& color = {1.f, 1.f, 1.f});
    static Mesh createCylinder(float radius, float height, int sectors = 32, const glm::vec3& color = {0.85f, 0.85f, 0.88f});
    static Mesh createAsteroid(float radius, int sectors = 36, int rings = 20, unsigned int seed = 42);

    static Mesh createSpaceStation();
    static Mesh createSatellite();
    static Mesh createStarfield(int count = 5000, float radius = 800.0f);
    static Mesh createSpaceDust(int count = 1500, float boxExtent = 40.0f);
    static Mesh createLineStrip(const std::vector<glm::vec3>& pts,
                                const glm::vec3& color = {1.f, 1.f, 1.f});
};
