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

  // MeshManager: Generals

  size_t MeshManager::createVBO( VBOType type )
  {
    if ( type == VBO_2D )
      return vbos2d_.pool_acquire();
    else if ( type == VBO_3D )
      return vbos3d_.pool_acquire();
    else if ( type == VBO_Text )
      return vbosText3d_.pool_acquire();
    else
      NEKO_EXCEPT( "Unknown VBO type" );
    return cInvalidIndexValue;
  }

  void MeshManager::freeVBO( VBOType type, size_t id )
  {
    if ( type == VBO_2D )
      vbos2d_.pool_release( id );
    else if ( type == VBO_3D )
      vbos3d_.pool_release( id );
    else if ( type == VBO_Text )
      vbosText3d_.pool_release( id );
    else
      NEKO_EXCEPT( "Unknown VBO type" );
  }

  size_t MeshManager::createEBO()
  {
    return ebos_.pool_acquire();
  }

  void MeshManager::freeEBO( size_t id )
  {
    ebos_.pool_release( id );
  }

  size_t MeshManager::createVAO()
  {
    return vaos_.pool_acquire();
  }

  void MeshManager::freeVAO( size_t id )
  {
    vaos_.pool_release( id );
  }

  // MeshManager: VBOs

  size_t MeshManager::pushVBO( vector<Vertex3D> vertices )
  {
    auto index = vbos3d_.pool_acquire();
    vbos3d_[index].storage_.swap( vertices );
    return index;
  }

  size_t MeshManager::pushVBO( vector<Vertex2D> vertices )
  {
    auto index = vbos2d_.pool_acquire();
    vbos2d_[index].storage_.swap( vertices );
    return index;
  }

  template <class T>
  void vboUploadHelper( VBOVector<T>& vbos )
  {
    vector<VBO<T>*> dirties;
    vector<VBO<T>*> subUpdates;
    for ( auto& buf : vbos )
      if ( !buf.used_ )
        continue;
      else if ( !buf.uploaded_ )
        dirties.push_back( &buf );
      else if ( buf.dirty_ )
        subUpdates.push_back( &buf );

    // Just to be sure.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // Generate & upload entirely new ones
    if ( !dirties.empty() )
    {
      vector<GLuint> ids;
      ids.resize( dirties.size() );
      glGenBuffers( (GLsizei)dirties.size(), ids.data() );
      for ( size_t i = 0; i < dirties.size(); ++i )
      {
        if ( !dirties[i]->storage_.empty() )
        {
          glBindBuffer( GL_ARRAY_BUFFER, ids[i] );
          glBufferData( GL_ARRAY_BUFFER,
            dirties[i]->storage_.size() * sizeof( T ),
            dirties[i]->storage_.data(),
            convertHint( dirties[i]->hint_ ) );
        }
        dirties[i]->id_ = ids[i];
        dirties[i]->uploaded_ = true;
        dirties[i]->dirty_ = false;
      }
    }

    // Update sub-data on existing ones
    for ( size_t i = 0; i < subUpdates.size(); ++i )
    {
      glNamedBufferSubData( subUpdates[i]->id_, 0,
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
    vboUploadHelper( vbosText3d_ );
  }

  // MeshManager: VAOs

  size_t MeshManager::pushVAO( VBOType type, size_t verticesVBO )
  {
    if ( type == VBO_3D && verticesVBO >= vbos3d_.size() )
      NEKO_EXCEPT( "VBO3D index out of bounds while defining VAO" );
    if ( type == VBO_2D && verticesVBO >= vbos2d_.size() )
      NEKO_EXCEPT( "VBO2D index out of bounds while defining VAO" );
    if ( type == VBO_Text && verticesVBO >= vbosText3d_.size() )
      NEKO_EXCEPT( "VBOTEXT index out of bounds while defining VAO" );
    auto index = vaos_.pool_acquire();
    vaos_[index].vboType_ = type;
    vaos_[index].vbo_ = verticesVBO;
    vaos_[index].useEBO_ = false;
    return index;
  }

  size_t MeshManager::pushVAO( VBOType type, size_t verticesVBO, size_t indicesEBO )
  {
    if ( type == VBO_3D && verticesVBO >= vbos3d_.size() )
      NEKO_EXCEPT( "VBO3D index out of bounds while defining VAO" );
    if ( type == VBO_2D && verticesVBO >= vbos2d_.size() )
      NEKO_EXCEPT( "VBO2D index out of bounds while defining VAO" );
    if ( type == VBO_Text && verticesVBO >= vbosText3d_.size() )
      NEKO_EXCEPT( "VBOTEXT index out of bounds while defining VAO" );
    if ( indicesEBO >= ebos_.size() )
      NEKO_EXCEPT( "EBO index out of bounds while defining VAO" );
    auto index = vaos_.pool_acquire();
    vaos_[index].vboType_ = type;
    vaos_[index].vbo_ = verticesVBO;
    vaos_[index].useEBO_ = true;
    vaos_[index].ebo_ = indicesEBO;
    return index;
  }

  class AttribWriter {
  private:
    struct Record {
      GLenum type_;
      GLsizei count_;
      size_t size_;
      Record( GLenum type, GLsizei count, size_t size ): type_( type ), count_( count ), size_( size ) {}
    };
    vector<Record> recs_;
    GLsizei stride_;
  public:
    AttribWriter(): stride_( 0 ) {}
    void add( GLenum type, GLsizei count )
    {
      GLsizei size = 0;
      if ( type == GL_FLOAT )
        size = ( count * sizeof( float ) );
      else
        NEKO_EXCEPT( "Unsupported vertex attribute type in writer" );

      recs_.emplace_back( type, count, size );
      stride_ += size;
    }
    void write()
    {
      size_t ptr = 0;
      for ( GLuint i = 0; i < recs_.size(); ++i )
      {
        glEnableVertexAttribArray( i );
        glVertexAttribPointer( i, recs_[i].count_, recs_[i].type_, GL_FALSE, stride_, (void*)ptr );
        ptr += recs_[i].size_;
      }
    }
  };

  DynamicMeshPtr MeshManager::createDynamic( GLenum drawMode, VBOType vertexType )
  {
    auto mesh = make_shared<DynamicMesh>( shared_from_this(), vertexType, drawMode );
    dynamics_.push_back( mesh );
    return move( mesh );
  }

  StaticMeshPtr MeshManager::createStatic( GLenum drawMode, vector<Vertex2D> verts )
  {
    auto mesh = make_shared<StaticMesh>( shared_from_this(), drawMode, move( verts ) );
    statics_.push_back( mesh );
    return move( mesh );
  }

  void MeshManager::uploadVAOs()
  {
    vector<VAO*> dirties;
    for ( auto& vao : vaos_ )
      if ( !vao.used_ )
        continue;
      else if ( !vao.uploaded_ )
        dirties.push_back( &vao );

    // Just to be sure.
    glBindVertexArray( 0 );

    // Early out if nothing to create; if VAOs can become "dirty" in the future, this needs to change.
    if ( dirties.empty() )
      return;

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
        if ( !vbo->uploaded_ )
          NEKO_EXCEPT( "VBO3D used for VAO has not been uploaded" );
        dirties[i]->size_ = vbo->storage_.size();
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id_ );
        if ( ebo )
        {
          glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo->id_ );
        }
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 3 ); // vec3 position
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.write();
        dirties[i]->size_ = vbo->storage_.size();
      }
      else if ( dirties[i]->vboType_ == VBO_2D )
      {
        auto vbo = &( vbos2d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded_ )
          NEKO_EXCEPT( "VBO2D used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id_ );
        if ( ebo )
        {
          glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo->id_ );
        }
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 2 ); // vec2 position
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.write();
        dirties[i]->size_ = vbo->storage_.size();
      }
      else if ( dirties[i]->vboType_ == VBO_Text )
      {
        auto vbo = &( vbosText3d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded_ )
          NEKO_EXCEPT( "VBOText used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id_ );
        if ( ebo )
        {
          glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo->id_ );
        }
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 3 ); // vec3 position
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.add( GL_FLOAT, 4 ); // vec4 color
        attribs.write();
        dirties[i]->size_ = vbo->storage_.size();
      } else
        NEKO_EXCEPT( "Unknown VBO format" );

      dirties[i]->id_ = ids[i];
      dirties[i]->uploaded_ = true;

      // Unbind the VAO !BEFORE! unbinding any attributes, VBOs or EBOs that are now bound to the VAO!
      glBindVertexArray( 0 );

      glDisableVertexAttribArray( MeshAttrib_Color );
      glDisableVertexAttribArray( MeshAttrib_Texcoord );
      glDisableVertexAttribArray( MeshAttrib_Position );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }
  }

  void VAO::draw( GLenum mode )
  {
    glBindVertexArray( id_ );
    if ( useEBO_ )
      glDrawElements( mode, (GLsizei)size_, GL_UNSIGNED_INT, nullptr );
    else
      glDrawArrays( mode, 0, (GLsizei)size_ );
    glBindVertexArray( 0 );
  }

  // MeshManager: EBOs

  size_t MeshManager::pushEBO( vector<GLuint> indexes )
  {
    auto index = ebos_.pool_acquire();
    ebos_[index].storage_.swap( indexes );
    return index;
  }

  void MeshManager::uploadEBOs()
  {
    vector<EBO*> dirties;
    vector<EBO*> subUpdates;
    for ( auto& ebo : ebos_ )
      if ( !ebo.used_ )
        continue;
      else if ( !ebo.uploaded_ )
        dirties.push_back( &ebo );
      else if ( ebo.dirty_ )
        subUpdates.push_back( &ebo );

    // Just to be sure.
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    if ( !dirties.empty() )
    {
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
        dirties[i]->dirty_ = false;
      }
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
    dynamics_.clear();
    statics_.clear();
  }

}