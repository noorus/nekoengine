#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "locator.h"
#include "console.h"

namespace neko {

  DynamicMesh::DynamicMesh( MeshManagerPtr manager, VBOType vertexType, GLenum drawMode ):
    manager_( move( manager ) ), drawMode_( drawMode )
  {
    vbo_ = move( manager_->createVBO( vertexType ) );
    ebo_ = move( manager_->createEBO() );
    vao_ = move( manager_->pushVAO( vbo_, ebo_ ) );
  }

  void DynamicMesh::pushVertices( const vector<Vertex2D>& vertices )
  {
    assert( vbo_->type_ == VBO_2D );
    vbo_->pushVertices( vertices );
  }

  void DynamicMesh::pushVertices( const vector<Vertex3D>& vertices )
  {
    assert( vbo_->type_ == VBO_3D );
    vbo_->pushVertices( vertices );
  }

  void DynamicMesh::pushVertices( const vector<VertexText3D>& vertices )
  {
    assert( vbo_->type_ == VBO_Text );
    vbo_->pushVertices( vertices );
  }

  void DynamicMesh::pushIndices( const vector<GLuint>& indices )
  {
    ebo_->storage_.insert( ebo_->storage_.end(), indices.begin(), indices.end() );
    ebo_->dirty_ = true;
  }

  void DynamicMesh::begin()
  {
    if ( !vao_->uploaded_ )
      return;
    vao_->begin();
  }

  void DynamicMesh::draw()
  {
    if ( !vao_->uploaded_ )
      return;
    vao_->draw( drawMode_, (GLsizei)indicesCount() );
  }

  DynamicMesh::~DynamicMesh()
  {
    if ( vao_ )
      manager_->freeVAO( vao_ );
    if ( ebo_ )
      manager_->freeEBO( ebo_ );
    if ( vbo_ )
      manager_->freeVBO( vbo_ );
  }

}