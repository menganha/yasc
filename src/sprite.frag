#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D sprite;

void main()
{
    color = texture(sprite, tex_coord);
}
