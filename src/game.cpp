#include "game.hpp"

#include "log.hpp"

#include <SDL2/SDL.h>

static void addToBuffer(Renderable& renderable, Vec4* offsets, int num_instances)
{

    // Generate buffers
    glGenBuffers(1, &renderable.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, renderable.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * 4, &QUAD[0], GL_STATIC_DRAW);

    glGenBuffers(1, &renderable.VBO_instance);
    glBindBuffer(GL_ARRAY_BUFFER, renderable.VBO_instance);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * num_instances, &offsets[0], GL_STATIC_DRAW);

    // Generate Vertex array
    glGenVertexArrays(1, &renderable.VAO);
    glBindVertexArray(renderable.VAO);

    // attributes of the VBO
    glBindBuffer(GL_ARRAY_BUFFER, renderable.VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // attributes of the instance VBO
    glBindBuffer(GL_ARRAY_BUFFER, renderable.VBO_instance);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);

    // unbind VBO and VAOS
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

EntityID regNewEntity(Registry& registry)
{
    LASSERT(registry.num_entities < MAX_ENTITIES, "Max number of entities reached: %i", MAX_ENTITIES);
    if ( registry.num_entities == 0 )
    {
        registry.num_entities++; // always add one, the zero one which will be always the "invalid entity"
    }
    int id = registry.num_entities;
    registry.num_entities++;
    return EntityID {id};
}

Entity& regGetEntity(Registry& registry, EntityID id)
{
    LASSERT(id < registry.num_entities, "Invalid ID %i. The ID is larger than the number of entities", id);
    return registry.entities[id];
}

void regMoveEntity(Registry& registry, EntityID ent_id, float delta_x, float delta_y)
{
    Entity& entity = regGetEntity(registry, ent_id);
    entity.pos.x += delta_x;
    entity.pos.y += delta_y;
    entity.renderable.model_mat[0][3] = entity.pos.x;
    entity.renderable.model_mat[1][3] = entity.pos.y;
}

void regRepositionEntity(Registry& registry, EntityID ent_id, float pos_x, float pos_y)
{
    Entity& entity = regGetEntity(registry, ent_id);
    entity.pos.x = pos_x;
    entity.pos.y = pos_y;
    entity.pos_prev.x = entity.pos.x;
    entity.pos_prev.y = entity.pos.y;
    entity.renderable.model_mat[0][3] = entity.pos.x;
    entity.renderable.model_mat[1][3] = entity.pos.y;
}

