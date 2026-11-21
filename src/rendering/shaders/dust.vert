#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 vColor;
out float vAlpha;

uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uCamPos;
uniform float uBoxExtent;

void main() {
    vec3 delta = aPos - uCamPos;
    float halfBox = uBoxExtent * 0.5;
    vec3 wrapped = mod(delta + vec3(halfBox), vec3(uBoxExtent)) - vec3(halfBox);
    vec3 worldPos = uCamPos + wrapped;

    float dist = length(wrapped);
    gl_Position = uProj * uView * vec4(worldPos, 1.0);

    // Perspective point size
    gl_PointSize = clamp(24.0 / max(dist, 0.4), 1.5, 4.5);

    // Fade out near the camera and near the bounds of the box
    float nearFade = smoothstep(0.8, 2.2, dist);
    float farFade  = smoothstep(halfBox, halfBox * 0.7, dist);
    vAlpha = nearFade * farFade * 0.75;
    vColor = aColor;
}
