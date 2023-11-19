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
#include "components.h"

namespace neko {

  void RenderSyncContext::constructed( js::Text* text )
  {
    frameNewTexts.push_back( text );
  }

  void RenderSyncContext::destructed( js::Text* text )
  {
    frameDeletedTexts.push_back( text );
  }

  void RenderSyncContext::createScene( const vec2& resolution )
  {
    sceneLock_.lock();
    assert( !scene_ );
    scene_ = make_shared<SManager>( resolution );
    sceneLock_.unlock();
  }

  void RenderSyncContext::destroyScene()
  {
    sceneLock_.lock();
    scene_.reset();
    sceneLock_.unlock();
  }

  SManagerPtr RenderSyncContext::lockSceneWrite()
  {
    sceneLock_.lock();
    return scene_;
  }

  void RenderSyncContext::unlockSceneWrite()
  {
    sceneLock_.unlock();
  }

  SManagerPtr RenderSyncContext::lockSceneShared()
  {
    sceneLock_.lockShared();
    return scene_;
  }

  void RenderSyncContext::unlockSceneShared()
  {
    sceneLock_.unlockShared();
  }

  void RenderSyncContext::syncFromScripting()
  {
    lock_.lock();
    // new texts
    totalNewTexts.reserve( totalNewTexts.size() + frameNewTexts.size() );
    totalNewTexts.insert( totalNewTexts.end(), frameNewTexts.begin(), frameNewTexts.end() );
    // deleted texts
    totalDeletedTexts.reserve( totalDeletedTexts.size() + frameDeletedTexts.size() );
    totalDeletedTexts.insert( totalDeletedTexts.end(), frameDeletedTexts.begin(), frameDeletedTexts.end() );
    lock_.unlock();
    // clear buffers
    frameNewTexts.clear();
    frameDeletedTexts.clear();
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
    frameNewTexts.clear();
    frameDeletedTexts.clear();
    totalNewTexts.clear();
    totalDeletedTexts.clear();
    lock_.unlock();
  }

}

#endif