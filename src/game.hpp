#pragma once

#include <glad/gl.h>

struct Vec2
{
    float x, y;
};

struct IVec2
{
    int x, y;
};

struct Vec4
{
    float x, y, z, w;
};

struct Renderable
{
    GLuint VAO;
    GLuint VBO;
    GLuint VBO_instance;
    int    num_instances;
    float  model_mat[4][4];
};

const IVec2 RES_SUPER_NINTENDO {256, 224};
const IVec2 RES_SUPER_NINTENDO_DOUBLE {512, 488}; // less used
const int   MAX_ENTITIES = 128;

const float TILE_SIZE = 16.f;
const float PIXEL_ADJ = 0.1f; // Needed for the correct texel interpolation
constexpr IVec2 LEVEL_DIM {16, 14};
const int   MAX_LEVELS = 50;  // Amount of horizontal x vertical blocks
const Vec4  QUAD[8] = {
  // Defines a quad and the texture uv corrdinates for a particular tile size
  {      0.f,       0.f,       0.f + PIXEL_ADJ,       0.f + PIXEL_ADJ},
  {TILE_SIZE,      0.0f, TILE_SIZE - PIXEL_ADJ,       0.f + PIXEL_ADJ},
  {TILE_SIZE, TILE_SIZE, TILE_SIZE - PIXEL_ADJ, TILE_SIZE - PIXEL_ADJ},
  {     0.0f, TILE_SIZE,       0.f + PIXEL_ADJ, TILE_SIZE - PIXEL_ADJ},
};

const float MODEL_MAT_ID[4][4] {
  {1.f, 0.f, 0.f, 0.f},
  {0.f, 1.f, 0.f, 0.f},
  {0.f, 0.f, 1.f, 0.f},
  {0.f, 0.f, 0.f, 1.f}
};

enum TileType
{
    TT_PLAYER = -1,
    TT_BOX = -2,
    TT_PRICE = -3,
    TT_EMPTY = -20,
    TT_WALL1 = 0,
    TT_WALL2 = 1,
    TT_WALL3 = 2,
    TT_WALL4 = 3,
};

struct Entity
{
    Vec2       pos;
    Vec2       pos_prev; // previous frame position
    IVec2      size;
    Renderable renderable;
    bool       movable; // TODO: move these booleans to bit flags
    bool       price;
    bool       occupied;
};

struct Registry
{
    Entity entities[MAX_ENTITIES];
    int    num_entities;
};

using EntityID = int;

const EntityID ENT_INVALID {0};
EntityID       regNewEntity(Registry& registry);
void           regRepositionEntity(Registry& registry, EntityID id, float pos_x, float pos_y);
void           regMoveEntity(Registry& registry, EntityID id, float delta_x, float delta_y);
Entity&        regGetEntity(Registry& registry, EntityID id);

EntityID LoadLevel(Registry& registry, int level);
void     Draw(GLuint program, Renderable& renderable);
EntityID HasCollided(Registry& registry, EntityID player_ent_id);
bool     HasWon(Registry& registry);
void     CleanUp(Registry& registry);

