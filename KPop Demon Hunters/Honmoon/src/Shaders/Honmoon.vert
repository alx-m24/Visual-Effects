#version 330 core
layout(location = 0) in vec2 aPositionXZ;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform mat4 view;
uniform mat4 projection;

uniform float hoverHeight;

out vec3 Position;

void main()
{
    // World-space vertex
    vec3 worldPos = vec3(aPositionXZ.x, 0.0, aPositionXZ.y);

    // Project to clip space
    vec4 clipPos = projection * view * vec4(worldPos, 1.0);

    // NDC coordinates
    vec3 ndc = clipPos.xyz / clipPos.w;

    // Convert NDC [-1,1] to UV [0,1] for texture lookup
    vec2 uv = ndc.xy * 0.5 + 0.5;
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        worldPos.y = hoverHeight; // just a flat fallback
    }
    uv = clamp(uv, 0.0, 1.0);

    float dx = 0.01;
    float dy = 0.01;

    vec3 n00 = texture(gNormal, uv + vec2(-dx,-dy)).xyz;
    vec3 n10 = texture(gNormal, uv + vec2( dx,-dy)).xyz;
    vec3 n01 = texture(gNormal, uv + vec2(-dx, dy)).xyz;
    vec3 n11 = texture(gNormal, uv + vec2( dx, dy)).xyz;
    
    vec3 terrainNormal = normalize((n00 + n10 + n01 + n11)/4.0);

    // Sample terrain position
    vec3 terrainPos = texture(gPosition, uv).xyz;

    // Set vertex Y to terrain height + hover
    //worldPos.y = terrainPos.y + hoverHeight;
    vec3 offsetDir = normalize(mix(vec3(0,1,0), terrainNormal, 0.5));
    worldPos = terrainPos + hoverHeight * offsetDir;

    Position = worldPos;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
