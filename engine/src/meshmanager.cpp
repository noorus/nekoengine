#include "stdafx.h"
#include "meshmanager.h"

namespace neko {

  // MeshManager

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
    for ( auto& buf : vbos )
      if ( !buf.uploaded )
        dirties.push_back( &buf );
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenBuffers( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      glBindBuffer( GL_ARRAY_BUFFER, ids[i] );
      glBufferData( GL_ARRAY_BUFFER, dirties[i]->storage_.size() * sizeof( T ), dirties[i]->storage_.data(), GL_STATIC_DRAW );
      dirties[i]->id = ids[i];
      dirties[i]->uploaded = true;
    }
  }

  void MeshManager::uploadVBOs()
  {
    vboUploadHelper( vbos3d_ );
    vboUploadHelper( vbos2d_ );
  }

  size_t MeshManager::pushVAO( VAO::VBOType type, size_t verticesVBO )
  {
    if ( type == VAO::VBO_3D && ( verticesVBO >= vbos3d_.size() || !vbos3d_[verticesVBO].uploaded ) )
      NEKO_EXCEPT( "VBO3D index out of bounds or VBO not uploaded while defining VAO" );
    if ( type == VAO::VBO_2D && ( verticesVBO >= vbos2d_.size() || !vbos2d_[verticesVBO].uploaded ) )
      NEKO_EXCEPT( "VBO2D index out of bounds or VBO not uploaded while defining VAO" );
    vaos_.emplace_back( type, verticesVBO );
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
      if ( dirties[i]->vboType_ == VAO::VBO_3D )
      {
        auto vbo = &( vbos3d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded )
          NEKO_EXCEPT( "VBO3D used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex3D ), nullptr ); // x, y, z
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex3D ), (void*)( 3 * sizeof( float ) ) ); // s, t
        dirties[i]->id = ids[i];
        dirties[i]->size_ = vbo->storage_.size();
        dirties[i]->uploaded_ = true;
      } else if ( dirties[i]->vboType_ == VAO::VBO_2D )
      {
        auto vbo = &( vbos2d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded )
          NEKO_EXCEPT( "VBO2D used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), nullptr ); // x, y
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), (void*)( 2 * sizeof( float ) ) ); // s, t
        dirties[i]->id = ids[i];
        dirties[i]->size_ = vbo->storage_.size();
        dirties[i]->uploaded_ = true;
      } else
        NEKO_EXCEPT( "Unknown VBO format" );

      glDisableVertexAttribArray( 1 );
      glDisableVertexAttribArray( 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
      glBindVertexArray( 0 );
    }
  }

  void VAO::draw( GLenum mode )
  {
    glBindVertexArray( id );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawArrays( mode, 0, (GLsizei)size_ );
    glBindVertexArray( 0 );
  }

  void MeshManager::teardown()
  {
    //
  }

}