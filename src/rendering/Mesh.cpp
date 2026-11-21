#include "Mesh.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Noise helper for procedural geometry (fast 3D hash & value noise)
// ─────────────────────────────────────────────────────────────────────────────
namespace {
    inline float fract(float x) { return x - std::floor(x); }

    inline float hash3D(float x, float y, float z) {
        float fx = fract(x * 0.3183099f + 0.1f);
        float fy = fract(y * 0.3183099f + 0.2f);
        float fz = fract(z * 0.3183099f + 0.3f);
        float p = (fx * 17.0f) * (fy * 17.0f) * (fz * 17.0f);
        return fract(p * (fx + fy + fz));
    }

    float noise3D(float x, float y, float z) {
        float ix = std::floor(x), fx = x - ix;
        float iy = std::floor(y), fy = y - iy;
        float iz = std::floor(z), fz = z - iz;

        fx = fx * fx * (3.0f - 2.0f * fx);
        fy = fy * fy * (3.0f - 2.0f * fy);
        fz = fz * fz * (3.0f - 2.0f * fz);

        float n000 = hash3D(ix,     iy,     iz);
        float n100 = hash3D(ix + 1, iy,     iz);
        float n010 = hash3D(ix,     iy + 1, iz);
        float n110 = hash3D(ix + 1, iy + 1, iz);
        float n001 = hash3D(ix,     iy,     iz + 1);
        float n101 = hash3D(ix + 1, iy,     iz + 1);
        float n011 = hash3D(ix,     iy + 1, iz + 1);
        float n111 = hash3D(ix + 1, iy + 1, iz + 1);

        float x00 = n000 + fx * (n100 - n000);
        float x10 = n010 + fx * (n110 - n010);
        float x01 = n001 + fx * (n101 - n001);
        float x11 = n011 + fx * (n111 - n011);

        float y0 = x00 + fy * (x10 - x00);
        float y1 = x01 + fy * (x11 - x01);

        return y0 + fz * (y1 - y0);
    }

    // Append a box with outward CCW faces
    void appendBox(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                   const glm::vec3& center, const glm::vec3& size, const glm::vec3& color) {
        float hw = size.x * 0.5f, hh = size.y * 0.5f, hd = size.z * 0.5f;

        struct Face { glm::vec3 normal; glm::vec3 v[4]; };
        Face faces[6] = {
            // +Z Front
            {{ 0, 0, 1}, {center + glm::vec3(-hw,-hh, hd), center + glm::vec3( hw,-hh, hd),
                          center + glm::vec3( hw, hh, hd), center + glm::vec3(-hw, hh, hd)}},
            // -Z Back
            {{ 0, 0,-1}, {center + glm::vec3( hw,-hh,-hd), center + glm::vec3(-hw,-hh,-hd),
                          center + glm::vec3(-hw, hh,-hd), center + glm::vec3( hw, hh,-hd)}},
            // +X Right
            {{ 1, 0, 0}, {center + glm::vec3( hw,-hh, hd), center + glm::vec3( hw,-hh,-hd),
                          center + glm::vec3( hw, hh,-hd), center + glm::vec3( hw, hh, hd)}},
            // -X Left
            {{-1, 0, 0}, {center + glm::vec3(-hw,-hh,-hd), center + glm::vec3(-hw,-hh, hd),
                          center + glm::vec3(-hw, hh, hd), center + glm::vec3(-hw, hh,-hd)}},
            // +Y Top
            {{ 0, 1, 0}, {center + glm::vec3(-hw, hh, hd), center + glm::vec3( hw, hh, hd),
                          center + glm::vec3( hw, hh,-hd), center + glm::vec3(-hw, hh,-hd)}},
            // -Y Bottom
            {{ 0,-1, 0}, {center + glm::vec3(-hw,-hh,-hd), center + glm::vec3( hw,-hh,-hd),
                          center + glm::vec3( hw,-hh, hd), center + glm::vec3(-hw,-hh, hd)}}
        };

        for (int f = 0; f < 6; ++f) {
            unsigned int base = static_cast<unsigned int>(verts.size());
            for (int i = 0; i < 4; ++i) {
                verts.push_back({faces[f].v[i], faces[f].normal, color});
            }
            indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
            indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
        }
    }

