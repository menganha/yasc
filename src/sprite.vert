#version 330 core

layout (location = 0) in vec4 vertex; 
// <vec2 position, vec2 texCoords>. We pass here a single Quad representing a generic sprite with a fixed width which
// will be used as the template for the different instances.

layout (location = 1) in vec4 vertex_offset;
// Contains offset in position and on the texture atlas in the same way the vertex vector does.
//
// The position offsets are only useful for instanced rendering where a single model matrix is used and it
// defines the reference point agains which this offsets are messured. For single entity drawing these should be set to zero 
// and the actual position would be managed by the "model" matrix.
//
// The texture offsets represent offsets from the origin (top-left) of the texture atlas. They work the same way for 
// both instanced an regular rendering

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 projection;
uniform mat2 texture_scaling; // Scales the texture coordinates back to the [0, 1]x[0, 1] range

void main()
{
    tex_coord = texture_scaling * (vertex.zw + vertex_offset.zw);
    gl_Position = projection * model * vec4(vertex.xy + vertex_offset.xy, 0.0, 1.0);
}

