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

  typedef struct {
    std::string data;
    std::string language;
    hb_script_t script;
    hb_direction_t direction;
    const char* c_data() { return data.c_str(); };
  } HBText;

  namespace HBFeature {
    const hb_tag_t KernTag = HB_TAG( 'k', 'e', 'r', 'n' ); // kerning operations
    const hb_tag_t LigaTag = HB_TAG( 'l', 'i', 'g', 'a' ); // standard ligature substitution
    const hb_tag_t CligTag = HB_TAG( 'c', 'l', 'i', 'g' ); // contextual ligature substitution

    static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };
  }

  class HBShaper {
  public:
    HBShaper( const string& fontFile, FontManagerPtr lib );
    ~HBShaper();

    void init();
    vector<Mesh*> drawText( RendererPtr renderer, HBText& text, float x, float y );
    void addFeature( hb_feature_t feature );

  private:
    FontManagerPtr lib;
    FontPtr face;

    hb_font_t* font;
    hb_buffer_t* buffer;
    vector<hb_feature_t> features;
  };

}