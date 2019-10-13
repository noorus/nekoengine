#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"

namespace neko {

  using namespace gl;

  DynamicMesh::DynamicMesh( MeshManagerPtr manager ): manager_( move( manager ) ),
    vbo_( cInvalidIndexValue ), ebo_( cInvalidIndexValue ), vao_( cInvalidIndexValue )
  {
    vbo_ = manager_->createVBO( VBO_2D );
    ebo_ = manager_->createEBO();
    vao_ = manager_->pushVAO( VBO_2D, vbo_, ebo_ );
  }

  void DynamicMesh::pushVertices( vector<Vertex2D> verts )
  {
    auto vbo = &manager_->getVBO2D( vbo_ );
    vbo->storage_.insert( vbo->storage_.end(), verts.begin(), verts.end() );
    vbo->dirty_ = true;
  }

  void DynamicMesh::pushIndices( vector<GLuint> indices )
  {
    auto ebo = &manager_->getEBO( ebo_ );
    ebo->storage_.insert( ebo->storage_.end(), indices.begin(), indices.end() );
    ebo->dirty_ = true;
  }

  void DynamicMesh::draw( GLenum mode )
  {
    auto vao = &manager_->getVAO( vao_ );
    if ( vao->used_ && vao->uploaded_ )
      vao->draw( mode );
  }

  DynamicMesh::~DynamicMesh()
  {
    if ( vao_ != cInvalidIndexValue )
      manager_->freeVAO( vao_ );
    if ( ebo_ != cInvalidIndexValue )
      manager_->freeEBO( ebo_ );
    if ( vbo_ != cInvalidIndexValue )
      manager_->freeVBO( VBO_2D, vbo_ );
  }

  StaticMesh::StaticMesh( MeshManagerPtr manager, vector<Vertex2D> verts ):
    manager_( move( manager ) ),
    vbo_( cInvalidIndexValue ), ebo_( cInvalidIndexValue ), vao_( cInvalidIndexValue )
  {
    vbo_ = manager_->pushVBO( move( verts ) );
    vao_ = manager_->pushVAO( VBO_2D, vbo_ );
  }

  void StaticMesh::draw( GLenum mode )
  {
    auto vao = &manager_->getVAO( vao_ );
    if ( vao->used_ && vao->uploaded_ )
      vao->draw( mode );
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