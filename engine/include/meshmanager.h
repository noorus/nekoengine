#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"
#include "mesh_primitives.h"
#include "js_mesh.h"
#include "scripting.h"
#include "director.h"

namespace neko {

#ifndef NEKO_NO_SCRIPTING
  using MeshMap = map<size_t, js::Mesh*>;
#endif

  class MeshManager: public enable_shared_from_this<MeshManager> {
  private:
    ConsolePtr console_;
    vector<VBOPtr> vbos_[Max_VBOType];
    vector<EBOPtr> ebos_;
    vector<VAOPtr> vaos_;
    vector<GLuint> freeVaos_;
    vector<GLuint> freeBuffers_;
    DynamicMeshVector dynamics_;
    StaticMeshVector statics_;
#ifndef NEKO_NO_SCRIPTING
    MeshMap meshes_;
    void addJSMesh( js::Mesh* mesh );
    void removeJSMesh( js::Mesh* mesh );
#endif
  public:
    MeshManager( ConsolePtr console ): console_( move( console ) ) {}
#ifndef NEKO_NO_SCRIPTING
    void jsUpdate( RenderSyncContext& renderCtx );
    void jsReset();
#endif
    VBOPtr pushVBO( const vector<Vertex3D>& vertices );
    VBOPtr pushVBO( const vector<Vertex2D>& vertices );
    VBOPtr pushVBO( const vector<VertexText3D>& vertices );
    VBOPtr pushVBO( const vector<MyGUI::Vertex>& vertices );
    void uploadVBOs();
    VAOPtr pushVAO( VBOPtr verticesVBO );
    VAOPtr pushVAO( VBOPtr verticesVBO, EBOPtr indicesEBO );
    void uploadVAOs();
    EBOPtr pushEBO( const vector<GLuint>& indices );
    void uploadEBOs();
    VBOPtr createVBO( VBOType type, bool mappable );
    void freeVBO( VBOPtr vbo );
    EBOPtr createEBO();
    void freeEBO( EBOPtr ebo );
    VAOPtr createVAO();
    void freeVAO( VAOPtr vao );
    void destroyFreed();
    DynamicMeshPtr createDynamic( GLenum drawMode, VBOType vertexType, bool useIndices, bool mappable );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex3D> verts );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex3D> verts, vector<GLuint> indices );
    void teardown();
  };

}