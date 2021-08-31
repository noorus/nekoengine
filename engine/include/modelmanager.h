#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"
#include "mesh_primitives.h"
#include "js_model.h"
#include "scripting.h"

namespace neko {

  using ModelMap = map<size_t, js::Model*>;

  class ModelManager: public enable_shared_from_this<ModelManager>, public nocopy {
  private:
    ConsolePtr console_;
    ModelMap models_;
    void addJSModel( js::Model* model );
    void removeJSModel( js::Model* model );
  public:
    ModelManager( ConsolePtr console ): console_( move( console ) ) {}
    void jsUpdate( RenderSyncContext& renderCtx );
    void jsReset();
    void teardown();
    inline ModelMap& models() { return models_; }
  };

  using ModelManagerPtr = shared_ptr<ModelManager>;

}

#endif