    // Append cylinder between two arbitrary points p0 and p1
    void appendCylinder(std::vector<Vertex>& verts, std::vector<unsigned int>& indices,
                        const glm::vec3& p0, const glm::vec3& p1, float radius,
                        const glm::vec3& color, int sectors = 24) {
        glm::vec3 d = p1 - p0;
        float h = glm::length(d);
        if (h < 1e-5f) return;
        glm::vec3 a = d / h;

        glm::vec3 u = (std::abs(a.y) < 0.9f) ? glm::cross(a, glm::vec3(0, 1, 0))
                                              : glm::cross(a, glm::vec3(1, 0, 0));
        u = glm::normalize(u);
        glm::vec3 v = glm::cross(a, u);

        unsigned int base = static_cast<unsigned int>(verts.size());

        // Side vertices: bottom ring followed by top ring
        for (int s = 0; s <= sectors; ++s) {
            float th = (float)s * 6.2831853f / (float)sectors;
            glm::vec3 rad = std::cos(th) * u + std::sin(th) * v;
            verts.push_back({p0 + rad * radius, rad, color});
        }
        for (int s = 0; s <= sectors; ++s) {
            float th = (float)s * 6.2831853f / (float)sectors;
            glm::vec3 rad = std::cos(th) * u + std::sin(th) * v;
            verts.push_back({p1 + rad * radius, rad, color});
        }

        // Side indices
        for (int s = 0; s < sectors; ++s) {
            unsigned int first  = base + s;
            unsigned int second = base + (sectors + 1) + s;
            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);

            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second + 1);
        }

        // Top cap (centered at p1, normal +a)
        unsigned int topCenter = static_cast<unsigned int>(verts.size());
        verts.push_back({p1, a, color});
        unsigned int topRing = static_cast<unsigned int>(verts.size());
        for (int s = 0; s < sectors; ++s) {
            float th = (float)s * 6.2831853f / (float)sectors;
            glm::vec3 rad = std::cos(th) * u + std::sin(th) * v;
            verts.push_back({p1 + rad * radius, a, color});
        }
        for (int s = 0; s < sectors; ++s) {
            indices.push_back(topCenter);
            indices.push_back(topRing + s);
            indices.push_back(topRing + ((s + 1) % sectors));
        }

        // Bottom cap (centered at p0, normal -a)
        unsigned int botCenter = static_cast<unsigned int>(verts.size());
        verts.push_back({p0, -a, color});
        unsigned int botRing = static_cast<unsigned int>(verts.size());
        for (int s = 0; s < sectors; ++s) {
            float th = (float)s * 6.2831853f / (float)sectors;
            glm::vec3 rad = std::cos(th) * u + std::sin(th) * v;
            verts.push_back({p0 + rad * radius, -a, color});
        }
        for (int s = 0; s < sectors; ++s) {
            indices.push_back(botCenter);
            indices.push_back(botRing + ((s + 1) % sectors));
            indices.push_back(botRing + s);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Move semantics
// ─────────────────────────────────────────────────────────────────────────────
Mesh::Mesh(Mesh&& o) noexcept
    : vao(o.vao), vbo(o.vbo), ebo(o.ebo), indexCount(o.indexCount), vertexCount(o.vertexCount) {
    o.vao = o.vbo = o.ebo = 0;
    o.indexCount = o.vertexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
        vao         = o.vao;
        vbo         = o.vbo;
        ebo         = o.ebo;
        indexCount  = o.indexCount;
        vertexCount = o.vertexCount;
        o.vao = o.vbo = o.ebo = 0;
        o.indexCount = o.vertexCount = 0;
    }
    return *this;
}

Mesh::~Mesh() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
}

