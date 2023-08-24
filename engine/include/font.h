#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "materials.h"
#include "mesh_primitives.h"
#include <nlohmann/json.hpp>
#include "shaders.h"
#include "resources.h"
#include "textureatlas.h"
#include "pch.h"
#include "buffers.h"

namespace neko {

  constexpr int c_dpi = 72;
  constexpr float c_fmagic = 64.0f;
  constexpr int c_magic = 64;

  using IDType = uint64_t;
  using Codepoint = uint32_t;
  using GlyphIndex = uint32_t;
  using StyleID = uint64_t;
  using FaceID = signed long;

  class Buffer {
  private:
    uint8_t* buffer_ = nullptr;
    uint32_t length_ = 0;
  public:
    Buffer( uint32_t length ): length_( length )
    {
      buffer_ = static_cast<uint8_t*>( Locator::memory().alloc( Memory::Sector::Fonts, length_ ) );
      assert( buffer_ );
    }
    Buffer( const span<uint8_t> source )
    {
      length_ = static_cast<uint32_t>( source.size() );
      buffer_ = static_cast<uint8_t*>( Locator::memory().alloc( Memory::Sector::Fonts, length_ ) );
      assert( buffer_ );
      memcpy( buffer_, source.data(), length_ );
    }
    ~Buffer()
    {
      if ( buffer_ )
        Locator::memory().free( Memory::Sector::Fonts, buffer_ );
    }
    inline const uint32_t length() const { return length_; }
    inline uint8_t* data() { return buffer_; }
  };

  class HBBuffer {
  protected:
    hb_language_t language_;
    hb_script_t script_;
    hb_direction_t direction_;
    hb_buffer_t* hbbuf_ = nullptr;
    hb_glyph_info_t* glyphinfo_ = nullptr;
    hb_glyph_position_t* glyphpos_ = nullptr;
    unsigned int glyphcount_ = 0;
  public:
    HBBuffer( const string& language )
    {
      hbbuf_ = hb_buffer_create();
      hb_buffer_set_cluster_level( hbbuf_, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS );
      language_ = hb_language_from_string( language.c_str(), static_cast<int>( language.size() ) );
      script_ = HB_SCRIPT_LATIN;
      direction_ = HB_DIRECTION_LTR;
    }
    ~HBBuffer()
    {
      if ( hbbuf_ )
        hb_buffer_destroy( hbbuf_ );
    }
    inline unsigned int count() const { return glyphcount_; }
    inline span<hb_glyph_info_t> glyphInfo()
    {
      assert( glyphinfo_ );
      span<hb_glyph_info_t> spn( glyphinfo_, glyphcount_ );
      return spn;
    }
    inline span<hb_glyph_position_t> glyphPosition()
    {
      assert( glyphpos_ );
      span<hb_glyph_position_t> spn( glyphpos_, glyphcount_ );
      return spn;
    }
    void setFrom( hb_font_t* font, const vector<hb_feature_t>& feats, const unicodeString& str )
    {
      hb_buffer_reset( hbbuf_ );

      hb_buffer_set_direction( hbbuf_, direction_ );
      hb_buffer_set_script( hbbuf_, script_ );
      hb_buffer_set_language( hbbuf_, language_ );

      uint32_t flags = hb_buffer_get_flags( hbbuf_ );
      flags |= HB_BUFFER_FLAG_BOT;
      flags |= HB_BUFFER_FLAG_EOT;
      hb_buffer_set_flags( hbbuf_, static_cast<hb_buffer_flags_t>( flags ) );

      hb_buffer_add_utf16( hbbuf_, reinterpret_cast<const uint16_t*>( str.getBuffer() ), str.length(), 0, str.length() );

      hb_shape( font, hbbuf_, feats.empty() ? nullptr : feats.data(), static_cast<int>( feats.size() ) );

      glyphinfo_ = hb_buffer_get_glyph_infos( hbbuf_, &glyphcount_ );
      glyphpos_ = hb_buffer_get_glyph_positions( hbbuf_, &glyphcount_ );
    }
  };

  #pragma pack( push, 1 )
  union FontStyleIndex
  {
    struct FontStyleData
    {
      uint16_t face;
      uint32_t size;
      uint8_t outlineType;
      uint8_t outlineSize;
    } components;
    StyleID value;
  };
#pragma pack( pop )

  struct Glyph
  {
    GlyphIndex index = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    vec2i bearing;
    vec2 coords[2];
  };

  using GlyphMap = map<GlyphIndex, Glyph>;

  enum FontRendering
  {
    FontRender_Normal = 0,
    FontRender_Outline_Expand
  };

#define NEKO_FREETYPE_EXCEPT( description, retval ) \
 {                                                     \
  throw std::exception( description );                 \
 }

