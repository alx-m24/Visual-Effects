#version 430 core

out vec4 FragColor;

in vec3 position;
uniform vec2 patternOrigin;  // center of concentric pattern
uniform float spacing;       // distance between rings
uniform float thickness;     // how thick each ring is
uniform vec4 color1;         // ring color
uniform vec4 color2;         // background color

void main()
{
    // distance from fragment to the origin
    float dist = length(position.xz - patternOrigin);

    

    // create thin ring shape within the spacing
    float modDist = mod(dist, spacing);
    float ringMask = smoothstep(0.0, thickness, modDist) 
                   * (1.0 - smoothstep(thickness, thickness * 1.2, modDist));

    // mix background and ring color
    FragColor = mix(color2, color1, ringMask);
}
