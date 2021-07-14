#include "stdafx.h"

#include "MyGUI_NekoPlatform.h"

#include "renderer.h"
#include "meshmanager.h"

namespace MyGUI {

  using namespace gl;

  const size_t c_vertexBufferReallocStep = ( 5 * VertexQuad::VertexCount );

  NekoVertexBuffer::NekoVertexBuffer( neko::Renderer* renderer ):
  renderer_( renderer ), needVertexCount_( 0 )
  {
    mesh_ = move( renderer_->meshes().createDynamic( GLenum::GL_TRIANGLES, neko::VBO_MyGUI, false, true ) );
  }

  NekoVertexBuffer::~NekoVertexBuffer()
  {
    mesh_.reset();
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
    if ( needVertexCount_ > mesh_->vertsCount() || mesh_->vertsCount() == 0 )
      resize();

    return mesh_->vbo_->lock<MyGUI::Vertex>();
  }

  void NekoVertexBuffer::unlock()
  {
    assert( mesh_ );
    mesh_->vbo_->unlock();
  }

  void NekoVertexBuffer::draw( int count )
  {
    mesh_->begin();

    auto& pipeline = renderer_->shaders().usePipeline( "mygui3d" );
    pipeline.setUniform( "tex", 0 );
    pipeline.setUniform( "yscale", 1.0f );
    mesh_->draw( count );
  }

  void NekoVertexBuffer::resize()
  {
    mesh_->resize( needVertexCount_ + c_vertexBufferReallocStep );
  }

}