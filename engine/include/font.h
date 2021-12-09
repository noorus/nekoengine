#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "textureatlas.h"
#include "materials.h"

namespace neko {

  using Codepoint = uint32_t;

  struct Glyph
  {
    Codepoint codepoint;
    size_t width;
    size_t height;
    Glyph()
        : codepoint( -1 ), width( 0 ), height( 0 )
    {
    }
  };

  namespace fonts {

    enum class RenderMode {
      Normal,
      OutlineEdge,
      OutlineOuter,
      OutlineInner,
      SDF
    };

    struct TexturedGlyph: public Glyph {
      vec2i offset;
      vec2 advance;
      vec2 coords[2];
      map<Codepoint, Real> kerning;
      RenderMode rendermode;
      Real outline_thickness;
      TexturedGlyph():
        rendermode( RenderMode::Normal ), outline_thickness( 0.0f ),
        offset( 0, 0 ), advance( 0.0f, 0.0f )
      {
        coords[0] = vec2( 0.0f );
        coords[1] = vec2( 0.0f );
      }
      inline Real getKerning( Codepoint codepoint )
      {
        if ( kerning.find( codepoint ) != kerning.end() )
          return kerning[codepoint];
        return 0.0f;
      }
    };

    using TexturedGlyphMap = map<Codepoint, TexturedGlyph>;

    class GraphicalFont
    {
      friend class FontManager;
      friend class ThreadedLoader;
    private:
      void postInit();
      void initEmptyGlyph();
      void generateKerning();
      void forceUCS2Charmap();

    protected:
      FT_Face face_;
      TexturedGlyphMap glyphs_;
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
      MaterialPtr material_;
    public:
      GraphicalFont( FontManagerPtr manager, size_t width, size_t height, size_t depth );
      ~GraphicalFont();
      void loadGlyph( Codepoint codepoint );
      TexturedGlyph* getGlyph( Codepoint codepoint );
      void loadFace( vector<uint8_t>& source, Real pointSize );
      bool use( Renderer* renderer, GLuint textureUnit );
      inline Real size() const { return size_; }
      inline Real ascender() const { return ascender_; }
      inline Real descender() const { return descender_; }
    };

    using GraphicalFontPtr = unique_ptr<GraphicalFont>;
    using GraphicalFontVector = vector<GraphicalFontPtr>;

  }

  class Font
  {
    friend class FontManager;
    friend class ThreadedLoader;
  public:
    struct Specs
    {
      vec2i atlasSize_;
      Real pointSize_;
    };
  protected:
    bool loaded_;
    fonts::GraphicalFontPtr impl_;
    FontManager* manager_;

  public:
    Font( FontManager* manager ): loaded_( false ), manager_( manager ) {}
    inline bool loaded() const { return loaded_; }
    inline Real size() const { return impl_->size(); }
    inline Real ascender() const { return impl_->ascender(); }
    inline Real descender() const { return impl_->descender(); }
    inline fonts::TexturedGlyph* glyph( Codepoint codepoint ) { return impl_->getGlyph( codepoint ); }
    inline bool use( Renderer* renderer, GLuint textureUnit ) { return impl_->use( renderer, textureUnit ); }
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
    struct FreeTypeVersion {
      FT_Int major;
      FT_Int minor;
      FT_Int patch;
      FT_TrueTypeEngineType trueTypeSupport;
    } ftVersion_;
    struct HarfbuzzVersion
    {
      unsigned int major;
      unsigned int minor;
      unsigned int patch;
    } hbVersion_;
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
    inline FT_Library library() { return freeType_; }
  };

}