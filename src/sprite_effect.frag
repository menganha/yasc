#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform float time;
uniform sampler2D sprite;

float random (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}

void main()
{
    vec2 ipos = floor(vec2(tex_coord.x*256, tex_coord.y*224)); // Multiply by the dimensions of the window
    float rnd = clamp(random(ipos * time)+0.5, 0.6, 1.0);
    color = vec4(vec3(texture(sprite, tex_coord)), rnd);
}
