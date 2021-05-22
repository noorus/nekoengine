#include "stdafx.h"

#include "neko_types.h"
#include "forwards.h"
#include "scripting.h"
#include "neko_platform.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"
#include "js_math.h"
#include "director.h"

namespace neko {

  void RenderSyncContext::constructed( js::MeshPtr mesh )
  {
    frameNewMeshes.push_back( move( mesh ) );
  }

  void RenderSyncContext::destructed( js::MeshPtr mesh )
  {
    frameDeletedMeshes.push_back( move( mesh ) );
  }

  void RenderSyncContext::constructed( js::ModelPtr model )
  {
    frameNewModels.push_back( move( model ) );
  }

  void RenderSyncContext::destructed( js::ModelPtr model )
  {
    frameDeletedModels.push_back( move( model ) );
  }

  void RenderSyncContext::syncFromScripting()
  {
    lock_.lock();
    // new meshes
    totalNewMeshes.reserve( totalNewMeshes.size() + frameNewMeshes.size() );
    totalNewMeshes.insert( totalNewMeshes.end(), frameNewMeshes.begin(), frameNewMeshes.end() );
    // deleted meshes
    totalDeletedMeshes.reserve( totalDeletedMeshes.size() + frameDeletedMeshes.size() );
    totalDeletedMeshes.insert( totalDeletedMeshes.end(), frameDeletedMeshes.begin(), frameDeletedMeshes.end() );
    // new models
    totalNewModels.reserve( totalNewModels.size() + frameNewModels.size() );
    totalNewModels.insert( totalNewModels.end(), frameNewModels.begin(), frameNewModels.end() );
    // deleted meshes
    totalDeletedModels.reserve( totalDeletedModels.size() + frameDeletedModels.size() );
    totalDeletedModels.insert( totalDeletedModels.end(), frameDeletedModels.begin(), frameDeletedModels.end() );
    lock_.unlock();
    // clear buffers
    frameNewMeshes.clear();
    frameDeletedMeshes.clear();
    frameNewModels.clear();
    frameDeletedModels.clear();
  }

  void RenderSyncContext::syncMeshesFromRenderer( js::MeshVector& outCreated, js::MeshVector& outDeleted )
  {
    lock_.lock();
    outCreated.swap( totalNewMeshes );
    outDeleted.swap( totalDeletedMeshes );
    lock_.unlock();
  }

  void RenderSyncContext::syncModelsFromRenderer( js::ModelVector& outCreated, js::ModelVector& outDeleted )
  {
    lock_.lock();
    outCreated.swap( totalNewModels );
    outDeleted.swap( totalDeletedModels );
    lock_.unlock();
  }

}