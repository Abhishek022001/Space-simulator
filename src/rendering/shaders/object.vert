#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 vNormal;
out vec3 vColor;
out vec3 vFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos      = worldPos.xyz;
    // Normal matrix — correct for uniform-scale transforms
    vNormal       = normalize(mat3(transpose(inverse(uModel))) * aNormal);
    vColor        = aColor;
    gl_Position   = uProj * uView * worldPos;
}
