cmake_minimum_required(VERSION 3.20)
project(SpaceSim VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─────────────────────────────────────────────────────────────────────────────
# Dependencies via FetchContent (downloads on first configure, cached after)
# ─────────────────────────────────────────────────────────────────────────────
include(FetchContent)

# GLFW 3.4 — windowing + input
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM 1.0.1 — header-only math (vectors, matrices, quaternions)
FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/releases/download/1.0.1/glm-1.0.1-light.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(glm)

# The light zip extracts so glm-src/ IS the glm/ folder (glm.hpp at its root).
# Copy glm-src/ into vendor/glm/glm/ at configure time so that
#   #include <glm/glm.hpp>
# resolves correctly when the include path is vendor/glm.
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${glm_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/vendor/glm/glm"
)

# ─────────────────────────────────────────────────────────────────────────────
# GLAD — vendored minimal loader (no Python, no external tools)
# ─────────────────────────────────────────────────────────────────────────────
add_library(glad STATIC
    vendor/glad/src/gl.c
)
target_include_directories(glad PUBLIC
    vendor/glad/include
)

# ─────────────────────────────────────────────────────────────────────────────
# SpaceSim executable — Phase 3: 6DOF quaternion camera
# ─────────────────────────────────────────────────────────────────────────────
add_executable(SpaceSim
    src/main.cpp
    src/rendering/Shader.cpp
    src/rendering/Mesh.cpp
    src/camera/Camera.cpp
)

target_include_directories(SpaceSim PRIVATE
    src
    vendor/glad/include
    vendor/glm          # vendor/glm/glm/glm.hpp => #include <glm/glm.hpp>
)

target_link_libraries(SpaceSim PRIVATE
    glfw
    glad
    opengl32   # Windows system OpenGL (provides GL 1.1 base functions directly)
)

# Output to a predictable location
set_target_properties(SpaceSim PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy shaders next to the exe after every build
add_custom_command(TARGET SpaceSim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/rendering/shaders"
        "$<TARGET_FILE_DIR:SpaceSim>/shaders"
    COMMENT "Copying shaders to output directory"
)

# Friendly build info
message(STATUS "=== SpaceSim Phase 3 ===")
message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ std    : ${CMAKE_CXX_STANDARD}")
