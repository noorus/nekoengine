#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"

#ifndef NEKO_NO_SCRIPTING
# include "js_mesh.h"
# include "js_model.h"
# include "js_text.h"
#endif

namespace neko {

#ifndef NEKO_NO_SCRIPTING

  struct RenderSyncContext {
  private:
    SManagerPtr scene_;
    platform::RWLock sceneLock_;
  public:
    platform::RWLock lock_;
    // per frame containers
    js::MeshVector frameNewMeshes;
    js::MeshVector frameDeletedMeshes;
    js::ModelVector frameNewModels;
    js::ModelVector frameDeletedModels;
    js::TextVector frameNewTexts;
    js::TextVector frameDeletedTexts;
    // accumulating containers (gfx thread empties)
    js::MeshVector totalNewMeshes;
    js::MeshVector totalDeletedMeshes;
    js::ModelVector totalNewModels;
    js::ModelVector totalDeletedModels;
    js::TextVector totalNewTexts;
    js::TextVector totalDeletedTexts;
    void constructed( js::Mesh* mesh );
    void destructed( js::Mesh* mesh );
    void constructed( js::Model* model );
    void destructed( js::Model* model );
    void constructed( js::Text* text );
    void destructed( js::Text* text );
    void syncFromScripting();
    void syncMeshesFromRenderer( js::MeshVector& outCreated, js::MeshVector& outDeleted );
    void syncModelsFromRenderer( js::ModelVector& outCreated, js::ModelVector& outDeleted );
    void syncTextsFromRenderer( js::TextVector& outCreated, js::TextVector& outDeleted );
    void resetFromRenderer();
    void createScene( const vec2& resolution );
    void destroyScene();
    SManagerPtr lockSceneWrite();
    void unlockSceneWrite();
    SManagerPtr lockSceneShared();
    void unlockSceneShared();
  };

#else

  struct RenderSyncContext
  {
    int dummy_;
  };

#endif

  class Director: public nocopy {
  protected:
    RenderSyncContext renderSync_; //!< Synchronizer between game and renderer objects
  public:
    inline RenderSyncContext& renderSync() { return renderSync_; }
    inline void reset() {}
  };

}