  constexpr uint32_t makeStoredFaceSize( Real size )
  {
    return static_cast<uint32_t>( size * 1000.0f );
  }

  inline StyleID makeStyleID( FaceID face, uint32_t size, FontRendering rendering, Real thickness )
  {
    FontStyleIndex d {};
    d.components.face = ( face & 0xFFFF ); // no instance/variations support
    d.components.outlineType = rendering;
    d.components.outlineSize = ( rendering == FontRender_Normal ? 0 : static_cast<uint8_t>( thickness * 10.0f ) );
    d.components.size = size;
    return d.value;
  }

  inline StyleID makeStyleID( FaceID face, Real size, FontRendering rendering, Real thickness )
  {
    return makeStyleID( face, makeStoredFaceSize( size ), rendering, thickness );
  }

  class FontManager;

  class FontStyle;
  using FontStylePtr = shared_ptr<FontStyle>;
  using FontStyleMap = map<StyleID, FontStylePtr>;

  class FontFace;
  using FontFacePtr = shared_ptr<FontFace>;
  using FontFaceMap = map<FaceID, FontFacePtr>;

  class Font;
  using FontPtr = shared_ptr<Font>;
  using FontVector = vector<FontPtr>;
  using FontMap = map<utf8String, FontPtr>;

  class FontStyle: public ShareableBase<FontStyle> {
    friend class ThreadedLoader;
    friend class FontManager;
    friend class FontFace;
    friend class Font;
    friend class Text;
  private:
    FontFacePtr face_;
    uint32_t storedFaceSize_;
    FT_Long storedFaceIndex_;
    FontRendering rendering_;
    Real outlineThickness_;
    TextureAtlasPtr atlas_;
    GlyphMap glyphs_;
    bool dirty_ = false;
    MaterialPtr material_;
    hb_font_t* hbfnt_ = nullptr;
    Real size_ = 0.0f;
    Real ascender_ = 0.0f;
    Real descender_ = 0.0f;
  protected:
    void initEmptyGlyph();
    void postLoad();
    void loadGlyph( FT_Library ft, FT_Face face, GlyphIndex index, bool hinting );
  public:
    FontStyle( FontFacePtr face, FT_Library ft, FT_Face ftface, Real size, vec2i atlasSize,
      FontRendering rendering, Real thickness, const unicodeString& prerenderGlyphs );
    inline StyleID id() const { return makeStyleID( storedFaceIndex_, storedFaceSize_, rendering_, outlineThickness_ ); }
    inline FontFacePtr face() const noexcept { return face_; }
    inline Real size() const noexcept { return size_; }
    inline Real ascender() const noexcept { return ascender_; }
    inline Real descender() const noexcept { return descender_; }
    Glyph* getGlyph( FT_Library ft, FT_Face face, GlyphIndex index );
    inline bool dirty() const { return dirty_; }
    inline void markClean() { dirty_ = false; }
    inline const MaterialPtr material() const { return material_; }
    const TextureAtlas& atlas() const;
    virtual ~FontStyle();
  };

  class FontFace: public ShareableBase<FontFace> {
    friend class ThreadedLoader;
    friend class FontManager;
    friend class FontStyle;
    friend class Font;
    friend class Text;
  private:
    FontPtr font_;
    FT_Library ft_ = nullptr;
    FT_Face face_ = nullptr;
    FontStyleMap styles_;
  protected:
    void forceUCS2Charmap();
  public:
    FontFace( FontPtr font, FT_Library ft, FT_Open_Args* args, FaceID faceIndex );
    FontStylePtr style( StyleID id );
    StyleID loadStyle( FontRendering rendering, Real size, Real thickness, const unicodeString& prerenderGlyphs );
    inline FontPtr font() { return font_; }
    inline const FontStyleMap& styles() const noexcept { return styles_; }
    virtual ~FontFace();
  };

  class Font: public LoadedResourceBase<Font>, public ShareableBase<Font> {
    friend class ThreadedLoader;
    friend class FontManager;
    friend class FontStyle;
    friend class FontFace;
    friend class Text;
  public:
    using Base = LoadedResourceBase<Font>;
    using Base::Ptr;
  private:
    FontManagerPtr manager_;
    FontFaceMap faces_;
    unique_ptr<Buffer> data_;
    IDType id_;
    FontFacePtr loadFace( span<uint8_t> source, FaceID faceIndex );
    void unload();
  public:
    Font( FontManagerPtr manager, IDType i, const utf8String& name );
    inline FontManagerPtr manager() { return manager_; }
    inline IDType id() const noexcept { return id_; }
    inline const FontFaceMap& faces() const noexcept { return faces_; }
    void update( Renderer* renderer );
    virtual ~Font();
  };

