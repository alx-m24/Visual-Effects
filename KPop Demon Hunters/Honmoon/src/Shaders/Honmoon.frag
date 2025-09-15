#version 430 core

out vec4 FragColor;

in vec3 position;

uniform vec2  patternOrigin;   // center of concentric pattern
uniform float spacing;         // distance between rings
uniform float thickness;       // thickness of each ring
uniform float edgeSoftness;    // how smooth the edges are
uniform float time;            // animate offset (optional)

uniform float progress; // distance to start fade-out

uniform vec4 color1;           // ring color
uniform vec4 color2;           // background color

uniform vec4 golden1;
uniform vec4 golden2;

uniform vec4 red1;
uniform vec4 red2;

uniform float twinkle_period;
uniform float wave_period;
uniform float wavy_ness;

uniform float golden_period;
uniform float golden_progress;

// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

void main() {
    float goldFactor = clamp(golden_progress, 0.0, 1.0);    

    // combine with golden_progress for smooth blending
    if (goldFactor < 1.0) {
        float goldNoise = snoise(position.xz * mix(0.1, golden_period, goldFactor) + vec2(time * 0.5));
        goldNoise = clamp(goldNoise * 0.5 + 0.5, 0.0, 1.0); // map to [0,1]
    
        // scale noise by goldFactor to fade in gold
        goldFactor *= goldNoise;
    }
    
    // final colors
    vec4 mainColor = mix(color1, golden1, goldFactor);
    vec4 bgColor   = mix(color2, golden2, goldFactor);

    // base distance from fragment to origin
    float dist = length(position.xz - patternOrigin);

    // animate the noise field (optional)
    float n = snoise(position.xz * wave_period);
    
    // use noise to perturb the distance field itself
    dist += n * wavy_ness;   // tweak 0.3 for how wavy you want it

    // fractional position within ring cycle
    float modDist = mod(dist, spacing);

    // build smooth ring mask with controllable softness
    float inner = smoothstep(thickness - edgeSoftness, thickness, modDist);
    float outer = 1.0 - smoothstep(thickness, thickness + edgeSoftness, modDist);
    float ringMask = inner * outer;

    float twinkle = snoise(position.xz * twinkle_period);
    twinkle = smoothstep(0.4, 1.0, twinkle); // only keep high values

     // fade factor based on progress (rings fade out near edge)
    float fade = 1.0 - smoothstep(progress - 0.1, progress, dist);

    // normal rings with fade
    vec4 ringsColor = mix(bgColor, mainColor + twinkle * 1.25, ringMask * fade);

    // *** progress border ring ***
    float edgeInner = smoothstep(progress - 0.5, progress - 0.05, dist);
    float edgeOuter = 1.0 - smoothstep(progress - 0.05, progress, dist);
    float progressRing = edgeInner * edgeOuter;

    vec4 progressColor = mainColor * progressRing;

    // combine, but kill everything beyond progress
    vec4 finalColor = ringsColor + progressColor;
    if (dist > progress) {
        finalColor = vec4(0.0);
    }

    FragColor = finalColor;
}