EntityID LoadLevel(Registry& registry)
{
    int raw_level[14 * 16] = {
      3, 2, 2, 2, 2, 2, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 3, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 6, 2, 2, 2,
      2, -1, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -2, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -3, -20, -20, -20, -20, -3, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, 2,
      2, 2, 2, 2, 2, 2, 20, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

    int      texture_width = 320; // TODO: Get them from reading the texture
    int      res_width = 256;     // TODO: Get them from reading the global settings
    int      tile_width = 16;
    EntityID player_ent_id = -1;

    int  num_static_tiles = 0;
    Vec4 offsets[14 * 16] {};
    for ( int idx = 0; idx < 14 * 16; idx++ )
    {
        float position_x = (idx * tile_width) % res_width;
        float position_y = ((idx * tile_width) / res_width) * tile_width;
        int   tile_id = raw_level[idx];
        if ( tile_id >= 0 )
        {
            float tile_offset_x = (tile_id * tile_width) % texture_width;
            float tile_offset_y = ((tile_id * tile_width) / texture_width) * tile_width;

            // Generates one entity for each background tile with a collider
            EntityID ent_id = regNewEntity(registry);
            Entity&  entity = regGetEntity(registry, ent_id);
            entity.pos.x = position_x;
            entity.pos.y = position_y;
            entity.size.x = TILE_SIZE;
            entity.size.y = TILE_SIZE;

            offsets[num_static_tiles].x = position_x;
            offsets[num_static_tiles].y = position_y;
            offsets[num_static_tiles].z = tile_offset_x;
            offsets[num_static_tiles].w = tile_offset_y;
            num_static_tiles++;
        }
        else if ( raw_level[idx] == -1 )
        {
            LASSERT(player_ent_id < 0, "More than one player specified in the map");

            EntityID ent_id = regNewEntity(registry);
            player_ent_id = ent_id;
            Entity& entity = regGetEntity(registry, ent_id);

            Vec4 quad_offset {0.f, 0.f, 128.f, 0.f};
            addToBuffer(entity.renderable, &quad_offset, 1);
            entity.renderable.num_instances = 1;

            entity.pos.x = position_x;
            entity.pos.y = position_y;
            entity.pos_prev.x = entity.pos.x;
            entity.pos_prev.y = entity.pos.y;
            entity.size.x = TILE_SIZE;
            entity.size.y = TILE_SIZE;

            // set initial position
            entity.renderable.model_mat[0][0] = 1.f;
            entity.renderable.model_mat[1][1] = 1.f;
            entity.renderable.model_mat[2][2] = 1.f;
            entity.renderable.model_mat[3][3] = 1.f;
            entity.renderable.model_mat[0][3] = entity.pos.x;
            entity.renderable.model_mat[1][3] = entity.pos.y;
        }
        else if ( raw_level[idx] == -2 )
        {

            EntityID ent_id = regNewEntity(registry);
            Entity&  entity = regGetEntity(registry, ent_id);

            Vec4 quad_offset {0.f, 0.f, 112.f, 128.f};
            addToBuffer(entity.renderable, &quad_offset, 1);
            entity.renderable.num_instances = 1;
            entity.movable = true;

            entity.pos.x = position_x;
            entity.pos.y = position_y;
            entity.pos_prev.x = entity.pos.x;
            entity.pos_prev.y = entity.pos.y;
            entity.size.x = TILE_SIZE;
            entity.size.y = TILE_SIZE;

            // set initial position
            entity.renderable.model_mat[0][0] = 1.f;
            entity.renderable.model_mat[1][1] = 1.f;
            entity.renderable.model_mat[2][2] = 1.f;
            entity.renderable.model_mat[3][3] = 1.f;
            entity.renderable.model_mat[0][3] = entity.pos.x;
            entity.renderable.model_mat[1][3] = entity.pos.y;
        }
        else if ( raw_level[idx] == -3 )
        {

            EntityID ent_id = regNewEntity(registry);
            Entity&  entity = regGetEntity(registry, ent_id);

            Vec4 quad_offset {0.f, 0.f, 80.f, 128.f};
            addToBuffer(entity.renderable, &quad_offset, 1);
            entity.renderable.num_instances = 1;
            entity.price = true;

            entity.pos.x = position_x;
            entity.pos.y = position_y;
            entity.pos_prev.x = entity.pos.x;
            entity.pos_prev.y = entity.pos.y;
            entity.size.x = TILE_SIZE;
            entity.size.y = TILE_SIZE;

            // set initial position
            entity.renderable.model_mat[0][0] = 1.f;
            entity.renderable.model_mat[1][1] = 1.f;
            entity.renderable.model_mat[2][2] = 1.f;
            entity.renderable.model_mat[3][3] = 1.f;
            entity.renderable.model_mat[0][3] = entity.pos.x;
            entity.renderable.model_mat[1][3] = entity.pos.y;
        }
    }

    EntityID ent_id = regNewEntity(registry);
    Entity&  entity = regGetEntity(registry, ent_id);

    addToBuffer(entity.renderable, offsets, num_static_tiles);
    entity.renderable.num_instances = num_static_tiles;

    // set initial position
    entity.renderable.model_mat[0][0] = 1.f;
    entity.renderable.model_mat[1][1] = 1.f;
    entity.renderable.model_mat[2][2] = 1.f;
    entity.renderable.model_mat[3][3] = 1.f;
    entity.pos.x = 0.f;
    entity.pos.y = 0.f;

    LASSERT(player_ent_id >= 0, "No player position was specified in the map");
    return player_ent_id;
};

void Draw(GLuint program, Renderable& renderable)
{
    if ( renderable.VAO == 0 ) // There's no renderable component
    {
        return;
    }
    GLint mat_model = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(mat_model, 1, GL_TRUE, &renderable.model_mat[0][0]);
    glBindVertexArray(renderable.VAO);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, renderable.num_instances);
}

EntityID HasCollided(Registry& registry, EntityID player_ent_id)
{

    Entity&  player_ent = regGetEntity(registry, player_ent_id);
    SDL_Rect collider_player = {(int)player_ent.pos.x, (int)player_ent.pos.y, player_ent.size.x, player_ent.size.y};
    for ( int idx = 0; idx < registry.num_entities; idx++ )
    {
        const Entity& ent = regGetEntity(registry, idx);
        if ( ent.size.x == 0 or idx == player_ent_id )
        {
            continue;
        }
        SDL_Rect collider {(int)ent.pos.x, (int)ent.pos.y, ent.size.x, ent.size.y};
        if ( SDL_HasIntersection(&collider, &collider_player) )
        {
            return idx;
        }
    }
    return ENT_INVALID;
}

bool HasWon(Registry& registry)
{

    int num_occupied = 0;
    int num_prices = 0;
    for ( int idx = 0; idx < registry.num_entities; idx++ )
    {
        const Entity& entity = regGetEntity(registry, idx);
        if ( entity.price )
        {
            num_prices++;
        }
        else if ( entity.occupied )
        {
            num_occupied++;
        }
    }
    return num_prices == num_occupied;
}
