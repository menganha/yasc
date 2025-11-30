#include "game.hpp"

#include "file_io.hpp"
#include "log.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>

struct LevelData
{
    char level_name[128];
    int  tiles[LEVEL_DIM.y][LEVEL_DIM.x];
};

static struct
{
    LevelData data[MAX_LEVELS];
    int       num_levels;
} g_levels;

void LoadLevelData(Arena& arena)
{
    arena.set_mark();
    char* file_str = fileRead("assets/levels", arena);
    char* line = std::strtok(file_str, "\n");

    bool parsing_level = false;
    bool parsing_data = false;
    char comment_char = '#';
    int  tile_counter = 0;

    LDEBUG("Parsing file assets/levels");
    while ( line )
    {
        char* ptr_start;
        char* ptr_end;
        ptr_start = strFindFirstNonEmpty(line);
        if ( not parsing_level )
        {
            if ( *ptr_start == '[' )
            {
                ptr_end = strFindCharOrCommentChar(ptr_start + 1, ']', comment_char);
                if ( *ptr_end == ']' )
                {
                    std::size_t count = ptr_end - ptr_start - 1;
                    std::strncpy(g_levels.data[g_levels.num_levels].level_name, ptr_start + 1, count);
                    g_levels.data[g_levels.num_levels].level_name[count] = '\0';
                    parsing_level = true;
                    LDEBUG("Parsing now level: %s", g_levels.data[g_levels.num_levels].level_name);
                }
            }
        }
        else
        {
            if ( not parsing_data )
            {
                ptr_end = strFindCharOrCommentChar(ptr_start + 1, '=', comment_char);
                if ( *ptr_end == '=' )
                {
                    *ptr_end = '\0'; // separate the key and the value
                    char* key;
                    char* value;
                    key = strStripWhitespaceRight(ptr_start, ptr_end);
                    value = strFindFirstNonEmpty(ptr_end + 1);

                    ptr_end = strFindCharOrCommentChar(ptr_end + 1, '\0', comment_char);
                    if ( *ptr_end == comment_char )
                    {
                        *ptr_end = '\0';
                    }
                    value = strStripWhitespaceRight(value, ptr_end);
                    LDEBUG("Parsing key-value property: %s : %s ", key, value);

                    if ( strCompare(key, "level") )
                    {
                        parsing_data = true;
                    }
                }
            }
            else
            {
                LDEBUG("Processing line %s", line);
                while ( *line )
                {
                    int tile_idx_x = tile_counter % LEVEL_DIM.x;
                    int tile_idx_y = tile_counter / LEVEL_DIM.x;
                    LASSERT(tile_idx_x < LEVEL_DIM.x, "Number of horizontal tiles is larger than the dimension %s", line);
                    LASSERT(tile_idx_y < LEVEL_DIM.y, "Number of vertical tiles is larger than the dimensions: %s", line);
                    if ( *line == '-' )
                    {
                        g_levels.data[g_levels.num_levels].tiles[tile_idx_y][tile_idx_x] = TT_EMPTY;
                        tile_counter++;
                    }
                    else if ( *line == '1' )
                    {
                        g_levels.data[g_levels.num_levels].tiles[tile_idx_y][tile_idx_x] = TT_WALL;
                        tile_counter++;
                    }
                    else if ( *line == 'E' )
                    {
                        g_levels.data[g_levels.num_levels].tiles[tile_idx_y][tile_idx_x] = TT_PLAYER;
                        tile_counter++;
                    }
                    else if ( *line == 'B' )
                    {
                        g_levels.data[g_levels.num_levels].tiles[tile_idx_y][tile_idx_x] = TT_BOX;
                        tile_counter++;
                    }
                    else if ( *line == 'O' )
                    {
                        g_levels.data[g_levels.num_levels].tiles[tile_idx_y][tile_idx_x] = TT_PRICE;
                        tile_counter++;
                    }
                    line++;
                    if ( tile_counter == LEVEL_DIM.x * LEVEL_DIM.y )
                    {
                        parsing_data = false;
                        parsing_level = false;
                        tile_counter = 0;
                        g_levels.num_levels++;
                        break;
                    }
                }
            }
        }

        line = std::strtok(nullptr, "\n");
    }

    arena.reset_to_mark();
}

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