// ─────────────────────────────────────────────────────────────────────────────
// Upload & GPU buffers
// ─────────────────────────────────────────────────────────────────────────────
void Mesh::upload(const std::vector<Vertex>& verts,
                  const std::vector<unsigned int>& indices) {
    vertexCount = static_cast<int>(verts.size());
    indexCount  = static_cast<int>(indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)),
                 verts.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(),
                 GL_STATIC_DRAW);

    const GLsizei stride = sizeof(Vertex);

    // attrib 0: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(uintptr_t)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // attrib 1: normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(uintptr_t)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // attrib 2: color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(uintptr_t)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::uploadPoints(const std::vector<Vertex>& verts) {
    vertexCount = static_cast<int>(verts.size());
    indexCount  = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)),
                 verts.data(),
                 GL_STATIC_DRAW);

    const GLsizei stride = sizeof(Vertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(uintptr_t)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(uintptr_t)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(uintptr_t)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::updateLines(const std::vector<Vertex>& verts,
                       const std::vector<unsigned int>& indices) {
    vertexCount = static_cast<int>(verts.size());
    indexCount  = static_cast<int>(indices.size());
    if (vertexCount == 0) return;

    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)),
                     verts.data(),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     indices.data(),
                     GL_DYNAMIC_DRAW);

        const GLsizei stride = sizeof(Vertex);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                              (void*)(uintptr_t)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                              (void*)(uintptr_t)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                              (void*)(uintptr_t)offsetof(Vertex, color));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)),
                     verts.data(),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     indices.data(),
                     GL_DYNAMIC_DRAW);
    }
}

void Mesh::draw() const {
    if (!vao || indexCount <= 0) return;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::drawPoints() const {
    if (!vao || vertexCount <= 0) return;
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, vertexCount);
    glBindVertexArray(0);
}

void Mesh::drawLines() const {
    if (!vao || indexCount <= 0) return;
    glBindVertexArray(vao);
    glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Factories
// ─────────────────────────────────────────────────────────────────────────────
Mesh Mesh::createBox(float w, float h, float d, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    appendBox(verts, indices, glm::vec3(0.0f), glm::vec3(w, h, d), color);
    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}

Mesh Mesh::createSphere(float radius, int sectors, int rings, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    verts.reserve((rings + 1) * (sectors + 1));

    for (int r = 0; r <= rings; ++r) {
        float v = (float)r / (float)rings;
        float phi = 3.14159265f * v;
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        for (int s = 0; s <= sectors; ++s) {
            float u = (float)s / (float)sectors;
            float theta = 6.2831853f * u;
            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);

            glm::vec3 n(cosTheta * sinPhi, cosPhi, sinTheta * sinPhi);
            glm::vec3 pos = n * radius;
            verts.push_back({pos, n, color});
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            unsigned int first  = r * (sectors + 1) + s;
            unsigned int second = first + sectors + 1;

            if (r == 0) {
                indices.push_back(first);
                indices.push_back(second + 1);
                indices.push_back(second);
            } else if (r == rings - 1) {
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second);
            } else {
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second);

                indices.push_back(first + 1);
                indices.push_back(second + 1);
                indices.push_back(second);
            }
        }
    }

    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}

Mesh Mesh::createCylinder(float radius, float height, int sectors, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    appendCylinder(verts, indices,
                   glm::vec3(0, -height * 0.5f, 0),
                   glm::vec3(0,  height * 0.5f, 0),
                   radius, color, sectors);
    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}

Mesh Mesh::createAsteroid(float radius, int sectors, int rings, unsigned int seed) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;

    float sOffset = (float)(seed % 1000) * 0.17f;

    for (int r = 0; r <= rings; ++r) {
        float v = (float)r / (float)rings;
        float phi = 3.14159265f * v;
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        for (int s = 0; s <= sectors; ++s) {
            float u = (float)s / (float)sectors;
            float theta = 6.2831853f * u;
            glm::vec3 dir(std::cos(theta) * sinPhi, cosPhi, std::sin(theta) * sinPhi);

            // Multi-octave 3D noise deformation
            float n1 = noise3D(dir.x * 2.2f + sOffset, dir.y * 2.2f, dir.z * 2.2f);
            float n2 = noise3D(dir.x * 4.8f, dir.y * 4.8f + sOffset, dir.z * 4.8f) * 0.5f;
            float n3 = noise3D(dir.x * 9.5f + sOffset, dir.y * 9.5f, dir.z * 9.5f) * 0.25f;

            float disp = (n1 + n2 + n3 - 0.85f);
            float actualR = radius * (1.0f + 0.38f * disp);

            // Shading: rocky grey-brown modulated by height
            float shade = 0.32f + 0.22f * (disp + 0.4f);
            glm::vec3 rockColor(shade * 1.05f, shade * 0.98f, shade * 0.92f);

            verts.push_back({dir * actualR, dir, rockColor});
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            unsigned int first  = r * (sectors + 1) + s;
            unsigned int second = first + sectors + 1;

            if (r == 0) {
                indices.push_back(first);
                indices.push_back(second + 1);
                indices.push_back(second);
            } else if (r == rings - 1) {
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second);
            } else {
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second);

                indices.push_back(first + 1);
                indices.push_back(second + 1);
                indices.push_back(second);
            }
        }
    }

    // Recompute smooth face-averaged vertex normals
    for (auto& v : verts) v.normal = glm::vec3(0.0f);
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];
        glm::vec3 fn = glm::cross(verts[i1].position - verts[i0].position,
                                  verts[i2].position - verts[i0].position);
        verts[i0].normal += fn;
        verts[i1].normal += fn;
        verts[i2].normal += fn;
    }
    for (auto& v : verts) {
        if (glm::length(v.normal) > 1e-5f)
            v.normal = glm::normalize(v.normal);
    }

    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}

