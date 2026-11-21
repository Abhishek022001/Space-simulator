#version 330 core

in vec3 vNormal;
in vec3 vLocalPos;
in vec3 vFragPos;

out vec4 FragColor;

uniform vec3 uLightDir; // direction FROM light
uniform vec3 uViewPos;  // camera world position
uniform float uTime;

// ── 3D Procedural Hash & Noise ───────────────────────────────────────────────
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

float fbm(vec3 p) {
    float v = 0.0;
    float a = 0.5;
    mat3 rot = mat3(
        0.00,  0.80,  0.60,
       -0.80,  0.36, -0.48,
       -0.60, -0.48,  0.64
    );
    for (int i = 0; i < 5; ++i) {
        v += a * noise(p);
        p = rot * p * 2.02;
        a *= 0.5;
    }
    return v;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uViewPos - vFragPos);
    vec3 H = normalize(L + V);

    // Coordinate on unit sphere
    vec3 P = normalize(vLocalPos);

    // Planetary rotation around Y axis
    float rotAngle = uTime * 0.015;
    float s = sin(rotAngle), c = cos(rotAngle);
    vec3 rotP = vec3(c * P.x + s * P.z, P.y, -s * P.x + c * P.z);

    // ── Continents vs Oceans ─────────────────────────────────────────────────
    float elev = fbm(rotP * 2.6);
    float oceanMask = smoothstep(0.47, 0.52, elev);

    vec3 surfaceColor;
    float specFactor = 0.0;

    if (oceanMask < 0.5) {
        // Ocean gradient: deep abyss to coastal shallows
        float depth = elev / 0.5;
        surfaceColor = mix(vec3(0.02, 0.08, 0.28), vec3(0.06, 0.28, 0.45), depth);
        specFactor = 0.85; // Mirror specular reflection on water
    } else {
        // Land biomes
        float landHeight = (elev - 0.5) / 0.5;
        vec3 coast = vec3(0.20, 0.42, 0.18);
        vec3 highland = vec3(0.45, 0.38, 0.20);
        vec3 rock = vec3(0.35, 0.28, 0.24);
        vec3 snow = vec3(0.92, 0.94, 0.96);

        if (landHeight < 0.3) {
            surfaceColor = mix(coast, highland, landHeight / 0.3);
        } else if (landHeight < 0.7) {
            surfaceColor = mix(highland, rock, (landHeight - 0.3) / 0.4);
        } else {
            surfaceColor = mix(rock, snow, (landHeight - 0.7) / 0.3);
        }
        specFactor = 0.05;
    }

    // ── Polar Ice Caps ───────────────────────────────────────────────────────
    float lat = abs(rotP.y);
    float iceCap = smoothstep(0.74, 0.82, lat + 0.05 * noise(rotP * 6.0));
    if (iceCap > 0.0) {
        surfaceColor = mix(surfaceColor, vec3(0.94, 0.96, 0.98), iceCap);
        specFactor = mix(specFactor, 0.3, iceCap);
    }

    // ── Dynamic Swirling Clouds ──────────────────────────────────────────────
    vec3 cloudCoord = rotP + vec3(uTime * 0.012, 0.0, uTime * 0.007);
    float cloudNoise = fbm(cloudCoord * 3.8);
    float clouds = smoothstep(0.48, 0.72, cloudNoise);

    // Cloud ground shadow
    vec3 shadowCoord = cloudCoord - L * 0.025;
    float cloudShadow = smoothstep(0.50, 0.74, fbm(shadowCoord * 3.8));
    surfaceColor *= (1.0 - 0.4 * cloudShadow * (1.0 - clouds));

    // Blend white clouds over surface
    surfaceColor = mix(surfaceColor, vec3(0.96, 0.96, 0.98), clouds * 0.92);

    // ── Lighting: Day / Night ────────────────────────────────────────────────
    float NdotL = dot(N, L);
    float dayLight = smoothstep(-0.08, 0.12, NdotL);
    float diff = max(NdotL, 0.0);

    vec3 ambient = 0.04 * surfaceColor;
    vec3 diffuse = diff * surfaceColor;

    // Specular (sun glint on ocean)
    float spec = pow(max(dot(N, H), 0.0), 64.0) * specFactor * dayLight;
    vec3 specular = spec * vec3(1.0, 0.95, 0.85);

    // ── Night Side City Lights ───────────────────────────────────────────────
    float night = 1.0 - dayLight;
    float cityNoise = pow(clamp(noise(rotP * 28.0), 0.0, 1.0), 6.0) * 12.0;
    // City lights appear on continents where it's not polar ice
    float canHaveCities = oceanMask * (1.0 - iceCap) * (1.0 - clouds);
    vec3 cityLights = canHaveCities * cityNoise * vec3(1.0, 0.78, 0.38) * night;

    // ── Rayleigh Atmospheric Limb Glow ───────────────────────────────────────
    float rim = 1.0 - max(dot(N, V), 0.0);
    float atmoLimb = pow(rim, 3.4);
    // Atmosphere is brighter on the sunlit hemisphere
    float atmoSunFactor = max(NdotL * 0.7 + 0.3, 0.08);
    vec3 atmoColor = vec3(0.18, 0.58, 1.0) * atmoLimb * atmoSunFactor * 1.3;

    vec3 finalColor = ambient + diffuse + specular + cityLights + atmoColor;
    FragColor = vec4(finalColor, 1.0);
}
