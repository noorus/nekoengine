#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "js_mesh.h"

namespace neko {

  struct RenderSyncContext
  {
    platform::RWLock lock_;
    // per frame containers
    vector<js::MeshPtr> frameNewMeshes;
    vector<js::MeshPtr> frameDeletedMeshes;
    // accumulating containers (gfx thread empties)
    vector<js::MeshPtr> totalNewMeshes;
    vector<js::MeshPtr> totalDeletedMeshes;
    void constructed( js::MeshPtr mesh );
    void destructed( js::MeshPtr mesh );
    void syncFromScripting();
    void syncFromRenderer( js::MeshVector& outCreated, js::MeshVector& outDeleted );
  };

  class Director {
  protected:
    RenderSyncContext renderSync_; //!< Synchronizer between game and renderer objects
  public:
    inline RenderSyncContext& renderSync() { return renderSync_; }
    inline void reset() {}
  };

}