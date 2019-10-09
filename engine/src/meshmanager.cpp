#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"

namespace neko {

  using namespace gl;

  // Should correspond to order in MeshDataModifyHint
  static const GLenum c_hintMapping[3] = { GL_STREAM_DRAW, GL_STATIC_DRAW, GL_DYNAMIC_DRAW };

  inline GLenum convertHint( MeshDataModifyHint hint )
  {
    return c_hintMapping[hint];
  }

  // MeshManager: VBOs

  size_t MeshManager::pushVBO( vector<Vertex3D> vertices )
  {
    VBO<Vertex3D> buf;
    buf.storage_.swap( vertices );
    vbos3d_.push_back( buf );
    return ( vbos3d_.size() - 1 );
  }

  size_t MeshManager::pushVBO( vector<Vertex2D> vertices )
  {
    VBO<Vertex2D> buf;
    buf.storage_.swap( vertices );
    vbos2d_.push_back( buf );
    return ( vbos2d_.size() - 1 );
  }

  template <class T>
  void vboUploadHelper( VBOVector<T>& vbos )
  {
    vector<VBO<T>*> dirties;
    vector<VBO<T>*> subUpdates;
    for ( auto& buf : vbos )
      if ( !buf.uploaded )
        dirties.push_back( &buf );
      else if ( buf.dirty_ )
        subUpdates.push_back( &buf );

    // Generate & upload entirely new ones
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenBuffers( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      glBindBuffer( GL_ARRAY_BUFFER, ids[i] );
      glBufferData( GL_ARRAY_BUFFER,
        dirties[i]->storage_.size() * sizeof( T ),
        dirties[i]->storage_.data(),
        convertHint( dirties[i]->hint_ ) );
      dirties[i]->id = ids[i];
      dirties[i]->uploaded = true;
    }

    // Update sub-data on existing ones
    for ( size_t i = 0; i < subUpdates.size(); ++i )
    {
      glNamedBufferSubData( subUpdates[i]->id, 0,
        subUpdates[i]->storage_.size() * sizeof( T ),
        subUpdates[i]->storage_.data() );
      subUpdates[i]->dirty_ = false;
    }

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
  }

  void MeshManager::uploadVBOs()
  {
    vboUploadHelper( vbos3d_ );
    vboUploadHelper( vbos2d_ );
  }

  // MeshManager: VAOs

  size_t MeshManager::pushVAO( VBOType type, size_t verticesVBO )
  {
    if ( type == VBO_3D && verticesVBO >= vbos3d_.size() )
      NEKO_EXCEPT( "VBO3D index out of bounds while defining VAO" );
    if ( type == VBO_2D && verticesVBO >= vbos2d_.size() )
      NEKO_EXCEPT( "VBO2D index out of bounds while defining VAO" );
    vaos_.emplace_back( type, verticesVBO );
    return ( vaos_.size() - 1 );
  }

  size_t MeshManager::pushVAO( VBOType type, size_t verticesVBO, size_t indicesEBO )
  {
    if ( type == VBO_3D && verticesVBO >= vbos3d_.size() )
      NEKO_EXCEPT( "VBO3D index out of bounds while defining VAO" );
    if ( type == VBO_2D && verticesVBO >= vbos2d_.size() )
      NEKO_EXCEPT( "VBO2D index out of bounds while defining VAO" );
    if ( indicesEBO >= ebos_.size() )
      NEKO_EXCEPT( "EBO index out of bounds while defining VAO" );
    vaos_.emplace_back( type, verticesVBO, indicesEBO );
    return ( vaos_.size() - 1 );
  }

  void MeshManager::uploadVAOs()
  {
    vector<VAO*> dirties;
    for ( auto& vao : vaos_ )
      if ( !vao.uploaded_ )
        dirties.push_back( &vao );
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenVertexArrays( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      EBO* ebo = nullptr;
      if ( dirties[i]->useEBO_ )
      {
        ebo = &( ebos_[dirties[i]->ebo_] );
        if ( !ebo->uploaded_ )
          NEKO_EXCEPT( "EBO used for VAO has not been uploaded" );
      }
      if ( dirties[i]->vboType_ == VBO_3D )
      {
        auto vbo = &( vbos3d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded )
          NEKO_EXCEPT( "VBO3D used for VAO has not been uploaded" );
        dirties[i]->size_ = vbo->storage_.size();
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
        if ( ebo )
        {
          glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo->id_ );
        }
        glEnableVertexAttribArray( MeshAttrib_Position );
        glVertexAttribPointer( MeshAttrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex3D ), nullptr ); // x, y, z
        glEnableVertexAttribArray( MeshAttrib_Texcoord );
        glVertexAttribPointer( MeshAttrib_Texcoord, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex3D ), (void*)( 3 * sizeof( float ) ) ); // s, t
        dirties[i]->size_ = vbo->storage_.size();
      }
      else if ( dirties[i]->vboType_ == VBO_2D )
      {
        auto vbo = &( vbos2d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded )
          NEKO_EXCEPT( "VBO2D used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
        if ( ebo )
        {
          glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo->id_ );
        }
        glEnableVertexAttribArray( MeshAttrib_Position );
        glVertexAttribPointer( MeshAttrib_Position, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), nullptr ); // x, y
        glEnableVertexAttribArray( MeshAttrib_Texcoord );
        glVertexAttribPointer( MeshAttrib_Texcoord, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), (void*)( 2 * sizeof( float ) ) ); // s, t
        dirties[i]->size_ = vbo->storage_.size();
      } else
        NEKO_EXCEPT( "Unknown VBO format" );

      dirties[i]->id = ids[i];
      dirties[i]->uploaded_ = true;

      // Unbind the VAO !BEFORE! unbinding any attributes, VBOs or EBOs that are now bound to the VAO!
      glBindVertexArray( 0 );

      glDisableVertexAttribArray( MeshAttrib_Texcoord );
      glDisableVertexAttribArray( MeshAttrib_Position );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }
  }

  void VAO::draw( GLenum mode )
  {
    glBindVertexArray( id );
    if ( useEBO_ )
      glDrawElements( mode, (GLsizei)size_, GL_UNSIGNED_INT, 0 );
    else
      glDrawArrays( mode, 0, (GLsizei)size_ );
    glBindVertexArray( 0 );
  }

  // MeshManager: EBOs

  size_t MeshManager::pushEBO( vector<GLuint> indexes )
  {
    EBO ebo;
    ebo.storage_.swap( indexes );
    ebos_.push_back( move( ebo ) );
    return ( ebos_.size() - 1 );
  }

  void MeshManager::uploadEBOs()
  {
    vector<EBO*> dirties;
    vector<EBO*> subUpdates;
    for ( auto& ebo : ebos_ )
      if ( !ebo.uploaded_ )
        dirties.push_back( &ebo );
      else if ( ebo.dirty_ )
        subUpdates.push_back( &ebo );

    // Generate & upload entirely new ones
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenBuffers( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ids[i] );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER,
        dirties[i]->storage_.size() * sizeof( GLuint ),
        dirties[i]->storage_.data(),
        convertHint( dirties[i]->hint_ ) );
      dirties[i]->id_ = ids[i];
      dirties[i]->uploaded_ = true;
    }

    // Update sub-data on existing ones
    for ( size_t i = 0; i < subUpdates.size(); ++i )
    {
      glNamedBufferSubData( subUpdates[i]->id_, 0,
        subUpdates[i]->storage_.size() * sizeof( GLuint ),
        subUpdates[i]->storage_.data() );
      subUpdates[i]->dirty_ = false;
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  }

  void MeshManager::teardown()
  {
    //
  }

}