Mesh Mesh::createSpaceStation() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;

    const glm::vec3 whiteHull (0.92f, 0.92f, 0.94f);
    const glm::vec3 darkMetal (0.28f, 0.30f, 0.32f);
    const glm::vec3 goldTruss (0.75f, 0.65f, 0.30f);
    const glm::vec3 solarBlue (0.07f, 0.16f, 0.42f);
    const glm::vec3 solarGold (0.85f, 0.68f, 0.18f);

    // 1. Central Core Habitat Module (along Z: -7m to +7m)
    appendCylinder(verts, indices, glm::vec3(0, 0, -7.0f), glm::vec3(0, 0, 7.0f), 1.35f, whiteHull, 28);

    // 2. Transverse Node Airlock (along X: -3m to +3m at z = 3m)
    appendCylinder(verts, indices, glm::vec3(-3.2f, 0, 3.0f), glm::vec3(3.2f, 0, 3.0f), 1.15f, whiteHull, 24);

    // 3. Forward Docking Port Ring (z = +7.2m)
    appendCylinder(verts, indices, glm::vec3(0, 0, 7.0f), glm::vec3(0, 0, 7.5f), 1.45f, darkMetal, 24);
    appendBox(verts, indices, glm::vec3(0, 0, 7.52f), glm::vec3(0.12f, 1.2f, 0.05f), glm::vec3(1.0f, 0.85f, 0.1f));
    appendBox(verts, indices, glm::vec3(0, 0, 7.52f), glm::vec3(1.2f, 0.12f, 0.05f), glm::vec3(1.0f, 0.85f, 0.1f));

    // 4. Aft Docking Port Ring (z = -7.2m)
    appendCylinder(verts, indices, glm::vec3(0, 0, -7.5f), glm::vec3(0, 0, -7.0f), 1.45f, darkMetal, 24);

    // 5. Cupola Observation Dome (at z = -2.5m, y = -1.5m facing planet)
    appendCylinder(verts, indices, glm::vec3(0, -1.3f, -2.5f), glm::vec3(0, -1.8f, -2.5f), 0.8f, glm::vec3(0.06f, 0.12f, 0.28f), 16);

    // 6. Integrated Truss Structure (along X: span 34m at y = 1.6m, z = 0m)
    appendBox(verts, indices, glm::vec3(0, 1.6f, 0), glm::vec3(34.0f, 0.65f, 0.65f), goldTruss);

    // 7. Rotary joints for solar arrays
    appendCylinder(verts, indices, glm::vec3(-11.0f, 1.6f, -0.6f), glm::vec3(-11.0f, 1.6f, 0.6f), 0.45f, darkMetal, 16);
    appendCylinder(verts, indices, glm::vec3( 11.0f, 1.6f, -0.6f), glm::vec3( 11.0f, 1.6f, 0.6f), 0.45f, darkMetal, 16);

    // 8. Solar Array Wings (4 massive wings, span 8.5m each, width 3.2m)
    // Starboard (+X): Upper & Lower
    appendBox(verts, indices, glm::vec3( 12.8f,  6.0f, 0), glm::vec3(3.2f, 8.5f, 0.08f), solarBlue);
    appendBox(verts, indices, glm::vec3( 12.8f,  6.0f, 0), glm::vec3(3.35f, 8.65f, 0.04f), solarGold);
    appendBox(verts, indices, glm::vec3( 12.8f, -6.0f, 0), glm::vec3(3.2f, 8.5f, 0.08f), solarBlue);
    appendBox(verts, indices, glm::vec3( 12.8f, -6.0f, 0), glm::vec3(3.35f, 8.65f, 0.04f), solarGold);

    // Port (-X): Upper & Lower
    appendBox(verts, indices, glm::vec3(-12.8f,  6.0f, 0), glm::vec3(3.2f, 8.5f, 0.08f), solarBlue);
    appendBox(verts, indices, glm::vec3(-12.8f,  6.0f, 0), glm::vec3(3.35f, 8.65f, 0.04f), solarGold);
    appendBox(verts, indices, glm::vec3(-12.8f, -6.0f, 0), glm::vec3(3.2f, 8.5f, 0.08f), solarBlue);
    appendBox(verts, indices, glm::vec3(-12.8f, -6.0f, 0), glm::vec3(3.35f, 8.65f, 0.04f), solarGold);

    // 9. Thermal Control Radiator Panels (white panels oriented perpendicular to sun)
    appendBox(verts, indices, glm::vec3( 5.5f, 3.8f, 0), glm::vec3(0.08f, 4.0f, 2.5f), whiteHull);
    appendBox(verts, indices, glm::vec3(-5.5f, 3.8f, 0), glm::vec3(0.08f, 4.0f, 2.5f), whiteHull);

    // 10. Navigation Beacon Strobes (values > 2.0 trigger blinking shader)
    // Port wingtip: Red Strobe (1 Hz)
    appendBox(verts, indices, glm::vec3(-17.2f, 1.6f, 0), glm::vec3(0.35f, 0.35f, 0.35f), glm::vec3(5.0f, 0.1f, 0.1f));
    // Starboard wingtip: Green Strobe (1 Hz)
    appendBox(verts, indices, glm::vec3( 17.2f, 1.6f, 0), glm::vec3(0.35f, 0.35f, 0.35f), glm::vec3(0.1f, 5.0f, 0.2f));
    // Forward / Aft: White Strobe (2 Hz)
    appendBox(verts, indices, glm::vec3(0, 1.8f,  7.2f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(5.0f, 5.0f, 5.0f));
    appendBox(verts, indices, glm::vec3(0, 1.8f, -7.2f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(5.0f, 4.0f, 0.3f));

    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}

Mesh Mesh::createSatellite() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;

    const glm::vec3 goldFoil (0.92f, 0.74f, 0.18f);
    const glm::vec3 antennaWh(0.95f, 0.95f, 0.96f);
    const glm::vec3 darkMetal(0.25f, 0.27f, 0.30f);
    const glm::vec3 solarBlue(0.08f, 0.18f, 0.44f);

    // 1. Satellite Bus (Gold MLI Kapton foil insulated body)
    appendBox(verts, indices, glm::vec3(0), glm::vec3(1.4f, 1.8f, 1.4f), goldFoil);

    // 2. High-gain Parabolic Dish Antenna
    appendCylinder(verts, indices, glm::vec3(0, 0.9f, 0), glm::vec3(0, 1.6f, 0.4f), 0.15f, darkMetal, 12);
    // Parabolic dish (shallow inverted cone/cylinder)
    appendCylinder(verts, indices, glm::vec3(0, 1.6f, 0.4f), glm::vec3(0, 1.9f, 0.8f), 1.25f, antennaWh, 24);
    // Sub-reflector feed horn
    appendBox(verts, indices, glm::vec3(0, 2.2f, 1.0f), glm::vec3(0.15f, 0.15f, 0.25f), darkMetal);

    // 3. Deployable Solar Wings (+X and -X)
    appendBox(verts, indices, glm::vec3( 2.8f, 0, 0), glm::vec3(3.2f, 1.1f, 0.06f), solarBlue);
    appendBox(verts, indices, glm::vec3(-2.8f, 0, 0), glm::vec3(3.2f, 1.1f, 0.06f), solarBlue);

    // 4. RCS thruster blocks on 4 corners
    appendBox(verts, indices, glm::vec3( 0.72f, 0.8f,  0.72f), glm::vec3(0.15f, 0.15f, 0.15f), darkMetal);
    appendBox(verts, indices, glm::vec3(-0.72f, 0.8f,  0.72f), glm::vec3(0.15f, 0.15f, 0.15f), darkMetal);
    appendBox(verts, indices, glm::vec3( 0.72f, 0.8f, -0.72f), glm::vec3(0.15f, 0.15f, 0.15f), darkMetal);
    appendBox(verts, indices, glm::vec3(-0.72f, 0.8f, -0.72f), glm::vec3(0.15f, 0.15f, 0.15f), darkMetal);

    // 5. Strobe beacon on top
    appendBox(verts, indices, glm::vec3(0, 1.0f, -0.6f), glm::vec3(0.18f, 0.18f, 0.18f), glm::vec3(5.0f, 5.0f, 5.0f));

    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}