void CleanUp(Registry& registry)
{
    for ( int idx = 0; idx < registry.num_entities; idx++ )
    {
        Entity& entity = regGetEntity(registry, idx);
        if ( entity.renderable.VAO )
        {
            glDeleteVertexArrays(1, &entity.renderable.VAO);
        }
        if ( entity.renderable.VBO )
        {
            glDeleteBuffers(1, &entity.renderable.VBO);
        }
        if ( entity.renderable.VBO_instance )
        {
            glDeleteBuffers(1, &entity.renderable.VBO_instance);
        }
    }
    std::memset(&registry.entities, 0, MAX_ENTITIES * sizeof(Entity)); // Resets to zero the entire array
    registry.num_entities = 0;
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

static bool IsLevelValid(const int level[LEVEL_DIM.y][LEVEL_DIM.x])
{
    int  n_players = 0;
    int  n_boxes = 0;
    int  n_prices = 0;
    int* level_one_dim = (int*)level;
    for ( int idx = 0; idx < LEVEL_DIM.x * LEVEL_DIM.y; idx++ )
    {
        int tile_id = level_one_dim[idx];
        if ( tile_id == TT_PLAYER )
        {
            n_players++;
        }
        else if ( tile_id == TT_BOX )
        {
            n_boxes++;
        }
        else if ( tile_id == TT_PRICE )
        {
            n_prices++;
        }
    }
    if ( n_players != 1 )
    {
        LERROR("Number of players is not one: %i", n_players);
        return false;
    }
    if ( n_boxes != n_prices )
    {
        LERROR("Amount of boxes %i is different than the number of prices %i", n_boxes, n_prices);
        return false;
    }
    return true;
}

EntityID LoadLevel(Registry& registry, int level)
{

    LASSERT(g_levels.num_levels, "No level data found");
    if ( level >= g_levels.num_levels )
    {
        LERROR("Trying to asses non-existing level %i. There are only %i available levels", level + 1, g_levels.num_levels);
        level = g_levels.num_levels - 1;
    }

    LASSERT(IsLevelValid(g_levels.data[level].tiles), "Level is not valid");

    int      texture_width = 320; // TODO: Get them from reading the texture
    int      res_width = 256;     // TODO: Get them from reading the global settings
    EntityID player_ent_id {ENT_INVALID_ID};

    int* level_one_dim = (int*)(g_levels.data[level].tiles);
    int  num_static_tiles = 0;
    Vec4 offsets[LEVEL_DIM.x * LEVEL_DIM.y] {};
    for ( int idx = 0; idx < LEVEL_DIM.x * LEVEL_DIM.y; idx++ )
    {
        int      tile_id = level_one_dim[idx];
        float    position_x = (idx * (int)TILE_SIZE) % res_width;
        float    position_y = ((idx * (int)TILE_SIZE) / res_width) * TILE_SIZE;            // NOLINT: We explicitly want integer division
        float    tile_offset_x = (tile_id * (int)TILE_SIZE) % texture_width;
        float    tile_offset_y = ((tile_id * (int)TILE_SIZE) / texture_width) * TILE_SIZE; // NOLINT: We explicitly want integer division
        EntityID ent_id {ENT_INVALID_ID};
        float    layer = 0.0;

        if ( level_one_dim[idx] == TT_PLAYER )
        {
            ent_id = regNewEntity(registry);
            player_ent_id = ent_id;
            Entity& entity = regGetEntity(registry, ent_id);
            Vec4    quad_offset {0.f, 0.f, 128.f, 0.f};
            addToBuffer(entity.renderable, &quad_offset, 1);
            entity.renderable.num_instances = 1;
            layer = -0.2;
        }
        else if ( level_one_dim[idx] == TT_BOX )
        {
            ent_id = regNewEntity(registry);
            Entity& entity = regGetEntity(registry, ent_id);
            Vec4    quad_offset {0.f, 0.f, 112.f, 128.f};
            addToBuffer(entity.renderable, &quad_offset, 1);
            entity.renderable.num_instances = 1;
            entity.movable = true;
            layer = -0.2;
        }
        else if ( level_one_dim[idx] == TT_PRICE )
        {
            ent_id = regNewEntity(registry);
            Entity& entity = regGetEntity(registry, ent_id);
            Vec4    quad_offset {0.f, 0.f, 80.f, 128.f};
            addToBuffer(entity.renderable, &quad_offset, 1);
            entity.renderable.num_instances = 1;
            entity.price = true;
            layer = -0.1;
        }
        else if ( tile_id >= 0 ) // BACKGROUND TILES
        {
            // Generates a single entity at the end containing all background tiles
            ent_id = regNewEntity(registry);
            offsets[num_static_tiles].x = position_x;
            offsets[num_static_tiles].y = position_y;
            offsets[num_static_tiles].z = tile_offset_x;
            offsets[num_static_tiles].w = tile_offset_y;
            num_static_tiles++;

            Entity& entity = regGetEntity(registry, ent_id);
            entity.movable = false;
        }

        if ( ent_id ) // Sets the position of all entities
        {
            Entity& entity = regGetEntity(registry, ent_id);
            entity.pos.x = position_x;
            entity.pos.y = position_y;
            entity.pos_prev.x = entity.pos.x;
            entity.pos_prev.y = entity.pos.y;
            entity.size.x = TILE_SIZE;
            entity.size.y = TILE_SIZE;
            if ( entity.renderable.VAO ) // If it has a renderable the set initial position of the transform matrix
            {
                entity.renderable.model_mat[0][0] = 1.f;
                entity.renderable.model_mat[1][1] = 1.f;
                entity.renderable.model_mat[2][2] = 1.f;
                entity.renderable.model_mat[3][3] = 1.f;
                entity.renderable.model_mat[0][3] = entity.pos.x;
                entity.renderable.model_mat[1][3] = entity.pos.y;
                entity.renderable.model_mat[2][3] = layer;
            }
        }
    }

    // Sets a single entity for the background renderable
    // TODO: What happens if there's no background?
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

    LASSERT(player_ent_id, "No player position was specified in the map");
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
    return ENT_INVALID_ID;
}

EntityID HasCollidedWithBlock(Registry& registry, EntityID player_ent_id)
{

    Entity&  player_ent = regGetEntity(registry, player_ent_id);
    SDL_Rect collider_player = {(int)player_ent.pos.x, (int)player_ent.pos.y, player_ent.size.x, player_ent.size.y};
    for ( int idx = 0; idx < registry.num_entities; idx++ )
    {
        const Entity& ent = regGetEntity(registry, idx);
        if ( ent.size.x == 0 or idx == player_ent_id or not ent.movable or not ent.price )
        {
            continue;
        }
        SDL_Rect collider {(int)ent.pos.x, (int)ent.pos.y, ent.size.x, ent.size.y};
        if ( SDL_HasIntersection(&collider, &collider_player) )
        {
            return idx;
        }
    }
    return ENT_INVALID_ID;
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

