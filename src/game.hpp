#pragma once

// const int MAX_TILES_HORIZ = 30;
// const int MAX_TILES_VERT = 20;
//
// struct Level
// {
//     int tiles[MAX_TILES_HORIZ][MAX_TILES_VERT];
// };
//
// Level getLevel(int level_idx);

// struct __xy { float x, y;};

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

struct Entity
{
    Vec2 position;
    IVec2 texture_atlas_offset;
};
