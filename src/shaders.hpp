//
// Shader utility functions
//

#pragma once


#include <glad/gl.h>

struct Shader
{
    GLuint program_id; // Make the paths part of the shader and perhaps initialize them passing the arena.
};

void shaderInit(Shader& shader_program, const char* vertex_shader_path, const char* fragmen_shader_path);


