#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "materials.h"
#include "mesh_primitives.h"
#include <newtype.h>
#include <nlohmann/json.hpp>
#include "shaders.h"
#include "resources.h"
#include "objectbase.h"

namespace neko {

  class FontManager;

  struct FontStyleData {
    newtype::StyleID id_ = 0;
    Material::Ptr material_;
  };

  using FontStyleMap = map<newtype::StyleID, FontStyleData>;

  class Font: public LoadedResourceBase<Font> {
  public:
    using Base = LoadedResourceBase<Font>;
    using Base::Ptr;
  private:
    newtype::Manager* mgr_;
    newtype::FontPtr font_;
    newtype::FontFacePtr face_;
    FontStyleMap styles_;
  public:
    // The constructor is public due to make_shared's peculiarities, but don't call it directly.
    Font() = delete;
    explicit Font( newtype::Manager* manager, const utf8String& name );
  public:
    newtype::IDType id() const;
    void loadFace( span<uint8_t> buffer, newtype::FaceID faceIndex, Real size );
    newtype::StyleID loadStyle( newtype::FontRendering rendering, Real thickness );
    bool usable( newtype::StyleID style ) const;
    inline newtype::FontFacePtr face() { return face_; }
    inline const FontStyleMap& styles() noexcept { return styles_; }
    inline const FontStyleData& style( newtype::StyleID sid )
    {
      return styles_[sid];
    }
    void update( Renderer* renderer );
    ~Font();
  };

  using FontPtr = Font::Ptr;
  using FontVector = vector<FontPtr>;

  struct NewtypeLibrary {
  private:
    HMODULE module_ = nullptr;
    newtype::Manager* manager_ = nullptr;
    newtype::fnNewtypeInitialize pfnNewtypeInitialize = nullptr;
    newtype::fnNewtypeShutdown pfnNewtypeShutdown = nullptr;
  public:
    void load( newtype::Host* host );
    void unload();
    inline newtype::Manager* mgr() { return manager_; }
    inline bool loaded() const noexcept { return ( module_ != nullptr ); }
  };

  class TextRenderBuffer {
  protected:
    using BufferType = MappedGLBuffer<newtype::Vertex>;
    using IndicesType = MappedGLBuffer<gl::GLuint>;
    unique_ptr<BufferType> buffer_;
    unique_ptr<IndicesType> indices_;
    GLuint vao_;
  public:
    TextRenderBuffer( GLuint maxVertices, GLuint maxIndices )
    {
      buffer_ = make_unique<BufferType>( maxVertices );
      indices_ = make_unique<IndicesType>( maxIndices );
      gl::glCreateVertexArrays( 1, &vao_ );
      gl::glVertexArrayElementBuffer( vao_, indices_->id() );
      neko::AttribWriter attribs;
      attribs.add( gl::GL_FLOAT, 3 ); // vec3 position
      attribs.add( gl::GL_FLOAT, 2 ); // vec2 texcoord
      attribs.add( gl::GL_FLOAT, 4 ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline BufferType& buffer() { return *buffer_; }
    inline IndicesType& indices() { return *indices_; }
    void draw( shaders::Shaders& shaders, const mat4& model, gl::GLuint texture )
    {
      gl::glBindVertexArray( vao_ );
      auto& pipeline = shaders.usePipeline( "text2d" );
      pipeline.setUniform( "model", model );
      pipeline.setUniform( "tex", 0 );
      gl::glBindTextureUnit( 0, texture );
      gl::glDrawElements( gl::GL_TRIANGLES, static_cast<gl::GLsizei>( indices_->size() ), gl::GL_UNSIGNED_INT, nullptr );
      gl::glBindVertexArray( 0 );
    }
    ~TextRenderBuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      indices_.reset();
      buffer_.reset();
    }
  };

  using TextMeshPtr = unique_ptr<TextRenderBuffer>;

  class Text: public TransformableObject {
  private:
    newtype::Manager* mgr_;
    FontPtr font_;
    newtype::TextPtr text_;
    TextMeshPtr mesh_;
    unicodeString content_;
  public:
    // The constructor is public due to make_shared's peculiarities, but don't call it directly.
    Text() = delete;
    explicit Text( newtype::Manager* manager, FontPtr font, newtype::StyleID style );
  public:
    newtype::IDType id() const;
    void content( unicodeString text );
    inline const unicodeString& content() noexcept { return content_; }
    void update( Renderer* renderer );
    void draw( Renderer* renderer );
  };

  class FontManager: public LoadedResourceManagerBase<Font>, public enable_shared_from_this<FontManager>, public newtype::Host {
  public:
    using Base = LoadedResourceManagerBase<Font>;
    using ResourcePtr = Base::ResourcePtr;
    using MapType = Base::MapType;
  protected:
    Renderer* renderer_ = nullptr;
    NewtypeLibrary nt_;
    map<newtype::IDType, FontPtr> fonts_;
    map<newtype::IDType, TextPtr> texts_;
    FontPtr createFont( const utf8String& name );
  protected:
    void* newtypeMemoryAllocate( uint32_t size ) override;
    void* newtypeMemoryReallocate( void* address, uint32_t newSize ) override;
    void newtypeMemoryFree( void* address ) override;
    void newtypeFontTextureCreated( newtype::Font& font, newtype::StyleID style, newtype::Texture& texture ) override;
    void newtypeFontTextureDestroyed( newtype::Font& font, newtype::StyleID style, newtype::Texture& texture ) override;
  public:
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
    TextPtr createText( const utf8String& fontName, newtype::StyleID style );
    void draw();
  };

}