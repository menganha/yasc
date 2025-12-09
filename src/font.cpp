#include "font.hpp"

#include "file_io.hpp"
#include "log.hpp"

#include <glad/gl.h>
#include <stb/stb_truetype.h>

unsigned char* FontLoad(FontData& font_data, Arena& arena)
{
    // Read the font file
    unsigned char* file_buffer = (unsigned char*)fileRead(font_data.file_path, arena);
    LDEBUG("Loading font %s", font_data.file_path);

    int font_count = stbtt_GetNumberOfFonts(file_buffer);
    LDEBUG("Font file has %i fonts", font_count);

    stbtt_fontinfo font_info = {};
    if ( !stbtt_InitFont(&font_info, file_buffer, 0) )
    {
        LERROR("Font file %s initialization failed", font_data.file_path);
    }

    unsigned char* texture_data = arena.allocate<unsigned char>(font_data.atlas_width * font_data.atlas_height);

    stbtt_pack_context ctx;

    // Initializes the packing context ctx
    stbtt_PackBegin(
      &ctx,                   // stbtt_pack_context (this call will initialize it)
      texture_data,           // Font Atlas texture data
      font_data.atlas_width,  // Width of the font atlas texture
      font_data.atlas_height, // Height of the font atlas texture
      0,                      // Stride in bytes
      1,                      // Padding between the glyphs
      nullptr);

    // stbtt_PackSetOversampling(&ctx, 2, 2); //       for improved quality on small fonts

    // Creates character bitmaps. Data for how to render them is stored on the packed_chars
    stbtt_PackFontRange(
      &ctx,                  // stbtt_pack_context
      file_buffer,           // Font Atlas texture data
      0,                     // Font Index
      font_data.size,        // Size of font in pixels. (Use STBTT_POINT_SIZE(fontSize) to use points)
      FIRST_CHAR,            // Code point of the first charecter
      NUMBER_OF_CHARS,       // No. of charecters to be included in the font atlas
      font_data.packed_chars // stbtt_packedchar array, this struct will contain the data to render a glyph
    );

    // Cleans up the packing context and frees all used memory
    stbtt_PackEnd(&ctx);

    // Stores all the information from the packed chars into the aligned_quads!
    // TODO: Maybe packed chars should be fred after this? i.e., only aligned_quads are needed for our font renedering
    for ( int idx = 0; idx < NUMBER_OF_CHARS; idx++ )
    {
        float unusedX, unusedY;

        stbtt_GetPackedQuad(
          font_data.packed_chars,        // Array of stbtt_packedchar
          font_data.atlas_width,         // Width of the font atlas texture
          font_data.atlas_height,        // Height of the font atlas texture
          idx,                           // Index of the glyph
          &unusedX, &unusedY,            // current position of the glyph in screen pixel coordinates, (not required as we have a different corrdinate system)
          &font_data.aligned_quads[idx], // output stbtt_alligned_quad struct. (this struct mainly consists of the texture coordinates)
          0                              // Allign X and Y position to a integer (doesn't matter because we are not using 'unusedX' and 'unusedY')
        );
    }
    return texture_data;
}

