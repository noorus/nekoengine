#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  struct Glyph {
    uint8_t* buffer;
    size_t width;
    size_t height;
    float bearing_x;
    float bearing_y;
  };

  struct Font {
    FT_Face face_;
    Font() { memset( this, 0, sizeof( Font ) ); }
  };

  using FontPtr = shared_ptr<Font>;
  using FontVector = vector<FontPtr>;

  class FontManager {
  protected:
    FontVector fonts_;
    FT_MemoryRec_ ftMemAllocator_;
    FT_Library freeType_;
    platform::RWLock faceLock_;
    void forceUCS2Charmap( FT_Face face );
  public:
    FontManager();
    ~FontManager();
    FontPtr loadFace( uint8_t* buffer, size_t length, int32_t pointSize, uint32_t hDPI, uint32_t vDPI );
    Glyph rasterize( FontPtr font, uint32_t glyphIndex );
    void freeFace( FontPtr face );
  };

}