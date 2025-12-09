#include "arena.hpp"
#include "control.hpp"
#include "file_io.hpp"
#include "font.hpp"
#include "game.hpp"
#include "log.hpp"
#include "shaders.hpp"

#include <SDL2/SDL.h>
#include <cmath>
#include <glad/gl.h>
#include <stb/stb_image.h>

constexpr Uint32 FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_OPENGL;
const char*      TITLE = "Unnamed title";
const IVec2      RESOLUTION = RES_SUPER_NINTENDO;
const int        RES_SCALING = 4;
const IVec2      WINDOW_SIZE {RESOLUTION.x * RES_SCALING, RESOLUTION.y* RES_SCALING};

int main([[maybe_unused]] int argc, char* argv[])
{

    // set_level(Logger::LOG_INFO);
    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0 )
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
    LDEBUG("Loaded GL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    Arena  arena {MEGABYTES(1)};
    Shader shader;
    Shader shader_effect;
    {
        arena.set_mark();
        const char* shader_str_vert = fileRead("src/sprite.vert", arena);
        const char* shader_str_frag = fileRead("src/sprite.frag", arena);
        const char* shader_str_frag_effect = fileRead("src/sprite_effect.frag", arena);
        shaderInit(shader, shader_str_vert, shader_str_frag);
        shaderInit(shader_effect, shader_str_vert, shader_str_frag_effect);
        arena.reset_to_mark();
    }

    FontData font_data {};
    GLuint   TEXTURE_FONT_ID;
    {
        font_data.file_path = "assets/PressStart2P.ttf";
        font_data.size = 8;
        font_data.atlas_width = 512;
        font_data.atlas_height = 512;
        unsigned char* data = FontLoad(font_data, arena);

        glGenTextures(1, &TEXTURE_FONT_ID);
        glBindTexture(GL_TEXTURE_2D, TEXTURE_FONT_ID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_data.atlas_width, font_data.atlas_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GLuint TEXTURE_ID;
    int    width, height;
    {
        int            nr_channels;
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
    }

    // Sets the projection matrix to both shaders
    {
        float proje_mat[4][4] {};
        proje_mat[0][0] = 2.f / RESOLUTION.x;
        proje_mat[1][1] = -2.f / RESOLUTION.y;
        proje_mat[2][2] = -1.f;
        proje_mat[3][3] = 1.f;

        proje_mat[0][3] = -1.f;
        proje_mat[1][3] = 1.f;
        proje_mat[2][3] = 0.f;

        glUseProgram(shader.program_id);
        GLint mat_loc_proj = glGetUniformLocation(shader.program_id, "projection");
        glUniformMatrix4fv(mat_loc_proj, 1, GL_TRUE, &proje_mat[0][0]);

        glUseProgram(shader_effect.program_id);
        mat_loc_proj = glGetUniformLocation(shader.program_id, "projection");
        glUniformMatrix4fv(mat_loc_proj, 1, GL_TRUE, &proje_mat[0][0]);
    }

    LoadLevelData(arena);
    GamepadState        keyboard {};
    Vec2                vel {};
    int                 movement_time_counter {};
    const int           movement_time {8}; // in frames
    bool                has_won {false};
    int                 current_level {};
    Registry            registry {};
    EntityID            ent_id_player = LoadLevel(registry, font_data, current_level);
    SDL_GameController* controller = ctrlFindController();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    SDL_Event event;
    bool      should_quit {false};
    float       time {};
    while ( not should_quit )
    {

        if ( time > 1e30f )
            time = 0.f;
        time+=0.017f;

        while ( SDL_PollEvent(&event) )
        {
            switch ( event.type )
            {
            case SDL_QUIT:
                should_quit = true;
                break;
            case SDL_KEYDOWN:
                switch ( event.key.keysym.sym )
                {
                case SDLK_F1: // Restart
                    CleanUp(registry);
                    LoadLevelData(arena);
                    ent_id_player = LoadLevel(registry, font_data, current_level);
                    break;
                case SDLK_F2: // Advance
                    CleanUp(registry);
                    current_level++;
                    ent_id_player = LoadLevel(registry, font_data, current_level);
                    break;
                case SDLK_F3: // Start from the beggining
                    CleanUp(registry);
                    current_level = 0;
                    ent_id_player = LoadLevel(registry, font_data, current_level);
                    break;
                }
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if ( !controller )
                {
                    LINFO("A new controller was connected with id %i", event.cdevice.which);
                    controller = SDL_GameControllerOpen(event.cdevice.which);
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                LINFO("Controller was disconnected with id %i", event.cdevice.which);
                if ( controller && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) )
                {
                    SDL_GameControllerClose(controller);
                    controller = ctrlFindController();
                }
                break;
            }
        }

        /// Update Game
        if ( controller )
        {
            ctrlUpdate(keyboard, controller);
        }
        else
        {
            ctrlUpdate(keyboard);
        }

        if ( ctrlIsPressed(keyboard, BUTTON_RIGHT) and not movement_time_counter )
        {
            vel.x = 1.f;
            vel.y = 0.f;
            movement_time_counter = movement_time;
        }
        else if ( ctrlIsPressed(keyboard, BUTTON_LEFT) and not movement_time_counter )
        {
            vel.x = -1.f;
            vel.y = 0.f;
            movement_time_counter = movement_time;
        }
        else if ( ctrlIsPressed(keyboard, BUTTON_UP) and not movement_time_counter )
        {
            vel.y = -1.f;
            vel.x = 0.f;
            movement_time_counter = movement_time;
        }
        else if ( ctrlIsPressed(keyboard, BUTTON_DOWN) and not movement_time_counter )
        {
            vel.y = 1.f;
            vel.x = 0.f;
            movement_time_counter = movement_time;
        }

        if ( movement_time_counter ) // is moving
        {
            Entity& ent_player = regGetEntity(registry, ent_id_player);
            regMoveEntity(registry, ent_id_player, vel.x * TILE_SIZE / movement_time, vel.y * TILE_SIZE / movement_time);
            movement_time_counter--;

            if ( movement_time_counter == 0 ) // Finish move
            {
                regRepositionEntity(
                  registry, ent_id_player,
                  std::round(ent_player.pos_prev.x + vel.x * TILE_SIZE),
                  std::round(ent_player.pos_prev.y + vel.y * TILE_SIZE));

                if ( EntityID ent_id_coll = HasCollided(registry, ent_id_player, ENT_FLAG_GOAL) )
                {
                    Entity& entity_collided = regGetEntity(registry, ent_id_coll);
                    if ( entity_collided.flags & ENT_FLAG_BOX )
                    {
                        regRepositionEntity(
                          registry, ent_id_coll,
                          std::round(entity_collided.pos_prev.x + vel.x * TILE_SIZE),
                          std::round(entity_collided.pos_prev.y + vel.y * TILE_SIZE));

                        if ( EntityID ent_id_coll_coll = HasCollided(registry, ent_id_coll) )
                        {
                            Entity& ent_coll_coll = regGetEntity(registry, ent_id_coll_coll);
                            if ( ent_coll_coll.flags & ENT_FLAG_GOAL ) // We collide a price with a movable block
                            {
                                entity_collided.flags |= ENT_FLAG_OCCUPIED;
                                if ( HasWon(registry) )
                                {
                                    has_won = true;
                                }
                            }
                        }
                        else
                        {
                            entity_collided.flags &= ~ENT_FLAG_OCCUPIED;
                        }
                    }
                }
            }
            else if ( HasCollided(registry, ent_id_player, ENT_FLAG_GOAL | ENT_FLAG_BOX) ) // General collision
            {
                regRepositionEntity(registry, ent_id_player, ent_player.pos_prev.x, ent_player.pos_prev.y);
                movement_time_counter = 0;
            }
            else if ( EntityID ent_id_coll = HasCollided(registry, ent_id_player, ENT_FLAG_GOAL) ) // Collision with boxes
            {
                Entity& entity_collided = regGetEntity(registry, ent_id_coll);
                // If we have collided with a movable box we move it with player, and if in this movement it collides
                // with a non-price block then we revert the two positions, namely, the player and the movable box
                regMoveEntity(registry, ent_id_coll, vel.x * TILE_SIZE / movement_time, vel.y * TILE_SIZE / movement_time);
                if ( EntityID ent_id_coll_coll = HasCollided(registry, ent_id_coll, ENT_FLAG_GOAL) )
                {
                    if ( ~regGetEntity(registry, ent_id_coll_coll).flags & ENT_FLAG_GOAL )
                    {
                        regRepositionEntity(registry, ent_id_coll, entity_collided.pos_prev.x, entity_collided.pos_prev.y);
                        regRepositionEntity(registry, ent_id_player, ent_player.pos_prev.x, ent_player.pos_prev.y);
                        movement_time_counter = 0;
                    }
                }
            }
        }

        ///  Draw section
        glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
        glClearDepth(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // First pass
        for ( auto& ent : registry.entities )
        {
            if ( ent.flags & ENT_FLAG_TEXT )
            {
                glUseProgram(shader.program_id);
                glBindTexture(GL_TEXTURE_2D, TEXTURE_FONT_ID);

                float texture_scaling[2][2] {};
                texture_scaling[0][0] = 1.f; /// font_data.atlas_width;
                texture_scaling[1][1] = 1.f; /// font_data.atlas_height;
                GLint mat_loc_proj = glGetUniformLocation(shader.program_id, "texture_scaling");
                glUniformMatrix2fv(mat_loc_proj, 1, GL_TRUE, &texture_scaling[0][0]);
                Draw(shader.program_id, ent.renderable);
            }
            else
            {

                float texture_scaling[2][2] {};
                texture_scaling[0][0] = 1.f / width;
                texture_scaling[1][1] = 1.f / height;

                GLuint program_id = shader.program_id;
                if ( ent.flags & ENT_FLAG_OCCUPIED )
                {
                    program_id = shader_effect.program_id;
                }
                glUseProgram(program_id);
                glBindTexture(GL_TEXTURE_2D, TEXTURE_ID);
                GLint time_var = glGetUniformLocation(program_id, "time");
                glUniform1f(time_var, time);

                GLint mat_loc_proj = glGetUniformLocation(program_id, "texture_scaling");
                glUniformMatrix2fv(mat_loc_proj, 1, GL_TRUE, &texture_scaling[0][0]);
                Draw(program_id, ent.renderable);
            }
        }

        SDL_GL_SwapWindow(window);

        if ( has_won )
        {
            LINFO("You have won!");
            CleanUp(registry);
            current_level++;
            has_won = false;
            ent_id_player = LoadLevel(registry, font_data, current_level);
        }
    }

    if ( controller )
    {
        SDL_GameControllerClose(controller);
    }
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(gl_context);
    SDL_Quit();
}
