#version 330 core
layout(location = 0) in vec2 aTexCoords;

uniform mat4 view;
uniform mat4 projection;

uniform float hoverHeight;
uniform float minHeight;
uniform float yCamOffset;
uniform vec3 origin;
uniform vec3 size;

uniform float near;
uniform float far;

uniform sampler2D terrrainHeight;

out vec3 position;

vec4 getSmoothedNormalAndHeightWithHover(vec2 uv, vec2 texel, float dx, float dz, float hover)
{
    vec3 sumNormal = vec3(0.0);
    float sumHeight = 0.0;
    int count = 0;

    for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
            vec2 offset = vec2(float(i), float(j)) * texel;
            float h = max(far - texture(terrrainHeight, uv + offset).r * (far - near) + near, minHeight);
            sumHeight += h;

            // Tangents for normal
            float hx = max(far - texture(terrrainHeight, uv + offset + vec2(texel.x, 0)).r * (far - near) + near, minHeight);
            float hz = max(far - texture(terrrainHeight, uv + offset + vec2(0, texel.y)).r * (far - near) + near, minHeight);

            vec3 tangentX = vec3(dx, hx - h, 0.0);
            vec3 tangentZ = vec3(0.0, hz - h, dz);
            sumNormal += normalize(cross(tangentZ, tangentX));

            count++;
        }
    }

    float avgHeight = sumHeight / float(count);
    vec3 avgNormal = normalize(sumNormal / float(count));

    return vec4(avgNormal, avgHeight);
}

vec3 getWorldPosition(vec2 uv, vec2 texel, float dx, float dz) {
    vec3 worldPos = origin + size * vec3(aTexCoords.x, 0.0, aTexCoords.y);

    vec3 sumNormal = vec3(0.0);
    float sumHeight = 0.0;
    int count = 0;

    // Loop over center + 4 neighbors (or 3x3 if you want)
    for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
            // For 5-point cross pattern, skip diagonals:
            if (abs(i) + abs(j) > 1) continue;

            vec2 offsetUV = uv + vec2(float(i), float(j)) * texel;
            vec4 nh = getSmoothedNormalAndHeightWithHover(offsetUV, texel, dx, dz, hoverHeight);

            sumNormal += vec3(nh.x, nh.y, -nh.z);
            sumHeight += nh.w;
            count++;
        }
    }

    vec3 avgNormal = normalize(sumNormal / float(count));
    float avgHeight = sumHeight / float(count);

    worldPos.y = avgHeight - yCamOffset;

    return worldPos + avgNormal * hoverHeight;
}



void main()
{
    vec2 texSize = vec2(textureSize(terrrainHeight, 0));
    vec2 texel = 1.0 / texSize;

    vec2 uv = aTexCoords;
    uv.y = 1.0 - uv.y;             // flip
    uv = clamp(uv + 0.5 * texel, texel, 1.0 - texel); // half-texel offset safely

    float dx = size.x / texSize.x;
    float dz = size.z / texSize.y;

    vec3 sumPos = vec3(0.0);
    int count = 0;

    // 3x3 loop to average neighbors
    for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
            vec2 offsetUV = uv + vec2(float(i), float(j)) * texel;

            vec3 pos = getWorldPosition(uv, texel, dx, dz);
            if (position == vec3(-1.0)) return;

            sumPos += getWorldPosition(offsetUV, texel, dx, dz);
            count++;
        }
    }

    position = sumPos / float(count);

    gl_Position = projection * view * vec4(position, 1.0);
}
