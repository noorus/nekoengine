#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  namespace fonts {

    struct TextureAtlas {
      vector<vec3i> nodes_;
      size_t width_;
      size_t height_;
      size_t depth_;
      size_t used_;
      GLuint id_;
      vector<uint8_t> data_;
      TextureAtlas( const size_t width, const size_t height, const size_t depth );
      void setRegion( const size_t x, const size_t y, const size_t width, const size_t height, const uint8_t* data, const size_t stride );
      int fit( const size_t index, const size_t width, const size_t height );
      void merge();
      vec4i getRegion( const size_t width, const size_t height );
      void clear();
    };

    using TextureAtlasPtr = shared_ptr<TextureAtlas>;

    struct Kerning {
      uint32_t codepoint;
      Real kerning;
    };

    enum RenderMode {
      RENDER_NORMAL,
      RENDER_OUTLINE_EDGE,
      RENDER_OUTLINE_POSITIVE,
      RENDER_OUTLINE_NEGATIVE,
      RENDER_SIGNED_DISTANCE_FIELD
    };

    struct TextureGlyph {
      uint32_t codepoint;
      size_t width;
      size_t height;
      vec2i offset;
      vec2 advance;
      vec2 coords[2];
      //Real s0; // v2.x of topleft
      //Real t0; // v2.y of topleft
      //Real s1; // v2.x of bottomright
      //Real t1; // v2.x of bottomright
      vector<Kerning> kerning;
      RenderMode rendermode;
      Real outline_thickness;
      TextureGlyph(): codepoint( -1 ), width( 0 ), height( 0 ),
        rendermode( RENDER_NORMAL ), outline_thickness( 0.0f ),
        offset( 0, 0 ), advance( 0.0f, 0.0f ) {}
      inline Real getKerning( uint32_t codepoint )
      {
        for ( size_t i = 0; i < kerning.size(); ++i )
        {
          if ( kerning[i].codepoint == codepoint )
            return kerning[i].kerning;
        }
        return 0.0f;
      }
    };

    using TextureGlyphVector = vector<TextureGlyph>;

    class GraphicalFont {
    private:
      void postInit();
      void initEmptyGlyph();
      void generateKerning();
      void forceUCS2Charmap();
    public:
      FT_Face face_;
      TextureGlyphVector glyphs_;
      TextureAtlasPtr atlas_;
      FontManagerPtr manager_;
      Real size_;
      Real ascender_;
      Real descender_;
      RenderMode rendermode_;
      uint8_t lcd_weights[5];
      bool hinting_;
      bool filtering_;
      bool kerning_;
      Real outline_thickness_;
      Real linegap_;
      Real underline_position;
      Real underline_thickness;
      int padding_;
      unique_ptr<utils::DumbBuffer> data_;
    public:
      GraphicalFont( FontManagerPtr manager, size_t width, size_t height, size_t depth );
      ~GraphicalFont();
      void loadGlyph( uint32_t codepoint );
      TextureGlyph* getGlyph( uint32_t codepoint )
      {
        for ( auto& glyph : glyphs_ )
          if ( glyph.codepoint == codepoint )
            return &glyph;
        return nullptr;
      }
      void loadFace( vector<uint8_t>& source, Real pointSize );
    };

    using GraphicalFontPtr = unique_ptr<GraphicalFont>;
    using GraphicalFontVector = vector<GraphicalFontPtr>;

  }

  namespace ftgl {

    struct Markup {
      char* family;
      Real size;
      int bold;
      int italic;
      Real spacing;
      Real gamma;
      vec4 foreground_color;
      vec4 background_color;
      int outline;
      vec4 outline_color;
      int underline;
      vec4 underline_color;
      int overline;
      vec4 overline_color;
      int strikethrough;
      vec4 strikethrough_color;
      //TextureFontPtr font;
    };

    using MarkupPtr = shared_ptr<Markup>;

    struct LineInfo {
      size_t line_start;
      vec4 bounds;
    };

    enum Alignment {
      ALIGN_LEFT,
      ALIGN_CENTER,
      ALIGN_RIGHT
    };

constexpr auto LCD_FILTERING_ON = 3;
constexpr auto LCD_FILTERING_OFF = 1;

    struct TextBuffer {
      // VertexBufferPtr buffer;
      vec4 base_color;
      vec2 origin;
      Real last_pen_py;
      vec4 bounds;
      size_t line_start;
      Real line_left;
      vector<LineInfo> lines;
      Real line_ascender;
      Real line_descender;
      TextBuffer();
      ~TextBuffer();
      void addText( vec2 pen, Markup& markup, const char* text, size_t length );
      void addChar( vec2 pen, Markup& markup, const char* current, const char* previous );
      void bufferAlign( vec2 pen, Alignment alignment );
      vec4 getBounds( vec2 pen );
      void clear();
    };

  }

  struct Font {
    struct Specs {
      vec2i atlasSize_;
      Real pointSize_;
    };
    bool loaded_;
    fonts::GraphicalFontPtr impl_;
    FontManager* manager_;
    Font( FontManager* manager ): loaded_( false ), manager_( manager ) {}
  };

  using FontPtr = shared_ptr<Font>;
  using FontVector = vector<FontPtr>;

  class FontManager: public enable_shared_from_this<FontManager> {
  protected:
    FontVector fonts_;
    FT_MemoryRec_ ftMemAllocator_;
    FT_Library freeType_;
    platform::RWLock faceLock_;
    EnginePtr engine_;
    void uploadFonts();
  public:
    FontManager( EnginePtr engine );
    ~FontManager();
    void initialize();
    FontPtr createFont();
    void loadFont( FontPtr font, const Font::Specs& specs, vector<uint8_t>& buffer );
    void unloadFont( Font* font );
    void shutdown();
    void prepare( GameTime time );
    inline FontVector& fonts() { return fonts_; }
    inline FT_Library lib() { return freeType_; }
  };

}