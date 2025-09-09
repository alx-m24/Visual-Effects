#version 450 core
layout (location = 0) in vec2 aTexCoord;

out vec2 TexCoords;
out vec3 Position;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoord;

    vec3 position = texture(gPosition, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).xyz;

    //position += normal * 0.5f;
    position.y += 0.5;

    Position = position;

    gl_Position = projection * view * model * vec4(position, 1.0);
}