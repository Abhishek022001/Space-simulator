#version 330 core

in vec3 vColor;
in float vAlpha;
out vec4 FragColor;

void main() {
    vec2 c = gl_PointCoord - vec2(0.5);
    float dSq = dot(c, c);
    if (dSq > 0.25) discard;

    float edge = exp(-dSq * 8.0);
    FragColor = vec4(vColor, vAlpha * edge);
}
