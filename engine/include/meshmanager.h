#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

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

}