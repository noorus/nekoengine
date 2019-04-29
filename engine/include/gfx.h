#pragma once
#include "subsystem.h"
#include "forwards.h"

namespace neko {

#pragma pack( push, 1 )
  struct Vertex {
    float x, y, z;
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

  using VertexBufferVector = vector<VBO<Vertex>>;

  class VAO {
  public:
    size_t vbo_;
    GLuint id;
    bool uploaded_;
    size_t size_;
    void draw();
    VAO( size_t vbo ): vbo_( vbo ), id( 0 ), size_( 0 ), uploaded_( false ) {}
  };

  using VAOVector = vector<VAO>;

  class MeshManager {
  private:
    VertexBufferVector buffers_;
    VAOVector vaos_;
  public:
    size_t pushVBO( vector<Vertex> vertices );
    void uploadVBOs();
    size_t pushVAO( size_t verticesVBO );
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