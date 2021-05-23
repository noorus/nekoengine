#include "stdafx.h"
#include "gfx_types.h"
#include "modelmanager.h"
#include "renderer.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  using namespace gl;

  void ModelManager::addJSModel( js::ModelPtr& model )
  {
    if ( models_.find( model->model().id_ ) != models_.end() )
    {
      NEKO_EXCEPT( "Model map key already exists" );
    }
    models_[model->model().id_] = model;
  }

  void ModelManager::removeJSModel( js::ModelPtr& model )
  {
    models_.erase( model->model().id_ );
  }

  void ModelManager::jsUpdate( RenderSyncContext& renderCtx )
  {
    js::ModelVector inModels;
    js::ModelVector outModels;
    renderCtx.syncModelsFromRenderer( inModels, outModels );
    if ( !inModels.empty() || !outModels.empty() )
    {
      Locator::console().printf( Console::srcGfx,
        "ModelManager::jsUpdate adding %i models, removing %i models",
        inModels.size(), outModels.size() );
    }
    for ( auto& newModel : inModels )
    {
      addJSModel( newModel );
    }
    for ( auto& deletingModel : outModels )
    {
      removeJSModel( deletingModel );
    }
  }

  void ModelManager::teardown()
  {
    //models_.clear();
  }

}