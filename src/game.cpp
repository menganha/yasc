#include "game.hpp"

Level getLevel(int level_idx){
    Level level{};
    if (level_idx == 0){
        level.tiles[0][0] = 1;
        level.tiles[0][1] = 1;
        level.tiles[0][2] = 1;
    }     
    return level;
}
