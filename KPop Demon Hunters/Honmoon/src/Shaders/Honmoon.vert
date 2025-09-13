#version 330 core
layout(location = 0) in vec2 aTexCoords;

uniform mat4 view;
uniform mat4 projection;

uniform float hoverHeight;
uniform float yCamOffset;
uniform vec3 origin;
uniform vec3 size;

uniform float near;
uniform float far;

uniform sampler2D terrrainHeight;

out vec3 position;

vec3 getWorldPosition(){
    vec3 worldPos = origin + size * vec3(aTexCoords.x, 0.0, aTexCoords.y);

    vec2 texSize = vec2(textureSize(terrrainHeight, 0));
    vec2 texel = 1.0 / texSize;

    // Skip edges entirely
    if(aTexCoords.x <= texel.x || aTexCoords.x >= 1.0 - texel.x ||
       aTexCoords.y <= texel.y || aTexCoords.y >= 1.0 - texel.y)
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        return -vec3(1.0);
    }

    vec2 uv = aTexCoords;
    uv.y = 1.0 - uv.y;             // flip
    uv = clamp(uv + 0.5 * texel, texel, 1.0 - texel); // half-texel offset safely

    // Tangent spacing in world units
    float dx = size.x / texSize.x;
    float dz = size.z / texSize.y;

    float H0 = texture(terrrainHeight, uv).r * (far - near) + near;
    float Hx = texture(terrrainHeight, uv + vec2(texel.x, 0)).r * (far - near) + near;
    float Hz = texture(terrrainHeight, uv + vec2(0, texel.y)).r * (far - near) + near;

    H0 = far - H0;
    Hx = far - Hx;
    Hz = far - Hz;

    // Build tangent vectors with correct scale
    vec3 tangentX = vec3(dx, Hx - H0, 0.0);
    vec3 tangentZ = vec3(0.0, Hz - H0, dz);

    vec3 normal = normalize(cross(tangentZ, tangentX));

    worldPos.y = H0;
    worldPos += normal * hoverHeight;
    worldPos.y -= yCamOffset;

    return worldPos;
}

void main()
{
    position = getWorldPosition();

    if (position == vec3(-1.0)) return;

    gl_Position = projection * view * vec4(position, 1.0);
}
