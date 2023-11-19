#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"

#ifndef NEKO_NO_SCRIPTING
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
    js::TextVector frameNewTexts;
    js::TextVector frameDeletedTexts;
    // accumulating containers (gfx thread empties)
    js::TextVector totalNewTexts;
    js::TextVector totalDeletedTexts;
    void constructed( js::Text* text );
    void destructed( js::Text* text );
    void syncFromScripting();
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