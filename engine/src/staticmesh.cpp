#include "pch.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "shaders.h"

namespace neko {

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts ):
  manager_( move( manager ) ), drawMode_( drawMode ), size_( (GLsizei)verts.size() )
  {
    vbo_ = move( manager_->pushVBO( verts ) );
    vao_ = manager_->pushVAO( vbo_ );
  }

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices ):
  manager_( move( manager ) ), drawMode_( drawMode ), size_( (GLsizei)indices.size() )
  {
    vbo_ = move( manager_->pushVBO( verts ) );
    ebo_ = move( manager_->pushEBO( indices ) );
    vao_ = manager_->pushVAO( vbo_, ebo_ );
  }

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex3D> verts ):
  manager_( move( manager ) ), drawMode_( drawMode ), size_( (GLsizei)verts.size() )
  {
    vbo_ = move( manager_->pushVBO( verts ) );
    vao_ = manager_->pushVAO( vbo_ );
  }

  StaticMesh::StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex3D> verts, vector<GLuint> indices ):
  manager_( move( manager ) ), drawMode_( drawMode ), size_( (GLsizei)indices.size() )
  {
    vbo_ = move( manager_->pushVBO( verts ) );
    ebo_ = move( manager_->pushEBO( indices ) );
    vao_ = manager_->pushVAO( vbo_, ebo_ );
  }

  void StaticMesh::begin()
  {
    if ( !vao_->uploaded_ )
      return;
    vao_->begin();
  }

  void StaticMesh::draw()
  {
    if ( !vao_->uploaded_ )
      return;
    vao_->draw( drawMode_ );
  }

  void StaticMesh::drawOnce( Pipeline& pipeline, vec3 position, vec3 scale, quaternion rotation )
  {
    if ( !vao_->uploaded_ )
      return;

    mat4 modelMatrix( 1.0f );
    modelMatrix = glm::translate( modelMatrix, position );
    modelMatrix = glm::scale( modelMatrix, scale );
    modelMatrix *= glm::toMat4( rotation );

    vao_->begin();
    pipeline.setUniform( "model", modelMatrix );
    vao_->draw( drawMode_ );
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