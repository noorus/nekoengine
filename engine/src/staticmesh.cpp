#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"

namespace neko {

  using namespace gl;

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts ):
    manager_( move( manager ) ), drawMode_( drawMode ),
    vbo_( cInvalidIndexValue ), ebo_( cInvalidIndexValue ), vao_( cInvalidIndexValue )
  {
    vbo_ = manager_->pushVBO( move( verts ) );
    vao_ = manager_->pushVAO( VBO_2D, vbo_ );
  }

  void StaticMesh::draw()
  {
    auto vao = &manager_->getVAO( vao_ );
    if ( vao->used_ && vao->uploaded_ )
      vao->draw( drawMode_ );
  }

  StaticMesh::~StaticMesh()
  {
    if ( vao_ != cInvalidIndexValue )
      manager_->freeVAO( vao_ );
    if ( ebo_ != cInvalidIndexValue )
      manager_->freeEBO( ebo_ );
    if ( vbo_ != cInvalidIndexValue )
      manager_->freeVBO( VBO_2D, vbo_ );
  }

}