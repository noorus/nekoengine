#include "stdafx.h"

#include "neko_types.h"
#include "forwards.h"
#include "scripting.h"
#include "neko_platform.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"

#include "js_console.h"
#include "js_util.h"

namespace neko {

#ifdef _DEBUG
# define NEKO_CONFIG_SUBDIRNAME "debug"
#else
# define NEKO_CONFIG_SUBDIRNAME "release"
#endif

  Scripting::Scripting( EnginePtr engine ):
    Subsystem( move( engine ) ),
    v8::ArrayBuffer::Allocator()
  {
    engine_->console()->printf( Console::srcScripting, "Initializing V8 v%s", v8::V8::GetVersion() );

    rootDirectory_ = platform::getCurrentDirectory();
    dataDirectory_ = rootDirectory_;
    dataDirectory_.append( "\\data\\v8\\" NEKO_CONFIG_SUBDIRNAME "\\" );

    if ( !v8::V8::InitializeICU( ( dataDirectory_ + "icudtl.dat" ).c_str() ) )
      NEKO_EXCEPT( "V8 ICU initialization failed" );

    v8::V8::InitializeExternalStartupDataFromFile(
      ( dataDirectory_ + "snapshot_blob.bin" ).c_str()
    );

    platform_ = move( v8::platform::NewDefaultPlatform(
      0, // threadpoolsize
      v8::platform::IdleTaskSupport::kDisabled // idle task support
    ) );

    v8::V8::InitializePlatform( platform_.get() );

    v8::V8::Initialize();
  }

  void Scripting::initialize()
  {
    global_ = make_shared<ScriptingContext>( this, this,
      utf8String( rootDirectory_ ) + R"(\script\)" );
  }

  void Scripting::postInitialize()
  {
    v8::HandleScope handleScope( global_->isolate() );
    global_->addAndRunScript( "initialization.js" );
  }

  void Scripting::shutdown()
  {
    global_.reset();
  }

  void Scripting::preUpdate( GameTime time )
  {
    //
  }

  void Scripting::tick( GameTime tick, GameTime time )
  {
    global_->tick( tick, time );
    global_->process();
  }

  void Scripting::postUpdate( GameTime delta, GameTime tick )
  {
    //
  }

  Scripting::~Scripting()
  {
    shutdown();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    platform_.reset();
  }

  // V8 Allocator interface implementation

  void* Scripting::Allocate( size_t length )
  {
    return Locator::memory().allocZeroed( Memory::Sector::Scripting, length );
  }

  void* Scripting::AllocateUninitialized( size_t length )
  {
    return Locator::memory().alloc( Memory::Sector::Scripting, length );
  }

  void Scripting::Free( void* data, size_t length )
  {
    Locator::memory().free( Memory::Sector::Scripting, data );
  }

}