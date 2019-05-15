#include "stdafx.h"
#include "neko_types.h"
#include "forwards.h"
#include "scripting.h"
#include "neko_platform.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"

namespace neko {

#ifdef _DEBUG
# define NEKO_CONFIG_SUBDIRNAME "debug"
#else
# define NEKO_CONFIG_SUBDIRNAME "release"
#endif

  using v8::HandleScope;
  using v8::ObjectTemplate;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Global;

  Scripting::Scripting( EnginePtr engine ):
    Subsystem( move( engine ) ),
    v8::ArrayBuffer::Allocator(),
    isolate_( nullptr )
  {
    engine_->console()->printf( Console::srcScripting, "Initializing V8 v%s", v8::V8::GetVersion() );

    auto rootDirectory = platform::getCurrentDirectory();
    auto dataDirectory = rootDirectory;
    dataDirectory.append( "\\data\\v8\\" NEKO_CONFIG_SUBDIRNAME "\\" );

    global_ = make_shared<ScriptingContext>();
    global_->scriptDirectory_ = rootDirectory;
    global_->scriptDirectory_.append( "\\script\\" );

    if ( !v8::V8::InitializeICU( ( dataDirectory + "icudtl.dat" ).c_str() ) )
      NEKO_EXCEPT( "V8 ICU initialization failed" );

    v8::V8::InitializeExternalStartupData(
      ( dataDirectory + "natives_blob.bin" ).c_str(),
      ( dataDirectory + "snapshot_blob.bin" ).c_str()
    );

    platform_ = move( v8::platform::NewDefaultPlatform(
      0, // threadpoolsize
      v8::platform::IdleTaskSupport::kDisabled // idle task support
    ) );

    v8::V8::InitializePlatform( platform_.get() );

    v8::V8::Initialize();

    Isolate::CreateParams params;
    params.array_buffer_allocator = this;
    isolate_ = Isolate::New( params );
    if ( !isolate_ )
      NEKO_EXCEPT( "V8 default isolation creation failed" );
  }

  void Scripting::initialize()
  {
    Isolate::Scope isolateScope( isolate_ );
    HandleScope handleScope( isolate_ );

    Local<ObjectTemplate> global = ObjectTemplate::New( isolate_ );

    // init natives here for global template

    Local<Context> context = Context::New( isolate_, nullptr, global );
    if ( context.IsEmpty() )
      NEKO_EXCEPT( "V8 default context creation failed" );

    Context::Scope contextScope( context );

    context_.Reset( isolate_, context );

    // init natives here for global context

    global_->console_ = engine_->console();
    global_->isolate_ = isolate_;

    utf8String scriptFile = global_->scriptDirectory_ + "initialization.js";
    Script script( global_, scriptFile );
    script.compile( context_ );
    script.execute( context_ );
  }

  void* Scripting::Allocate( size_t length )
  {
    return Locator::memory().allocZeroed( Memory::Scripting, length );
  }

  void* Scripting::AllocateUninitialized( size_t length )
  {
    return  Locator::memory().alloc( Memory::Scripting, length );
  }

  void Scripting::Free( void* data, size_t length )
  {
    Locator::memory().free( Memory::Scripting, data );
  }

  void Scripting::shutdown()
  {
    context_.Reset();
    assert( context_.IsEmpty() );
  }

  void Scripting::preUpdate( GameTime time )
  {
    //
  }

  void Scripting::tick( GameTime tick, GameTime time )
  {
    //
  }

  void Scripting::postUpdate( GameTime delta, GameTime tick )
  {
    //
  }

  Scripting::~Scripting()
  {
    shutdown();
    if ( isolate_ )
    {
      isolate_->ContextDisposedNotification();
      // None of these matter. It still leaks f*****g memory.
      /*isolate_->IdleNotificationDeadline( platform_->MonotonicallyIncreasingTime() + 1.0 );
      isolate_->LowMemoryNotification();
      Sleep( 2000 );*/
      isolate_->TerminateExecution();
      isolate_->Dispose();
    }
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
  }

}