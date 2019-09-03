#include "stdafx.h"
#ifndef NEKO_NO_SCRIPTING

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

    v8::V8::InitializeExternalStartupData(
      ( dataDirectory_ + "natives_blob.bin" ).c_str(),
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
    global_ = make_shared<ScriptingContext>( this, this );
    global_->scriptDirectory_ = rootDirectory_;
    global_->scriptDirectory_.append( "\\script\\" );
  }

  void Scripting::postInitialize()
  {
    utf8String scriptFile = global_->scriptDirectory_ + "initialization.js";
    Script script( global_, scriptFile );
    script.compile( global_->ctx() );
    script.execute( global_->ctx() );
  }

  void* Scripting::Allocate( size_t length )
  {
    return Locator::memory().allocZeroed( Memory::Scripting, length );
  }

  void* Scripting::AllocateUninitialized( size_t length )
  {
    return Locator::memory().alloc( Memory::Scripting, length );
  }

  void Scripting::Free( void* data, size_t length )
  {
    Locator::memory().free( Memory::Scripting, data );
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
    global_->tick();
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

}

#endif // !NEKO_NO_SCRIPTING