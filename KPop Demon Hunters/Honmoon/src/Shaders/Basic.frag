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
    sampler2D diffuse;
    sampler2D normal;
    sampler2D specular;
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
    vec3 Albedo = texture(material.diffuse, TexCoords).rgb;

    // sample normal, transform from [0,1] -> [-1,1]
    vec3 tangentNormal = texture(material.normal, TexCoords).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;

    // bring it into world space
    vec3 normal = normalize(TBN * tangentNormal);

    // view direction
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 SpecularMap = texture(material.specular, TexCoords).rgb;

    vec3 lighting = calculateDirLight(dirLight, normal, viewDir, Albedo, SpecularMap, 33.0);

    gPosition = FragPos;
    gNormal = normal;
    gColor = vec4(lighting, 1.0);
}
