#version 330 core

in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

uniform vec3 uViewPos;
uniform float uTime;

float hash(vec3 p) {
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), f.x),
            mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), f.x),
            mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), f.x), f.y),
        f.z);
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vFragPos);
    float NdotV = max(dot(N, V), 0.0);

    // Solar limb darkening
    float limb = pow(NdotV, 0.38);
    vec3 coreColor = vec3(1.0, 0.99, 0.94);
    vec3 rimColor  = vec3(1.0, 0.52, 0.10);
    vec3 sunColor  = mix(rimColor, coreColor, limb);

    // Surface convection granulation
    float gran = noise(N * 18.0 + vec3(uTime * 0.15)) * 0.08;
    sunColor *= (1.0 + gran);

    // Brilliant emissive HDR star output
    FragColor = vec4(sunColor * 1.6, 1.0);
}
