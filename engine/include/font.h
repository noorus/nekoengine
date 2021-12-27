#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "textureatlas.h"
#include "materials.h"
#include "mesh_primitives.h"
#include <newtype.h>
#include "shaders.h"

namespace neko {

  struct FontStyleData {
    newtype::StyleID id_;
    MaterialPtr material_;
  };

  class Font {
  public:
    newtype::FontPtr font_;
    newtype::FontFacePtr face_;
    map<newtype::StyleID, FontStyleData> styles_;
    bool usable( newtype::StyleID style ) const;
  };

  using FontPtr = shared_ptr<Font>;
  using FontVector = vector<FontPtr>;

  struct NewtypeLibrary {
  public:
    HMODULE module_;
    newtype::Manager* manager_;
    newtype::fnNewtypeInitialize pfnNewtypeInitialize;
    newtype::fnNewtypeShutdown pfnNewtypeShutdown;
    void load( newtype::Host* host );
    void unload();
    inline newtype::Manager* mgr() { return manager_; }
  };

  struct TextData {
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
    inline BufferType& buffer() { return *buffer_.get(); }
    inline IndicesType& indices() { return *indices_.get(); }
    void draw( shaders::Shaders& shaders, gl::GLuint texture )
    {
      gl::glBindVertexArray( vao_ );
      auto& pipeline = shaders.usePipeline( "text2d" );
      mat4 mdl( 1.0f );
      mdl = glm::translate( mdl, vec3( 0.0f ) );
      mdl = glm::scale( mdl, vec3( 1.0f ) );
      pipeline.setUniform( "model", mdl );
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

  class FontManager: public enable_shared_from_this<FontManager>, public newtype::Host {
  protected:
    EnginePtr engine_;
    Renderer* renderer_;
    NewtypeLibrary nt_;
    FontPtr fnt_;
    newtype::TextPtr txt_;
    map<newtype::IDType, FontPtr> ntfonts_;
    //map<newtype::IDType, TextData> textdata_;
    unique_ptr<TextRenderBuffer> mesh_;
  public:
    void* newtypeMemoryAllocate( uint32_t size ) override;
    void* newtypeMemoryReallocate( void* address, uint32_t newSize ) override;
    void newtypeMemoryFree( void* address ) override;
    void newtypeFontTextureCreated( newtype::Font& font, newtype::StyleID style, newtype::Texture& texture ) override;
    void newtypeFontTextureDestroyed( newtype::Font& font, newtype::StyleID style, newtype::Texture& texture ) override;
  public:
    FontManager( EnginePtr engine );
    ~FontManager();
    void initialize();
    void shutdown();
    void prepareLogic( GameTime time );
    void prepareRender( Renderer* renderer );
    void draw( Renderer* renderer );
  };

}