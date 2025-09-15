#version 450 core

out vec4 FragColor;

in vec3 FragPos; 
in vec2 TexCoords;
in mat3 TBN;  // tangent -> world space basis

struct DirLight {
    vec3 direction;
    vec3 color;
    vec3 ambient;
};

struct Material {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;
};

uniform DirLight dirLight;
uniform Material material;
uniform vec3 viewPos;

// ----------------------------------------------------------------------------
// Helper: normal distribution (GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159 * denom * denom);
}

// ----------------------------------------------------------------------------
// Helper: geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
// Helper: Fresnel Schlick
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
// Core BSDF (Cook–Torrance microfacet)
vec3 bsdf(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, vec3 lightColor) {
    vec3 H = normalize(V + L);

    // Fresnel reflectance at normal incidence
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Cook–Torrance
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular     = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / 3.14159 + specular) * lightColor * NdotL;
}

// ----------------------------------------------------------------------------
void main() {
    // --- Textures ---
    vec3 albedo    = pow(texture(material.albedoMap, TexCoords).rgb, vec3(2.2)); // gamma -> linear
    float metallic = texture(material.metallicMap, TexCoords).r;
    float roughness= texture(material.roughnessMap, TexCoords).r;
    float ao       = texture(material.aoMap, TexCoords).r;

    // Normal mapping
    vec3 N = texture(material.normalMap, TexCoords).rgb;
    N = normalize(N * 2.0 - 1.0);
    N = normalize(TBN * N);

    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(-dirLight.direction);

    // --- BSDF lighting ---
    vec3 radiance = bsdf(N, V, L, albedo, metallic, roughness, dirLight.color);

    // add ambient via AO
    vec3 ambient = dirLight.ambient * albedo * ao;

    vec3 color = ambient + radiance;

    // gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
