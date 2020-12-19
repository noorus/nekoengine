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

  void RenderSyncContext::syncFromScripting()
  {
    lock_.lock();
    totalNewMeshes.reserve( totalNewMeshes.size() + frameNewMeshes.size() );
    totalNewMeshes.insert( totalNewMeshes.end(), frameNewMeshes.begin(), frameNewMeshes.end() );
    totalDeletedMeshes.reserve( totalDeletedMeshes.size() + frameDeletedMeshes.size() );
    totalDeletedMeshes.insert( totalDeletedMeshes.end(), frameDeletedMeshes.begin(), frameDeletedMeshes.end() );
    lock_.unlock();
    frameNewMeshes.clear();
    frameDeletedMeshes.clear();
  }

  void RenderSyncContext::syncFromRenderer( js::MeshVector& outCreated, js::MeshVector& outDeleted )
  {
    lock_.lock();
    outCreated.swap( totalNewMeshes );
    outDeleted.swap( totalDeletedMeshes );
    lock_.unlock();
  }

}