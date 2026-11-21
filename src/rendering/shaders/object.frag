#version 330 core

in vec3 vNormal;
in vec3 vColor;
in vec3 vFragPos;

out vec4 FragColor;

uniform vec3 uLightDir; // direction FROM light
uniform vec3 uViewPos;  // camera world position
uniform float uTime;

void main() {
    // Check if this is an emissive beacon (strobe light)
    float maxC = max(vColor.r, max(vColor.g, vColor.b));
    if (maxC > 2.0) {
        // Flash rate depends on beacon color
        float flash = 1.0;
        if (vColor.r > 3.0 && vColor.g < 1.0) {
            // Port Red beacon: 1 Hz flash
            flash = step(0.5, fract(uTime * 1.0));
        } else if (vColor.g > 3.0 && vColor.r < 1.0) {
            // Starboard Green beacon: 1 Hz flash
            flash = step(0.5, fract(uTime * 1.0));
        } else {
            // White / Amber anti-collision strobe: 2 Hz double-flash
            float f = fract(uTime * 1.5);
            flash = (f < 0.15 || (f > 0.25 && f < 0.40)) ? 1.0 : 0.05;
        }
        FragColor = vec4((vColor / maxC) * (flash * 2.0 + 0.1), 1.0);
        return;
    }

    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uViewPos - vFragPos);
    vec3 H = normalize(L + V);

    // Harsh space ambient (minimal bounce in vacuum)
    vec3 ambient = 0.08 * vColor;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * vColor;

    // Material detection: solar panels have metallic cyan/blue specular
    bool isSolarPanel = (vColor.b > 0.25 && vColor.r < 0.15 && vColor.g < 0.25);
    float shininess = isSolarPanel ? 96.0 : 36.0;
    float specIntensity = isSolarPanel ? 1.2 : 0.35;
    vec3 specColor = isSolarPanel ? vec3(0.5, 0.7, 1.0) : vec3(0.9, 0.9, 0.9);

    float spec = pow(max(dot(N, H), 0.0), shininess) * specIntensity;
    vec3 specular = diff > 0.0 ? spec * specColor : vec3(0.0);

    // Subtle edge rim light (spacecraft silhouette definition)
    float rim = 1.0 - max(dot(N, V), 0.0);
    rim = pow(rim, 3.0) * 0.20;
    vec3 rimLight = rim * vec3(0.3, 0.5, 0.8);

    FragColor = vec4(ambient + diffuse + specular + rimLight, 1.0);
}
