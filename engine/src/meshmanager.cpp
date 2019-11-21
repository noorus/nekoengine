#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  using namespace gl;

  // MeshManager: Generics

  VBOPtr MeshManager::createVBO( VBOType type )
  {
    VBOPtr ptr = make_shared<VBO>( type );
    console_->printf( Console::Source::srcGfx, "MeshManager created %s vbo 0x%I64X", type == VBO_2D ? "2D" : type == VBO_3D ? "3D" : "Text3D", ptr.get() );
    vbos_[type].push_back( ptr );
    return move( ptr );
  }

  void MeshManager::freeVBO( VBOPtr vbo )
  {
    auto target = &vbos_[vbo->type_];
    target->erase( std::remove( target->begin(), target->end(), move( vbo ) ), target->end() );
  }

  EBOPtr MeshManager::createEBO()
  {
    EBOPtr ptr = make_shared<EBO>();
    console_->printf( Console::Source::srcGfx, "MeshManager created ebo 0x%I64X", ptr.get() );
    ebos_.push_back( ptr );
    return move( ptr );
  }

  void MeshManager::freeEBO( EBOPtr ebo )
  {
    ebos_.erase( std::remove( ebos_.begin(), ebos_.end(), move( ebo ) ), ebos_.end() );
  }

  VAOPtr MeshManager::createVAO()
  {
    VAOPtr ptr = make_shared<VAO>();
    console_->printf( Console::Source::srcGfx, "MeshManager created vao 0x%I64X", ptr.get() );
    vaos_.push_back( ptr );
    return move( ptr );
  }

  void MeshManager::freeVAO( VAOPtr vao )
  {
    vaos_.erase( std::remove( vaos_.begin(), vaos_.end(), move( vao ) ), vaos_.end() );
  }

  // MeshManager: VBOs

  VBOPtr MeshManager::pushVBO( const vector<Vertex2D>& vertices )
  {
    auto vbo = createVBO( VBO_2D );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  VBOPtr MeshManager::pushVBO( const vector<Vertex3D>& vertices )
  {
    auto vbo = createVBO( VBO_3D );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  VBOPtr MeshManager::pushVBO( const vector<VertexText3D>& vertices )
  {
    auto vbo = createVBO( VBO_Text );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  void vboUploadHelper( vector<VBOPtr>& vbos, Console* console )
  {
    vector<VBO*> dirties;
    for ( auto& buf : vbos )
      if ( !buf->uploaded_ )
      {
        console->printf( Console::Source::srcGfx, "MeshManager uploading new vbo 0x%I64X", buf.get() );
        dirties.push_back( buf.get() );
      } else if ( buf->dirty_ )
      {
        console->printf( Console::Source::srcGfx, "MeshManager uploading dirty vbo 0x%I64X", buf.get() );
        dirties.push_back( buf.get() );
      }

    if ( !dirties.empty() )
    {
      vector<GLuint> ids;
      ids.resize( dirties.size() );
      glCreateBuffers( (GLsizei)dirties.size(), ids.data() );
      for ( size_t i = 0; i < dirties.size(); ++i )
      {
        if ( dirties[i]->uploaded_ )
          glDeleteBuffers( 1, &dirties[i]->id_ );
        dirties[i]->id_ = ids[i];
        const size_t elementSize = ( dirties[i]->type_ == VBO_2D ? sizeof( Vertex2D ) : dirties[i]->type_ == VBO_3D ? sizeof( Vertex3D ) : sizeof( VertexText3D ) );
        if ( !dirties[i]->storage_.empty() )
        {
          glNamedBufferStorage( dirties[i]->id_,
            dirties[i]->storage_.size() * elementSize,
            dirties[i]->storage_.data(),
            GL_DYNAMIC_STORAGE_BIT );
        }
        dirties[i]->uploaded_ = true;
        dirties[i]->dirty_ = false;
      }
    }
  }

  void MeshManager::uploadVBOs()
  {
    for ( size_t i = 0; i < Max_VBOType; ++i )
      vboUploadHelper( vbos_[i], console_.get() );
  }

  // MeshManager: VAOs

  VAOPtr MeshManager::pushVAO( VBOPtr verticesVBO )
  {
    auto ptr = createVAO();
    ptr->vbo_ = move( verticesVBO );
    return move( ptr );
  }

  VAOPtr MeshManager::pushVAO( VBOPtr verticesVBO, EBOPtr indicesEBO )
  {
    auto ptr = createVAO();
    ptr->vbo_ = move( verticesVBO );
    ptr->ebo_ = move( indicesEBO );
    return move( ptr );
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
    inline GLsizei stride() const { return stride_; }
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
    void write( GLuint handle )
    {
      GLuint ptr = 0;
      for ( GLuint i = 0; i < recs_.size(); ++i )
      {
        glEnableVertexArrayAttrib( handle, i );
        glVertexArrayAttribBinding( handle, i, 0 );
        glVertexArrayAttribFormat( handle, i, recs_[i].count_, recs_[i].type_, GL_FALSE, ptr );
        ptr += (GLuint)recs_[i].size_;
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

  StaticMeshPtr MeshManager::createStatic( GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices )
  {
    auto mesh = make_shared<StaticMesh>( shared_from_this(), drawMode, move( verts ), move( indices ) );
    statics_.push_back( mesh );
    return move( mesh );
  }

  void MeshManager::uploadVAOs()
  {
    vector<VAO*> dirties;
    for ( auto& vao : vaos_ )
      if ( !vao->uploaded_ )
      {
        console_->printf( Console::Source::srcGfx, "MeshManager uploading new vao 0x%I64X", vao.get() );
        dirties.push_back( vao.get() );
      }

    if ( dirties.empty() )
      return;

    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glCreateVertexArrays( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      assert( dirties[i]->vbo_ );
      auto vbo = dirties[i]->vbo_.get();
      if ( !vbo->uploaded_ )
        NEKO_EXCEPT( "VBO used for VAO has not been uploaded" );
      if ( dirties[i]->ebo_ )
      {
        glVertexArrayElementBuffer( ids[i], dirties[i]->ebo_->id_ );
      }
      if ( vbo->type_ == VBO_3D )
      {
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 3 ); // vec3 position
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.write( ids[i] );
        glVertexArrayVertexBuffer( ids[i], 0, vbo->id_, 0, attribs.stride() );
      }
      else if ( vbo->type_ == VBO_2D )
      {
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 2 ); // vec2 position
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.write( ids[i] );
        glVertexArrayVertexBuffer( ids[i], 0, vbo->id_, 0, attribs.stride() );
      }
      else if ( dirties[i]->vbo_->type_ == VBO_Text )
      {
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 3 ); // vec3 position
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.add( GL_FLOAT, 4 ); // vec4 color
        attribs.write( ids[i] );
        glVertexArrayVertexBuffer( ids[i], 0, vbo->id_, 0, attribs.stride() );
      }
      else
        NEKO_EXCEPT( "Unknown VBO format" );

      dirties[i]->id_ = ids[i];
      dirties[i]->uploaded_ = true;
    }
  }

  void VAO::draw( GLenum mode, GLsizei size )
  {
    glBindVertexArray( id_ );
    if ( ebo_ )
    {
      glDrawElements( mode, size, GL_UNSIGNED_INT, nullptr );
    }
    else
      glDrawArrays( mode, 0, size );
  }

  // MeshManager: EBOs

  EBOPtr MeshManager::pushEBO( const vector<GLuint>& indices )
  {
    auto ebo = createEBO();
    auto oldSize = ebo->storage_.size();
    size_t size = indices.size();
    ebo->storage_.resize( oldSize + size );
    memcpy( (GLuint*)ebo->storage_.data() + oldSize, indices.data(), size * sizeof( GLuint ) );
    return move( ebo );
  }

  void MeshManager::uploadEBOs()
  {
    vector<EBO*> dirties;
    for ( auto& ebo : ebos_ )
      if ( !ebo->uploaded_ )
      {
        console_->printf( Console::Source::srcGfx, "MeshManager uploading new ebo 0x%I64X", ebo.get() );
        dirties.push_back( ebo.get() );
      } else if ( ebo->dirty_ )
      {
        console_->printf( Console::Source::srcGfx, "MeshManager uploading dirty ebo 0x%I64X", ebo.get() );
        dirties.push_back( ebo.get() );
      }

    if ( !dirties.empty() )
    {
      vector<GLuint> ids;
      ids.resize( dirties.size() );
      glCreateBuffers( (GLsizei)dirties.size(), ids.data() );
      for ( size_t i = 0; i < dirties.size(); ++i )
      {
        if ( dirties[i]->uploaded_ )
          glDeleteBuffers( 1, &dirties[i]->id_ );
        dirties[i]->id_ = ids[i];
        if ( !dirties[i]->storage_.empty() )
        {
          glNamedBufferStorage( dirties[i]->id_,
            dirties[i]->storage_.size() * sizeof( GLuint ),
            dirties[i]->storage_.data(),
            GL_DYNAMIC_STORAGE_BIT );
        }
        dirties[i]->uploaded_ = true;
        dirties[i]->dirty_ = false;
      }
    }
  }

  void MeshManager::teardown()
  {
    dynamics_.clear();
    statics_.clear();
  }

}