#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

/**
 * Shader — compiles and links a GLSL vertex+fragment program.
 * Usage:
 *   Shader s("shaders/object.vert", "shaders/object.frag");
 *   s.use();
 *   s.setMat4("uMVP", mvp);
 */
class Shader {
public:
    GLuint id = 0;

    Shader() = default;
    Shader(const char* vertPath, const char* fragPath);
    ~Shader();

    // Non-copyable, movable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& o) noexcept : id(o.id) { o.id = 0; }

    void use() const { glUseProgram(id); }

    // Uniform setters — all look up by name each call (fast enough for our use)
    void setInt  (const char* name, int v)              const;
    void setFloat(const char* name, float v)             const;
    void setVec3 (const char* name, const glm::vec3& v) const;
    void setVec4 (const char* name, const glm::vec4& v) const;
    void setMat3 (const char* name, const glm::mat3& m) const;
    void setMat4 (const char* name, const glm::mat4& m) const;

private:
    static GLuint      compileShader(GLenum type, const char* src, const char* label);
    static std::string loadFile(const char* path);
};
