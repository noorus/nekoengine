#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

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

  void RenderSyncContext::constructed( js::Mesh* mesh )
  {
    frameNewMeshes.push_back( mesh );
  }

  void RenderSyncContext::destructed( js::Mesh* mesh )
  {
    frameDeletedMeshes.push_back( mesh );
  }

  void RenderSyncContext::constructed( js::Model* model )
  {
    frameNewModels.push_back( model );
  }

  void RenderSyncContext::destructed( js::Model* model )
  {
    frameDeletedModels.push_back( model );
  }

  void RenderSyncContext::constructed( js::Text* text )
  {
    frameNewTexts.push_back( text );
  }

  void RenderSyncContext::destructed( js::Text* text )
  {
    frameDeletedTexts.push_back( text );
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
    // deleted models
    totalDeletedModels.reserve( totalDeletedModels.size() + frameDeletedModels.size() );
    totalDeletedModels.insert( totalDeletedModels.end(), frameDeletedModels.begin(), frameDeletedModels.end() );
    // new texts
    totalNewTexts.reserve( totalNewTexts.size() + frameNewTexts.size() );
    totalNewTexts.insert( totalNewTexts.end(), frameNewTexts.begin(), frameNewTexts.end() );
    // deleted texts
    totalDeletedTexts.reserve( totalDeletedTexts.size() + frameDeletedTexts.size() );
    totalDeletedTexts.insert( totalDeletedTexts.end(), frameDeletedTexts.begin(), frameDeletedTexts.end() );
    lock_.unlock();
    // clear buffers
    frameNewMeshes.clear();
    frameDeletedMeshes.clear();
    frameNewModels.clear();
    frameDeletedModels.clear();
    frameNewTexts.clear();
    frameDeletedTexts.clear();
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

  void RenderSyncContext::syncTextsFromRenderer( js::TextVector& outCreated, js::TextVector& outDeleted )
  {
    lock_.lock();
    outCreated.swap( totalNewTexts );
    outDeleted.swap( totalDeletedTexts );
    lock_.unlock();
  }

  void RenderSyncContext::resetFromRenderer()
  {
    lock_.lock();
    frameNewMeshes.clear();
    frameDeletedMeshes.clear();
    frameNewModels.clear();
    frameDeletedModels.clear();
    frameNewTexts.clear();
    frameDeletedTexts.clear();
    totalNewModels.clear();
    totalDeletedModels.clear();
    totalNewMeshes.clear();
    totalDeletedMeshes.clear();
    totalNewTexts.clear();
    totalDeletedTexts.clear();
    lock_.unlock();
  }

}

#endif