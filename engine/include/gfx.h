#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "camera.h"

namespace neko {

#pragma pack( push, 1 )
  struct Vertex3D {
    float x, y, z; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
  };
  struct Vertex2D {
    float x, y; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
  };
#pragma pack( pop )

  template <class T>
  class VBO {
  public:
    vector<T> storage_;
    GLuint id;
    bool uploaded;
    VBO(): id( 0 ), uploaded( false ) {}
  };

  template <class T>
  class VBOVector: public vector<VBO<T>> {};

  class VAO {
  public:
    enum VBOType {
      VBO_3D,
      VBO_2D
    } vboType_;
    size_t vbo_;
    GLuint id;
    bool uploaded_;
    size_t size_;
    void draw( GLenum mode );
    VAO( VBOType type, size_t vbo ): vboType_( type ), vbo_( vbo ), id( 0 ), size_( 0 ), uploaded_( false ) {}
  };

  using VAOVector = vector<VAO>;

  class MeshManager {
  private:
    VBOVector<Vertex3D> vbos3d_;
    VBOVector<Vertex2D> vbos2d_;
    VAOVector vaos_;
  public:
    size_t pushVBO( vector<Vertex3D> vertices );
    size_t pushVBO( vector<Vertex2D> vertices );
    void uploadVBOs();
    size_t pushVAO( VAO::VBOType type, size_t verticesVBO );
    VAO& getVAO( size_t index ) { return vaos_[index]; }
    void uploadVAOs();
    void teardown();
  };

  using MeshManagerPtr = shared_ptr<MeshManager>;

  class Gfx: public Subsystem {
  protected:
    SDL_Window* window_;
    SDL_Surface* screenSurface_;
    SDL_DisplayMode displayMode_;
    SDL_GLContext glContext_;
    ShadersPtr shaders_;
    MeshManagerPtr meshes_;
    unique_ptr<Camera> camera_;
    void preInitialize();
  public:
    void postInitialize();
    Gfx( EnginePtr engine );
    virtual void preUpdate( GameTime time ) override;
    virtual void tick( GameTime tick, GameTime time ) override;
    virtual void postUpdate( GameTime delta, GameTime tick ) override;
    void shutdown();
    void restart();
    virtual ~Gfx();
  };

}