#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aExtra; // x = point size, y = twinkle speed, z = twinkle phase
layout(location = 2) in vec3 aColor;

out vec3 vColor;

uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;

void main() {
    // Strip translation from view matrix: stars remain at optical infinity
    mat4 rotView = mat4(mat3(uView));
    vec4 pos = uProj * rotView * vec4(aPos, 1.0);

    // Place at far plane (z = w in clip space => depth = 1.0)
    gl_Position = pos.xyww;

    float twinkle = 0.82 + 0.18 * sin(uTime * aExtra.y + aExtra.z);
    vColor = aColor * twinkle;

    // Smooth point size
    float sz = aExtra.x * (0.9 + 0.2 * sin(uTime * (aExtra.y * 0.7) + aExtra.z));
    gl_PointSize = clamp(sz, 1.0, 6.0);
}
