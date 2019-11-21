#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"

namespace neko {

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts ):
    manager_( move( manager ) ), drawMode_( drawMode ), size_( (GLsizei)verts.size() )
  {
    vbo_ = move( manager_->pushVBO( move( verts ) ) );
    vao_ = manager_->pushVAO( vbo_ );
  }

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices ):
    manager_( move( manager ) ), drawMode_( drawMode ), size_( (GLsizei)indices.size() )
  {
    vbo_ = move( manager_->pushVBO( move( verts ) ) );
    ebo_ = move( manager_->pushEBO( move( indices ) ) );
    vao_ = manager_->pushVAO( vbo_, ebo_ );
  }

  void StaticMesh::draw()
  {
    if ( vao_->uploaded_ )
      vao_->draw( drawMode_, size_ );
  }

  StaticMesh::~StaticMesh()
  {
    if ( vao_ )
      manager_->freeVAO( vao_ );
    if ( ebo_ )
      manager_->freeEBO( ebo_ );
    if ( vbo_ )
      manager_->freeVBO( vbo_ );
  }

}