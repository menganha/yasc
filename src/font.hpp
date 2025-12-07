#pragma once
#include "arena.hpp"

#include <stb/stb_truetype.h>

static const int NUMBER_OF_CHARS = 95;  // Number of ASCII characters (everything after the space)
static const int FIRST_CHAR = (int)' '; // Code point of the first printable ASCII character

struct FontData
{
    const char*        file_path;
    int                size;
    int                atlas_width;
    int                atlas_height;
    stbtt_packedchar   packed_chars[NUMBER_OF_CHARS];
    stbtt_aligned_quad aligned_quads[NUMBER_OF_CHARS];
};

unsigned char* FontLoad(FontData& font_data, Arena& arena);
