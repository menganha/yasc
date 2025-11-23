#include "game.hpp"

void loadLevel( Array<Vec4>& position_texture_coord, Arena& arena)
{

    int raw_level[14 * 16] = {
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
      2,  3,  2,  2,  2,  2,  2,  2,  2,  0,  2,  2,  6,  2,  2, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    };

    int count_solid = 0;
    for ( int idx = 0; idx < 14 * 16; idx++ )
    {
        if ( raw_level[idx] < 10 )
        {
            count_solid++;
        }
    }
    arraySetCapacity(position_texture_coord, count_solid, arena);

    int texture_width = 320; // TODO: Get them from reading the texture
    // int texture_height = 192;
    int res_width = 256;     // TODO: Get them from reading the global settings
    // int res_height = 224;
    int tile_width = 16;

    for ( int idx = 0; idx < 14 * 16; idx++ )
    {
        int tile_id = raw_level[idx];
        if ( tile_id < 10 )
        {
            float tile_offset_x = (tile_id* tile_width % texture_width);
            float tile_offset_y = (tile_id* tile_width / texture_width) * tile_width;
            float position_offset_x = (idx*tile_width % res_width);
            float position_offset_y = (idx*tile_width / res_width) * tile_width;
            arrayPushBack(position_texture_coord, Vec4 {position_offset_x, position_offset_y, tile_offset_x, tile_offset_y});
        }
    }
};
