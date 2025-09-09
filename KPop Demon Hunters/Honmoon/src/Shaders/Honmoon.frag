#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Position;

uniform sampler2D gNormal;

void main()
{
    // distance from world origin in the XZ plane
    float dist = length(Position.xz);

    // parameters to tweak
    float ringSpacing = 2.0;   // how far apart rings are
    float ringThickness = 0.2; // thickness of each band

    // figure out distance within its nearest ring cycle
    float modDist = mod(dist, ringSpacing);

    // condition placeholder
    if (modDist < ringThickness) {
        FragColor = vec4(1.0);
    } else {
        FragColor = vec4(0.0);
        discard;
    }
}
