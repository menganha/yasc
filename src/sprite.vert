#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
layout (location = 1) in vec4 vertex_offset;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 projection;
uniform mat2 texture_scaling; // Scales the texture coordinates back to the [0, 1]x[0, 1] range

void main()
{
    tex_coord = texture_scaling * (vertex.zw + vertex_offset.zw);
    gl_Position = projection * model * vec4(vertex.xy + vertex_offset.xy, 0.0, 1.0);
}
