#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

struct Particle {
    vec4 position;      // xyz = pos, w = mass
    vec4 velocity;
    vec4 acceleration;
    vec4 jerk;
    vec4 predictedPosition;
    vec4 predictedVelocity;
    vec4 newAcceleration;
    vec4 newJerk;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

out float heat;

float GetRadius(float Mass){
    return Mass;
}

void main()
{
    Particle p = particles[gl_InstanceID];

    // Model transform: translate by particle position, scale by radius
    vec3 scaledPos = aPos * GetRadius(p.position.w); // scale mesh by radius
    vec3 worldPos = p.position.xyz + scaledPos;

    heat = p.velocity.w;

    FragPos = worldPos;
    Normal = normalize(aNormal);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
