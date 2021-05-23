#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"
#include "mesh_primitives.h"
#include "js_model.h"
#include "scripting.h"

namespace neko {

  using ModelMap = map<size_t, js::ModelPtr>;

  class ModelManager : public enable_shared_from_this<ModelManager> {
  private:
    ConsolePtr console_;
    ModelMap models_;
    void addJSModel( js::ModelPtr& model );
    void removeJSModel( js::ModelPtr& model );

  public:
    ModelManager( ConsolePtr console ): console_( move( console ) ) {}
    void jsUpdate( RenderSyncContext& renderCtx );
    void teardown();
    inline ModelMap& models() { return models_; }
  };

  using ModelManagerPtr = shared_ptr<ModelManager>;

}