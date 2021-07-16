#include "stdafx.h"

#include "MyGUI_NekoPlatform.h"

#include "renderer.h"
#include "meshmanager.h"

namespace MyGUI {

  using namespace gl;

  const size_t c_vertexBufferReallocStep = ( 5 * VertexQuad::VertexCount );

  NekoVertexBuffer::NekoVertexBuffer( neko::Renderer* renderer ):
  renderer_( renderer ), needVertexCount_( 0 ), vao_( 0 ), vertexCount_( 0 )
  {
  }

  NekoVertexBuffer::~NekoVertexBuffer()
  {
  }

  void NekoVertexBuffer::setVertexCount( size_t count )
  {
    needVertexCount_ = count;
  }

  size_t NekoVertexBuffer::getVertexCount() const
  {
    return needVertexCount_;
  }

  Vertex* NekoVertexBuffer::lock()
  {
    if ( needVertexCount_ > vertexCount_ || vertexCount_ == 0 )
      resize();
    buffer_->lock();
    return buffer_->buffer().data();
  }

  void NekoVertexBuffer::unlock()
  {
    assert( buffer_ );
    buffer_->unlock();
  }

  void NekoVertexBuffer::draw( int count )
  {
    glBindVertexArray( vao_ );
    auto& pipeline = renderer_->shaders().usePipeline( "mygui3d" );
    pipeline.setUniform( "tex", 0 );
    pipeline.setUniform( "yscale", 1.0f );
    glDrawArrays( GL_TRIANGLES, 0, count );
    glBindVertexArray( 0 );
  }

  void NekoVertexBuffer::create()
  {
    buffer_ = make_unique<neko::SmarterBuffer<MyGUI::Vertex>>( vertexCount_ );
    glCreateVertexArrays( 1, &vao_ );
    neko::AttribWriter attribs;
    attribs.add( GL_FLOAT, 3 ); // vec3 position
    attribs.add( GL_UNSIGNED_BYTE, 4, true ); // b4 color
    attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
    attribs.write( vao_ );
    glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
  }

  void NekoVertexBuffer::destroy()
  {
    glDeleteVertexArrays( 1, &vao_ );
    buffer_.reset();
  }

  void NekoVertexBuffer::resize()
  {
    vertexCount_ = ( needVertexCount_ + c_vertexBufferReallocStep );
    destroy();
    create();
  }

}