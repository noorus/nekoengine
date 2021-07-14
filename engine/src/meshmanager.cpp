#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "renderer.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  using namespace gl;

  // MeshManager: Generics

  VBOPtr MeshManager::createVBO( VBOType type, bool mappable )
  {
    VBOPtr ptr = make_shared<VBO>( type, mappable );
    vbos_[type].push_back( ptr );
    return move( ptr );
  }

  void MeshManager::freeVBO( VBOPtr vbo )
  {
    freeBuffers_.push_back( vbo->id_ );
    auto target = &vbos_[vbo->type_];
    target->erase( std::remove( target->begin(), target->end(), move( vbo ) ), target->end() );
  }

  EBOPtr MeshManager::createEBO()
  {
    EBOPtr ptr = make_shared<EBO>();
    ebos_.push_back( ptr );
    return move( ptr );
  }

  void MeshManager::freeEBO( EBOPtr ebo )
  {
    freeBuffers_.push_back( ebo->id_ );
    ebos_.erase( std::remove( ebos_.begin(), ebos_.end(), move( ebo ) ), ebos_.end() );
  }

  VAOPtr MeshManager::createVAO()
  {
    VAOPtr ptr = make_shared<VAO>();
    vaos_.push_back( ptr );
    return move( ptr );
  }

  void MeshManager::freeVAO( VAOPtr vao )
  {
    freeVaos_.push_back( vao->id_ );
    vaos_.erase( std::remove( vaos_.begin(), vaos_.end(), move( vao ) ), vaos_.end() );
  }

  void MeshManager::addJSMesh( js::Mesh* mesh )
  {
    auto& inmesh = mesh->mesh();
    assert(
      ( !inmesh.vbo_->uploaded_ ) &&
      ( !inmesh.ebo_->uploaded_ ) &&
      ( !inmesh.vao_ || !inmesh.vao_->uploaded_ ) );

    if ( meshes_.find( mesh->mesh().id_ ) != meshes_.end() )
    {
      NEKO_EXCEPT( "Mesh map key already exists" );
    }

    vbos_[inmesh.vbo_->type_].push_back( inmesh.vbo_ );
    ebos_.push_back( inmesh.ebo_ );
    inmesh.vao_ = move( pushVAO( inmesh.vbo_, inmesh.ebo_ ) );

    meshes_[mesh->mesh().id_] = mesh;
  }

  void MeshManager::removeJSMesh( js::Mesh* mesh )
  {
    auto& inmesh = mesh->mesh();
    if ( inmesh.vao_ )
      freeVAO( inmesh.vao_ );
    if ( inmesh.ebo_ )
      freeEBO( inmesh.ebo_ );
    if ( inmesh.vbo_ )
      freeVBO( inmesh.vbo_ );

    meshes_.erase( mesh->mesh().id_ );
    delete mesh;
  }

  void MeshManager::jsUpdate( RenderSyncContext& renderCtx )
  {
    js::MeshVector inMeshes;
    js::MeshVector outMeshes;
    renderCtx.syncMeshesFromRenderer( inMeshes, outMeshes );
    if ( !inMeshes.empty() || !outMeshes.empty() )
    {
      Locator::console().printf( Console::srcGfx,
        "MeshManager::jsUpdate adding %i meshes, removing %i meshes",
        inMeshes.size(), outMeshes.size() );
    }
    for ( auto& newMesh : inMeshes )
    {
      addJSMesh( newMesh );
    }
    for ( auto& deletingMesh : outMeshes )
    {
      removeJSMesh( deletingMesh );
    }
  }

  void MeshManager::jsReset()
  {
    for ( auto& it : meshes_ )
    {
      delete it.second;
    }
    meshes_.clear();
  }

  // MeshManager: VBOs

  VBOPtr MeshManager::pushVBO( const vector<Vertex2D>& vertices )
  {
    auto vbo = createVBO( VBO_2D, false );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  VBOPtr MeshManager::pushVBO( const vector<Vertex3D>& vertices )
  {
    auto vbo = createVBO( VBO_3D, false );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  VBOPtr MeshManager::pushVBO( const vector<VertexText3D>& vertices )
  {
    auto vbo = createVBO( VBO_Text, false );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  VBOPtr MeshManager::pushVBO( const vector<MyGUI::Vertex>& vertices )
  {
    auto vbo = createVBO( VBO_MyGUI, false );
    vbo->pushVertices( vertices );
    return move( vbo );
  }

  void vboUploadHelper( vector<VBOPtr>& vbos, Console* console )
  {
    vector<VBO*> dirties;
    for ( auto& buf : vbos )
      if ( ( !buf->uploaded_ || buf->dirty_ ) && !buf->empty() )
        dirties.push_back( buf.get() );

    auto resolveFlags = []( VBO* vbo ) -> gl::BufferStorageMask
    {
      auto mapflags = ( gl::GL_MAP_WRITE_BIT | gl::GL_MAP_READ_BIT | gl::GL_MAP_PERSISTENT_BIT | gl::GL_MAP_COHERENT_BIT );
      auto storeflags = ( mapflags | gl::GL_DYNAMIC_STORAGE_BIT );
      return ( vbo->mappable_ ? ( storeflags | mapflags ) : storeflags );
    };

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
        if ( !dirties[i]->empty() )
          switch ( dirties[i]->type_ )
          {
            case VBO_2D:
            {
              auto& store = dirties[i]->v2d();
              glNamedBufferStorage( dirties[i]->id_,
                store.size() * sizeof( Vertex2D ),
                store.data(),
                resolveFlags( dirties[i] ) );
            } break;
            case VBO_3D:
            {
              auto& store = dirties[i]->v3d();
              glNamedBufferStorage( dirties[i]->id_,
                store.size() * sizeof( Vertex3D ),
                store.data(),
                resolveFlags( dirties[i] ) );
            } break;
            case VBO_Text:
            {
              auto& store = dirties[i]->vt3d();
              glNamedBufferStorage( dirties[i]->id_,
                store.size() * sizeof( VertexText3D ),
                store.data(),
                resolveFlags( dirties[i] ) );
            }
            break;
            case VBO_MyGUI:
            {
              auto& store = dirties[i]->guiv();
              glNamedBufferStorage( dirties[i]->id_,
                store.size() * sizeof( MyGUI::Vertex ),
                store.data(),
                resolveFlags( dirties[i] ) );
            }
            break;
          }
        dirties[i]->uploaded_ = true;
        dirties[i]->dirty_ = false;
      }
    }
  }

  void MeshManager::uploadVBOs()
  {
    for ( auto& vbo : vbos_ )
      vboUploadHelper( vbo, console_.get() );
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
      bool normalize_;
      Record( GLenum type, GLsizei count, size_t size, bool normalize = false ):
        type_( type ), count_( count ), size_( size ), normalize_( normalize ) {}
    };
    vector<Record> recs_;
    GLsizei stride_;
  public:
    AttribWriter(): stride_( 0 ) {}
    inline GLsizei stride() const { return stride_; }
    void add( GLenum type, GLsizei count, bool normalize = false )
    {
      GLsizei size = 0;
      if ( type == GL_FLOAT )
        size = ( count * sizeof( float ) );
      else if ( type == GL_UNSIGNED_BYTE )
        size = ( count * sizeof( uint8_t ) );
      else
        NEKO_EXCEPT( "Unsupported vertex attribute type in writer" );

      recs_.emplace_back( type, count, size, normalize );
      stride_ += size;
    }
    void write( GLuint handle )
    {
      GLuint ptr = 0;
      for ( GLuint i = 0; i < recs_.size(); ++i )
      {
        glEnableVertexArrayAttrib( handle, i );
        glVertexArrayAttribBinding( handle, i, 0 );
        glVertexArrayAttribFormat( handle, i, recs_[i].count_, recs_[i].type_, recs_[i].normalize_ ? GL_TRUE : GL_FALSE, ptr );
        ptr += (GLuint)recs_[i].size_;
      }
    }
  };

  DynamicMeshPtr MeshManager::createDynamic( GLenum drawMode, VBOType vertexType, bool useIndices, bool mappable )
  {
    auto mesh = make_shared<DynamicMesh>( shared_from_this(), vertexType, drawMode, useIndices, mappable );
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

  void MeshManager::destroyFreed()
  {
    glDeleteVertexArrays( (GLsizei)freeVaos_.size(), freeVaos_.data() );
    glDeleteBuffers( (GLsizei)freeBuffers_.size(), freeBuffers_.data() );
    freeVaos_.clear();
    freeBuffers_.clear();
  }

  void MeshManager::uploadVAOs()
  {
    vector<VAO*> dirties;
    for ( auto& vao : vaos_ )
      if ( !vao->uploaded_ && vao->valid() )
        dirties.push_back( vao.get() );

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
        if ( !dirties[i]->ebo_->uploaded_ )
          NEKO_EXCEPT( "EBO used for VAO has not been uploaded" );
        glVertexArrayElementBuffer( ids[i], dirties[i]->ebo_->id_ );
      }

      if ( vbo->type_ == VBO_3D )
      {
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 3 ); // vec3 position
        attribs.add( GL_FLOAT, 3 ); // vec3 normal
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
      else if ( dirties[i]->vbo_->type_ == VBO_MyGUI )
      {
        AttribWriter attribs;
        attribs.add( GL_FLOAT, 3 ); // vec3 position
        attribs.add( GL_UNSIGNED_BYTE, 4, true ); // b4 color
        attribs.add( GL_FLOAT, 2 ); // vec2 texcoord
        attribs.write( ids[i] );
        glVertexArrayVertexBuffer( ids[i], 0, vbo->id_, 0, attribs.stride() );
      }
      else
        NEKO_EXCEPT( "Unknown VBO format" );

      dirties[i]->id_ = ids[i];
      dirties[i]->uploaded_ = true;
    }
  }

  void VAO::begin()
  {
    glBindVertexArray( id_ );
  }

  void VAO::draw( GLenum mode, int count )
  {
    if ( !valid() || !uploaded_ )
      return;
    if ( ebo_ && !ebo_->empty() )
      glDrawElements( mode, count ? count : (GLsizei)ebo_->storage_.size(), GL_UNSIGNED_INT, nullptr );
    else if ( !vbo_->empty() )
      glDrawArrays( mode, 0, count ? count : (GLsizei)vbo_->size() );
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
      if ( ( !ebo->uploaded_ || ebo->dirty_ ) && !ebo->storage_.empty() )
        dirties.push_back( ebo.get() );

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
        assert( !dirties[i]->storage_.empty() );
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
    jsReset();
    dynamics_.clear();
    statics_.clear();
  }

}