#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────
Shader::Shader(const char* vertPath, const char* fragPath) {
    std::string vertSrc = loadFile(vertPath);
    std::string fragSrc = loadFile(fragPath);

    if (vertSrc.empty() || fragSrc.empty()) {
        std::cerr << "[Shader] Aborting — could not load source files.\n";
        return;
    }

    GLuint vert = compileShader(GL_VERTEX_SHADER,   vertSrc.c_str(), vertPath);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str(), fragPath);

    id = glCreateProgram();
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    glLinkProgram(id);

    GLint ok = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        std::cerr << "[Shader] Link error (" << vertPath << " + " << fragPath << "):\n"
                  << log << "\n";
        glDeleteProgram(id);
        id = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (id) glDeleteProgram(id);
}

// ─────────────────────────────────────────────────────────────────────────────
// Uniform setters
// ─────────────────────────────────────────────────────────────────────────────
void Shader::setInt  (const char* n, int v)              const { glUniform1i (glGetUniformLocation(id, n), v); }
void Shader::setFloat(const char* n, float v)             const { glUniform1f (glGetUniformLocation(id, n), v); }
void Shader::setVec3 (const char* n, const glm::vec3& v)  const { glUniform3fv(glGetUniformLocation(id, n), 1, glm::value_ptr(v)); }
void Shader::setVec4 (const char* n, const glm::vec4& v)  const { glUniform4fv(glGetUniformLocation(id, n), 1, glm::value_ptr(v)); }
void Shader::setMat3 (const char* n, const glm::mat3& m)  const { glUniformMatrix3fv(glGetUniformLocation(id, n), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::setMat4 (const char* n, const glm::mat4& m)  const { glUniformMatrix4fv(glGetUniformLocation(id, n), 1, GL_FALSE, glm::value_ptr(m)); }

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────
GLuint Shader::compileShader(GLenum type, const char* src, const char* label) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::cerr << "[Shader] Compile error (" << label << "):\n" << log << "\n";
    }
    return s;
}

std::string Shader::loadFile(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[Shader] Cannot open: " << path << "\n";
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
