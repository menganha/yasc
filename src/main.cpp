#include "arena.hpp"
#include "control.hpp"
#include "file_io.hpp"
#include "game.hpp"
#include "levels.hpp"
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

    // Sets the projection matrix
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

    GamepadState        keyboard {};
    Vec2                vel {};
    int                 movement_time_counter {};
    const int           movement_time {8}; // in frames
    bool                has_won {false};
    int                 current_level {};
    Registry            registry {};
    EntityID            ent_id_player = LoadLevel(registry, current_level);
    SDL_GameController* controller = ctrlFindController();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

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
            case SDL_KEYDOWN:
                switch ( event.key.keysym.sym )
                {
                case SDLK_F1: // Restart
                    CleanUp(registry);
                    ent_id_player = LoadLevel(registry, current_level);
                    break;
                case SDLK_F2: // Advance
                    CleanUp(registry);
                    current_level++;
                    ent_id_player = LoadLevel(registry, current_level);
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

        if ( ctrlIsPressed(keyboard, BUTTON_SELECT) )
            LINFO("SELECT BUTTON PRESSED");

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

        if ( movement_time_counter )
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

                if ( EntityID ent_id_coll = HasCollided(registry, ent_id_player) )
                {
                    Entity& entity_collided = regGetEntity(registry, ent_id_coll);
                    if ( entity_collided.movable )
                    {
                        regRepositionEntity(
                          registry, ent_id_coll,
                          std::round(entity_collided.pos_prev.x + vel.x * TILE_SIZE),
                          std::round(entity_collided.pos_prev.y + vel.y * TILE_SIZE));

                        if ( EntityID ent_id_coll_coll = HasCollided(registry, ent_id_coll) )
                        {
                            Entity& ent_coll_coll = regGetEntity(registry, ent_id_coll_coll);
                            if ( ent_coll_coll.price ) // We collide a price with a movable block
                            {
                                entity_collided.occupied = true;
                                if ( HasWon(registry) )
                                {
                                    has_won = true;
                                }
                            }
                        }
                        else
                        {
                            entity_collided.occupied = false;
                        }
                    }
                }
            }
            else if ( EntityID ent_id_coll = HasCollided(registry, ent_id_player) ) // Collision
            {
                Entity& entity_collided = regGetEntity(registry, ent_id_coll);
                // If we have collided with a movable box we move it with player, and if in this movement it collides
                // with a non-price block then we revert the two positions, namely, the player and the movable box
                if ( entity_collided.movable )
                {
                    regMoveEntity(registry, ent_id_coll, vel.x * TILE_SIZE / movement_time, vel.y * TILE_SIZE / movement_time);
                    if ( EntityID ent_id_coll_coll = HasCollided(registry, ent_id_coll) )
                    {
                        if ( not regGetEntity(registry, ent_id_coll_coll).price )
                        {
                            regRepositionEntity(registry, ent_id_coll, entity_collided.pos_prev.x, entity_collided.pos_prev.y);
                            regRepositionEntity(registry, ent_id_player, ent_player.pos_prev.x, ent_player.pos_prev.y);
                            movement_time_counter = 0;
                        }
                    }
                }
                // If the collision is not with a movable nor a price, then just revert the position
                else if ( not entity_collided.price )
                {
                    regRepositionEntity(registry, ent_id_player, ent_player.pos_prev.x, ent_player.pos_prev.y);
                    movement_time_counter = 0;
                }
            }
        }

        ///  Draw

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClearDepth(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, TEXTURE_ID);
        glUseProgram(shader.program_id);

        for ( auto& ent : registry.entities )
        {
            Draw(shader.program_id, ent.renderable);
        }
        SDL_GL_SwapWindow(window);

        if ( has_won )
        {
            LINFO("You have won!");
            CleanUp(registry);
            current_level++;
            has_won = false;
            ent_id_player = LoadLevel(registry, current_level);
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
