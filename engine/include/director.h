#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "js_mesh.h"
#include "js_model.h"

namespace neko {

  struct RenderSyncContext
  {
    platform::RWLock lock_;
    // per frame containers
    js::MeshVector frameNewMeshes;
    js::MeshVector frameDeletedMeshes;
    js::ModelVector frameNewModels;
    js::ModelVector frameDeletedModels;
    // accumulating containers (gfx thread empties)
    js::MeshVector totalNewMeshes;
    js::MeshVector totalDeletedMeshes;
    js::ModelVector totalNewModels;
    js::ModelVector totalDeletedModels;
    void constructed( js::Mesh* mesh );
    void destructed( js::Mesh* mesh );
    void constructed( js::Model* model );
    void destructed( js::Model* model );
    void syncFromScripting();
    void syncMeshesFromRenderer( js::MeshVector& outCreated, js::MeshVector& outDeleted );
    void syncModelsFromRenderer( js::ModelVector& outCreated, js::ModelVector& outDeleted );
    void resetFromRenderer();
  };

  class Director {
  protected:
    RenderSyncContext renderSync_; //!< Synchronizer between game and renderer objects
  public:
    inline RenderSyncContext& renderSync() { return renderSync_; }
    inline void reset() {}
  };

}