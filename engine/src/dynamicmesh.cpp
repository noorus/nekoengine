#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "locator.h"
#include "console.h"

namespace neko {

  using namespace gl;

  DynamicMesh::DynamicMesh( MeshManagerPtr manager, VBOType vertexType, GLenum drawMode ):
    manager_( move( manager ) ), vertexType_( vertexType ), drawMode_( drawMode ),
    vbo_( cInvalidIndexValue ), ebo_( cInvalidIndexValue ), vao_( cInvalidIndexValue )
  {
    vbo_ = manager_->createVBO( vertexType_ );
    ebo_ = manager_->createEBO();
    vao_ = manager_->pushVAO( vertexType_, vbo_, ebo_ );
  }

  void DynamicMesh::pushVertices( vector<Vertex2D> verts )
  {
    assert( vertexType_ == VBO_2D );
    auto vbo = &manager_->getVBO2D( vbo_ );
    vbo->storage_.insert( vbo->storage_.end(), verts.begin(), verts.end() );
    vbo->dirty_ = true;
  }

  void DynamicMesh::pushVertices( vector<Vertex3D> verts )
  {
    assert( vertexType_ == VBO_3D );
    auto vbo = &manager_->getVBO3D( vbo_ );
    vbo->storage_.insert( vbo->storage_.end(), verts.begin(), verts.end() );
    vbo->dirty_ = true;
  }

  void DynamicMesh::pushVertices( vector<VertexText3D> verts )
  {
    assert( vertexType_ == VBO_Text );
    auto vbo = &manager_->getVBOText( vbo_ );
    vbo->storage_.insert( vbo->storage_.end(), verts.begin(), verts.end() );
    vbo->dirty_ = true;
  }

  void DynamicMesh::pushIndices( vector<GLuint> indices )
  {
    auto ebo = &manager_->getEBO( ebo_ );
    ebo->storage_.insert( ebo->storage_.end(), indices.begin(), indices.end() );
    ebo->dirty_ = true;
  }

  void DynamicMesh::draw()
  {
    auto vao = &manager_->getVAO( vao_ );
    if ( vao->used_ && vao->uploaded_ )
      vao->draw( drawMode_ );
  }

  void DynamicMesh::dump( const utf8String& name )
  {
    auto ebo = &manager_->getEBO( ebo_ );
    Locator::console().printf( Console::srcGfx, "DynamicMesh %s:", name.c_str() );
    string verts;
    size_t vertCount = 0;
    if ( vertexType_ == VBO_2D )
    {
      auto vbo = &manager_->getVBO2D( vbo_ );
      vertCount = vbo->storage_.size();
      for ( auto& vert : vbo->storage_ )
      {
        char asd[128];
        sprintf_s( asd, 128, "[%.4f,%.4f,%.4f,%.4f] ", vert.x, vert.y, vert.s, vert.t );
        verts.append( asd );
      }
    }
    else if ( vertexType_ == VBO_Text )
    {
      auto vbo = &manager_->getVBOText( vbo_ );
      vertCount = vbo->storage_.size();
      for ( auto& vert : vbo->storage_ )
      {
        char asd[128];
        sprintf_s( asd, 128, "[%.4f,%.4f,%.4f,%.4f] ",
          vert.position.x, vert.position.y, vert.texcoord.s, vert.texcoord.t );
        verts.append( asd );
      }
    }
    Locator::console().printf( Console::srcGfx, "Vertices %d", vertCount );
    Locator::console().print( Console::srcGfx, verts );
    Locator::console().printf( Console::srcGfx, "Indices %d", ebo->storage_.size() );
    string indices;
    for ( auto& index : ebo->storage_ )
    {
      char asd[128];
      sprintf_s( asd, 128, "[%d] ", index );
      indices.append( asd );
    }
    Locator::console().print( Console::srcGfx, indices );
  }

  const size_t DynamicMesh::vertsCount() const
  {
    auto vbo = &manager_->getVBO2D( vbo_ );
    return vbo->storage_.size();
  }

  const size_t DynamicMesh::indicesCount() const
  {
    auto ebo = &manager_->getEBO( ebo_ );
    return ebo->storage_.size();
  }

  DynamicMesh::~DynamicMesh()
  {
    if ( vao_ != cInvalidIndexValue )
      manager_->freeVAO( vao_ );
    if ( ebo_ != cInvalidIndexValue )
      manager_->freeEBO( ebo_ );
    if ( vbo_ != cInvalidIndexValue )
      manager_->freeVBO( vertexType_, vbo_ );
  }

}