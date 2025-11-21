#include "shaders.hpp"

#include "log.hpp"

#include <cstdio>

void shaderInit(Shader& shader, const char* vertex_shader_str, const char* fragment_shader_str)
{
    GLuint shader_program = glCreateProgram();
    shader.program_id = shader_program;
    char info_log_buffer[512];

    int index = 0;
    while ( index < 2 )
    {
        // Create shader in open GL and compile
        GLuint      gl_shader;
        const char* shader_str;

        if ( index == 0 )
        {
            shader_str = vertex_shader_str;
            gl_shader = glCreateShader(GL_VERTEX_SHADER);
        }
        else
        {
            shader_str = fragment_shader_str;
            gl_shader = glCreateShader(GL_FRAGMENT_SHADER);
        }

        glShaderSource(gl_shader, 1, &shader_str, NULL);
        glCompileShader(gl_shader);

        // check for shader compile errors
        int success;
        glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &success);
        if ( not success )
        {
            glGetShaderInfoLog(gl_shader, 512, nullptr, info_log_buffer);
            LERROR("Shader compilation failed for shader type %i:  %s", index, info_log_buffer);
        }
        glAttachShader(shader_program, gl_shader);
        glDeleteShader(gl_shader); // flagd for deletion

        index++;
    }

    // link the shader program
    int success;
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success); // check for linking errors
    if ( not success )
    {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log_buffer);
        LERROR("Shader linking failed: %s", info_log_buffer);
    }
}

// void SetMat4(ShaderProgram* shader_program, const char* name, const glm::mat4& matrix)
// {
//     auto matrix_location = glGetUniformLocation(shader_program->program_id, name);
//     glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &matrix[0][0]);
// }