Mesh Mesh::createStarfield(int count, float radius) {
    std::vector<Vertex> verts;
    verts.reserve(count);

    // Fibonacci sphere distribution for uniform celestial coverage
    for (int i = 0; i < count; ++i) {
        float z = 1.0f - (2.0f * (float)i) / (float)(count - 1);
        float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
        float phi = (float)i * 2.39996323f; // Golden ratio spiral angle
        float x = r * std::cos(phi);
        float y = r * std::sin(phi);

        glm::vec3 pos(x * radius, y * radius, z * radius);

        // Star spectral classes
        float type = hash3D((float)i, 42.0f, 17.0f);
        glm::vec3 baseCol;
        if (type < 0.15f) {
            baseCol = glm::vec3(0.72f, 0.85f, 1.0f); // Class O/B (Blue-white)
        } else if (type < 0.65f) {
            baseCol = glm::vec3(0.96f, 0.98f, 1.0f); // Class A/F (Pure white)
        } else if (type < 0.88f) {
            baseCol = glm::vec3(1.0f, 0.95f, 0.75f);  // Class G (Yellow / Solar)
        } else {
            baseCol = glm::vec3(1.0f, 0.62f, 0.38f);  // Class K/M (Orange / Red giant)
        }

        // Stellar brightness & size
        float mag = hash3D(13.0f, (float)i, 99.0f);
        float brightness = std::pow(mag, 2.8f) * 1.5f + 0.3f;
        float pointSize = 1.2f + 2.8f * mag;
        float twinkleSpeed = 1.0f + 3.0f * hash3D((float)i, 7.0f, 31.0f);
        float twinklePhase = 6.28f * hash3D((float)i, 83.0f, 2.0f);

        verts.push_back({pos, glm::vec3(pointSize, twinkleSpeed, twinklePhase), baseCol * brightness});
    }

    Mesh mesh;
    mesh.uploadPoints(verts);
    return mesh;
}

