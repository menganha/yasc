#pragma once
#include "arena.hpp"
#include "array.hpp"

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

const IVec2 RES_SUPER_NINTENDO {256, 224};
const IVec2 RES_SUPER_NINTENDO_DOUBLE {512, 488}; // less used

const float TILE_SIZE = 16.f;
const float PIXEL_ADJ = 0.1f; // Needed for the correct texel interpolation
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

struct Entity
{
    Vec2  position;
    IVec2 texture_atlas_offset;
};

void loadLevel(Array<Vec4>& , Arena&);

