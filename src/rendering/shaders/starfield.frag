#version 330 core

in vec3 vColor;
out vec4 FragColor;

void main() {
    vec2 c = gl_PointCoord - vec2(0.5);
    float distSq = dot(c, c);
    if (distSq > 0.25) discard;

    // Soft anti-aliased stellar core with faint glow
    float alpha = exp(-distSq * 10.0);
    FragColor = vec4(vColor, alpha);
}
