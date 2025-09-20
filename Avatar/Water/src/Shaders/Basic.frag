#version 450 core

out vec4 FragColor;

in vec3 FragPos; 
in vec3 Normal;
flat in float heat;

struct DirLight {
    vec3 direction;
    vec3 color;
    vec3 ambient;
};

uniform DirLight dirLight;
uniform vec3 viewPos;

uniform float maxHeat;

vec3 HSVtoRGB(vec3 hsv) {
    float h = hsv.x; // 0..1
    float s = hsv.y; // 0..1
    float v = hsv.z; // 0..1

    vec3 rgb = vec3(0.0);
    float i = floor(h * 6.0);
    float f = h * 6.0 - i;
    float p = v * (1.0 - s);
    float q = v * (1.0 - f * s);
    float t = v * (1.0 - (1.0 - f) * s);

    int modI = int(mod(i, 6.0));
    if (modI == 0) rgb = vec3(v, t, p);
    else if (modI == 1) rgb = vec3(q, v, p);
    else if (modI == 2) rgb = vec3(p, v, t);
    else if (modI == 3) rgb = vec3(p, q, v);
    else if (modI == 4) rgb = vec3(t, p, v);
    else if (modI == 5) rgb = vec3(v, p, q);

    return rgb;
}


void main() {
    vec3 lightDirection =  normalize(FragPos - vec3(0.0f));

    // Normalize inputs
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection); // directional light points from light TO object
    vec3 viewDir = normalize(viewPos - FragPos);

    // --- Ambient ---
    vec3 ambient = dirLight.ambient * dirLight.color;

    // --- Diffuse ---
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * dirLight.color;

    // --- Specular (Phong) ---
    float shininess = 32.0; // adjust for "glossiness"
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * dirLight.color;

    // Combine results
    vec3 result = ambient + diffuse + specular;

    float clampledheat = clamp(heat / maxHeat, 0.0, 1.0);

    // Cold (blue) = hue 0.66, hot (red) = hue 0.0
    float hue = mix(0.66, 0.0, clampledheat);

    vec3 hsv = vec3(hue, 1.0, 1.0); // full saturation and value
    vec3 color = HSVtoRGB(hsv);

    //if (heat == 0.0) color = vec3(1.0);

    result *= color;

    FragColor = vec4(result, 1.0);
}
