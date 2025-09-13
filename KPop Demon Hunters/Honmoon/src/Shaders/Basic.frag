#version 450 core
layout (location = 2) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout(location = 0) out vec4 gColor;

//out vec4 FragColor;

in vec3 FragPos; 
in vec2 TexCoords;
in mat3 TBN;  // from vertex shader

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 color;
};

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_normal1;
    sampler2D texture_specular1;
    sampler2D texture_roughness1;
    sampler2D texture_metallic1;
    sampler2D texture_ao1;
};

uniform DirLight dirLight;
uniform Material material;
uniform vec3 viewPos;

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, vec3 specMap, float shininess) {
    light.ambient *= light.color;
    light.diffuse *= light.color;
    light.specular *= light.color;

    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * specMap; // multiply by spec map

    return ambient + diffuse + specular;
}

void main() {
    // sample albedo
    vec3 Albedo = texture(material.texture_diffuse1, TexCoords).rgb;

    // Normal map (tangent space -> world space)
    vec3 normal = texture(material.texture_normal1, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(TBN * normal);

    // view direction
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 SpecularMap = texture(material.texture_specular1, TexCoords).rgb;

    vec3 lighting = calculateDirLight(dirLight, normal, viewDir, Albedo, SpecularMap, 33.0);

    gPosition = FragPos;
    gNormal = normal;
    gColor = vec4(lighting, 1.0);
}
