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
    void constructed( js::MeshPtr mesh );
    void destructed( js::MeshPtr mesh );
    void constructed( js::ModelPtr model );
    void destructed( js::ModelPtr model );
    void syncFromScripting();
    void syncMeshesFromRenderer( js::MeshVector& outCreated, js::MeshVector& outDeleted );
    void syncModelsFromRenderer( js::ModelVector& outCreated, js::ModelVector& outDeleted );
  };

  class Director {
  protected:
    RenderSyncContext renderSync_; //!< Synchronizer between game and renderer objects
  public:
    inline RenderSyncContext& renderSync() { return renderSync_; }
    inline void reset() {}
  };

}