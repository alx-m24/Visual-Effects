#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 color;
};

uniform DirLight dirLight;

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, float shininess) {
    light.ambient *= light.color;
    light.diffuse *= light.color;
    light.specular *= light.color;

    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * color;
    return (ambient + diffuse + specular);
}

void main()
{             
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;   

    vec3 terrainColor = calculateDirLight(dirLight, Normal, normalize(viewPos - FragPos), Albedo, 33.0);

    float gridPhaseOffsetX = 1.0f;
    float gridPhaseOffsetZ = 1.0f;
    float spacing = 2.0f;
    float lineWidth = 1.0f;
    vec3 gridColor = vec3(1.0f);

    vec3 worldPos = FragPos + Normal * 1.0f;
    vec3 offsetPos = worldPos + vec3(gridPhaseOffsetX, 0.0, gridPhaseOffsetZ);
    float lineX = abs(fract(offsetPos.x / spacing) - 0.5);
    float lineZ = abs(fract(offsetPos.z / spacing) - 0.5);
    float line = min(lineX, lineZ);
    float gridIntensity = smoothstep(0.0, lineWidth, line);
    vec3 finalColor = mix(terrainColor, gridColor, gridIntensity); 
    
    FragColor = vec4(finalColor, 1.0);
}  