  using TextMeshPtr = unique_ptr<TextRenderBuffer>;

  enum AdvanceType
  {
    Advance_Invalid = 0,
    Advance_Glyph,
    Advance_InlineObject,
    Advance_Linebreak,
    Advance_Bidi,
    Advance_Layout
  };

  struct AdvanceInfo
  {
    AdvanceType type;
    int32_t x;
    int32_t y;
  };

  enum TextAlignment
  {
    TextAlign_Unspecified = 0,
    TextAlign_Start,
    TextAlign_End,
    TextAlign_Left,
    TextAlign_Right,
    TextAlign_Center
  };

  enum ScriptDirection
  {
    Direction_Unspecified = 0,
    Direction_LeftToRight,
    Direction_RightToLeft
  };

  class Text {
  public:
    struct Features
    {
      bool ligatures : 1;
      bool kerning   : 1;
    };
  private:
    FontManagerPtr manager_;
    //hb_language_t language_;
    //hb_script_t script_;
    // hb_direction_t direction_;
    // hb_buffer_t* hbbuf_ = nullptr;
    bool dirty_ = false;
    FontStylePtr style_;
    vector<hb_feature_t> features_;
    unique_ptr<HBBuffer> hbbuf_;
    unicodeString text_;
    IDType id_;
    TextMeshPtr mesh_;
    vector<VertexText> vertices_;
    Indices indices_;
    vec2 meshDimensions_ = { 0.0f, 0.0f };
    bool dead_ = false;
  public:
    Text() = delete;
    Text( FontManagerPtr manager, IDType id, FontStylePtr style, const Text::Features& features );
    void text( const unicodeString& text );
    const unicodeString& text();
    inline void content( unicodeString t ) { text( t ); }
    inline const unicodeString& content() noexcept { return text_; }
    void update( Renderer& renderer );
    bool dirty() const noexcept { return dirty_; }
    FontStylePtr style() { return style_; }
    void style( FontStylePtr newStyle );
    StyleID styleid() const noexcept { return style_->id(); }
    void regenerate();
    inline IDType id() const noexcept { return id_; }
    void draw( Renderer& renderer, const mat4& modelMatrix );
    inline void markDead() { dead_ = true; }
    inline const bool dead() const noexcept { return dead_; }
    inline const vec2& dimensions() const noexcept { return meshDimensions_; }
  };

  class FontManager: public LoadedResourceManagerBase<Font>, public ShareableBase<FontManager> {
  public:
    using Base = LoadedResourceManagerBase<Font>;
    using ResourcePtr = Base::ResourcePtr;
    using MapType = Base::MapType;
    friend class Font;
    friend class FontFace;
    friend class FontStyle;
    friend class Text;
  protected:
    Renderer* renderer_ = nullptr;
    map<IDType, TextPtr> texts_;
  private:
    FT_MemoryRec_ ftMemAllocator_;
    FT_Library freeType_ = nullptr;
    struct FreeTypeVersion
    {
      FT_Int major;
      FT_Int minor;
      FT_Int patch;
      FT_TrueTypeEngineType trueTypeSupport;
    } ftVersion_ = { 0 };
    struct HarfbuzzVersion
    {
      unsigned int major;
      unsigned int minor;
      unsigned int patch;
    } hbVersion_ = { 0 };
    FontMap fonts_;
    IDType fontIndex_ = 0;
    IDType textIndex_ = 0;
  protected:
    inline FT_Library ft() { return freeType_; }
  public:
    inline FontManagerPtr ptr() noexcept { return this->shared_from_this(); }
    FontManager( ThreadedLoaderPtr loader );
    ~FontManager();
    void initializeLogic();
    void initializeRender( Renderer* renderer );
    void shutdownLogic();
    void shutdownRender();
    void prepareLogic( GameTime time );
    void prepareRender();
    void loadJSONRaw( const json& arr );
    void loadJSON( const utf8String& input );
    void loadFile( const utf8String& filename );
    // Font overrides
    FontPtr createFont( const utf8String& name );
    FontFacePtr loadFace( FontPtr font, span<uint8_t> buffer, FaceID faceIndex );
    StyleID loadStyle(
      FontFacePtr face, Real size, FontRendering rendering, Real thickness, const unicodeString& prerenderGlyphs );
    void unloadFont( FontPtr font );
    // Text overrides
    TextPtr createText( FontStylePtr style );
    // Other overrides
    inline FontPtr font( const utf8String& name )
    {
      return ( fonts_.find( name ) == fonts_.end() ) ? FontPtr() : fonts_.at( name );
    }
    inline FontMap& fonts() { return fonts_; }
    void update();
    void draw();
  };

}