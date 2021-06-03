#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"
#include "mesh_primitives.h"
#include "js_mesh.h"
#include "scripting.h"

namespace neko {

  using MeshMap = map<size_t, js::Mesh*>;

  class MeshManager: public enable_shared_from_this<MeshManager> {
  private:
    ConsolePtr console_;
    vector<VBOPtr> vbos_[Max_VBOType];
    vector<EBOPtr> ebos_;
    vector<VAOPtr> vaos_;
    DynamicMeshVector dynamics_;
    StaticMeshVector statics_;
    MeshMap meshes_;
    void addJSMesh( js::Mesh* mesh );
    void removeJSMesh( js::Mesh* mesh );
  public:
    MeshManager( ConsolePtr console ): console_( move( console ) ) {}
    void jsUpdate( RenderSyncContext& renderCtx );
    VBOPtr pushVBO( const vector<Vertex3D>& vertices );
    VBOPtr pushVBO( const vector<Vertex2D>& vertices );
    VBOPtr pushVBO( const vector<VertexText3D>& vertices );
    void uploadVBOs();
    VAOPtr pushVAO( VBOPtr verticesVBO );
    VAOPtr pushVAO( VBOPtr verticesVBO, EBOPtr indicesEBO );
    void uploadVAOs();
    EBOPtr pushEBO( const vector<GLuint>& indices );
    void uploadEBOs();
    VBOPtr createVBO( VBOType type );
    void freeVBO( VBOPtr vbo );
    EBOPtr createEBO();
    void freeEBO( EBOPtr ebo );
    VAOPtr createVAO();
    void freeVAO( VAOPtr vao );
    DynamicMeshPtr createDynamic( GLenum drawMode, VBOType vertexType );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices );
    void jsReset();
    void teardown();
  };

}