#include "arena.hpp"
#include "control.hpp"
#include "file_io.hpp"
#include "game.hpp"
#include "log.hpp"
#include "shaders.hpp"

#include <SDL2/SDL.h>
#include <glad/gl.h>
#include <stb/stb_image.h>

constexpr Uint32 FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_OPENGL;
const char*      TITLE = "Unnamed title";
const IVec2      RESOLUTION = RES_SUPER_NINTENDO;
const int        RES_SCALING = 4;
const IVec2      WINDOW_SIZE {RESOLUTION.x * RES_SCALING, RESOLUTION.y* RES_SCALING};

int main([[maybe_unused]] int argc, char* argv[])
{

    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        LERROR("SDL error when initializing: %s", SDL_GetError());
        return 1;
    }

    auto window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE.x, WINDOW_SIZE.y, FLAGS);
    if ( window == nullptr )
    {
        LERROR("SDL error when creating window: %s", SDL_GetError());
        return 1;
    }

    auto gl_context = SDL_GL_CreateContext(window);
    if ( window == nullptr )
    {
        LERROR("SDL error when creating OpenGL context: %s", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    LDEBUG("Loaded GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    const int num_instances = 6;
    // Set the quad for the sprite
    GLuint    InstanceVBO, VBO, VAO;
    {

        // Position in pixels and texels
        Vec4 pos_tex[num_instances] {
          {       0.f,   0.f,       128.f, 64.f},
          {      16.f,   0.f,         0.f,  0.f},
          {     100.f, 100.f,        16.f, 32.f},
          {     100.f,   0.f, 48.f + 32.f,  0.f},
          {    100.1f,  16.f, 48.f + 32.f,  0.f},
          {99.999999f,  32.f, 48.f + 32.f,  0.f}
        };

        // Generate buffers
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * 4, &QUAD[0], GL_STATIC_DRAW);

        glGenBuffers(1, &InstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * num_instances, &pos_tex[0], GL_STATIC_DRAW);

        // Generate Vertex array
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // attributes of the VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // attributes of the InstanceVBO
        glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glVertexAttribDivisor(1, 1);

        // unbind VBO and VAOS
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    GLuint VBO_player, VBO_player_offset, VAO_player;
    {

        // Generate buffers
        glGenBuffers(1, &VBO_player);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_player);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * 4, &QUAD[0], GL_STATIC_DRAW);

        Vec4 quad_offset {0.f, 0.f, 128.f, 0.f};
        glGenBuffers(1, &VBO_player_offset);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_player_offset);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4), &quad_offset, GL_STATIC_DRAW);

        // Generate Vertex array
        glGenVertexArrays(1, &VAO_player);
        glBindVertexArray(VAO_player);

        // attributes of the VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO_player);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // attributes of the InstanceVBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO_player_offset);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glVertexAttribDivisor(1, 1);

        // unbind VBO and VAOS
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Arena  arena {MEGABYTES(4)};
    Shader shader;
    {

        const char* vert_shader_str = fileRead("src/sprite.vert", arena);
        const char* frag_shader_str = fileRead("src/sprite.frag", arena);
        shaderInit(shader, vert_shader_str, frag_shader_str);

        glUseProgram(shader.program_id);
    }

    GLuint TEXTURE_ID;
    {
        int            width, height, nr_channels;
        const char*    file = "assets/tiles.png";
        unsigned char* data = stbi_load(file, &width, &height, &nr_channels, 0);

        glGenTextures(1, &TEXTURE_ID);
        glBindTexture(GL_TEXTURE_2D, TEXTURE_ID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);

        GLint sprite_loc = glGetUniformLocation(shader.program_id, "sprite");
        glUniform1i(sprite_loc, 0);

        // upload the uniform for scaling the texel vertices
        float texture_scaling[2][2] {};
        texture_scaling[0][0] = 1.f / width;
        texture_scaling[1][1] = 1.f / height;

        GLint mat_loc_proj = glGetUniformLocation(shader.program_id, "texture_scaling");
        glUniformMatrix2fv(mat_loc_proj, 1, GL_TRUE, &texture_scaling[0][0]);
    }

    {
        float proje_mat[4][4] {};
        proje_mat[0][0] = 2.f / RESOLUTION.x;
        proje_mat[1][1] = -2.f / RESOLUTION.y;
        proje_mat[2][2] = -1.f;
        proje_mat[3][3] = 1.f;

        proje_mat[0][3] = -1.f;
        proje_mat[1][3] = 1.f;
        proje_mat[2][3] = 0.f;

        GLint mat_loc_proj = glGetUniformLocation(shader.program_id, "projection");
        glUniformMatrix4fv(mat_loc_proj, 1, GL_TRUE, &proje_mat[0][0]);
    }

    float model_mat_player[4][4] {
      {1.f, 0.f, 0.f, 32.f},
      {0.f, 1.f, 0.f, 32.f},
      {0.f, 0.f, 1.f,  0.f},
      {0.f, 0.f, 0.f,  1.f}
    };

    Keyboard keyboard {};

    SDL_Event event;
    bool      should_quit {false};
    while ( not should_quit )
    {
        while ( SDL_PollEvent(&event) )
        {
            switch ( event.type )
            {
            case SDL_QUIT:
                should_quit = true;
                break;
            }
        }
        //////////////////////////
        /// Controls
        //////////////////////////
        ctrlUpdate(keyboard);
        if ( ctrlIsDown(keyboard, BUTTON_RIGHT) )
        {
            model_mat_player[0][3] += 1.f;
        }
        if ( ctrlIsDown(keyboard, BUTTON_LEFT) )
        {
            model_mat_player[0][3] -= 1.f;
        }
        if ( ctrlIsDown(keyboard, BUTTON_UP) )
        {
            model_mat_player[1][3] -= 1.f;
        }
        if ( ctrlIsDown(keyboard, BUTTON_DOWN) )
        {
            model_mat_player[1][3] += 1.f;
        }

        glClearColor(0.7f, 0.7f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ///////////////////////////
        /// Draw calls
        ///////////////////////////
        // glActiveTexture(GL_TEXTURE0); // Necessary?
        glBindTexture(GL_TEXTURE_2D, TEXTURE_ID);

        glUseProgram(shader.program_id);
        GLint mat_model = glGetUniformLocation(shader.program_id, "model");

        // Fixed geometry
        glUniformMatrix4fv(mat_model, 1, GL_TRUE, &MODEL_MAT_ID[0][0]);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_instances);

        // Non fixed geometry
        glUniformMatrix4fv(mat_model, 1, GL_TRUE, &model_mat_player[0][0]);
        glBindVertexArray(VAO_player);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 1);

        SDL_GL_SwapWindow(window);
    }

    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(gl_context);
    SDL_Quit();
}