Mesh Mesh::createSpaceDust(int count, float boxExtent) {
    std::vector<Vertex> verts;
    verts.reserve(count);

    float h = boxExtent * 0.5f;
    for (int i = 0; i < count; ++i) {
        float x = (hash3D((float)i * 3.1f, 1.0f, 5.0f) * 2.0f - 1.0f) * h;
        float y = (hash3D(2.0f, (float)i * 4.7f, 9.0f) * 2.0f - 1.0f) * h;
        float z = (hash3D(7.0f, 3.0f, (float)i * 6.3f) * 2.0f - 1.0f) * h;

        float bright = 0.5f + 0.5f * hash3D((float)i, 11.0f, 19.0f);
        glm::vec3 dustCol(0.85f * bright, 0.92f * bright, 1.0f * bright);

        verts.push_back({glm::vec3(x, y, z), glm::vec3(0, 1, 0), dustCol});
    }

    Mesh mesh;
    mesh.uploadPoints(verts);
    return mesh;
}

Mesh Mesh::createLineStrip(const std::vector<glm::vec3>& pts, const glm::vec3& color) {
    std::vector<Vertex> verts;
    verts.reserve(pts.size());
    for (auto& p : pts) {
        verts.push_back({p, {0,1,0}, color});
    }
    std::vector<unsigned int> indices(pts.size());
    for (unsigned int i = 0; i < indices.size(); ++i) indices[i] = i;

    Mesh mesh;
    mesh.upload(verts, indices);
    return mesh;
}
