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

  using v8::Handle;
  using v8::HandleScope;
  using v8::ObjectTemplate;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Global;

  class IsolScope {
  public:
    Isolate* isol_;
    IsolScope( Isolate* isol ): isol_( isol )
    {
      isol_->Enter();
    }
    ~IsolScope()
    {
      isol_->Exit();
    }
  };

  Scripting::Scripting( EnginePtr engine ):
    Subsystem( move( engine ) ),
    isolate_( nullptr )
  {
    engine_->console()->printf( Console::srcScripting, "Initializing V8 v%s", v8::V8::GetVersion() );

    utfString rootDirectory = platform::getCurrentDirectory();
    rootDirectory.append( "\\data\\v8\\" NEKO_CONFIG_SUBDIRNAME "\\" );

    if ( !v8::V8::InitializeICU( ( rootDirectory + "icudtl.dat" ).c_str() ) )
      NEKO_EXCEPT( "V8 ICU initialization failed" );

    v8::V8::InitializeExternalStartupData(
      ( rootDirectory + "natives_blob.bin" ).c_str(),
      ( rootDirectory + "snapshot_blob.bin" ).c_str()
    );

    platform_ = move( v8::platform::NewDefaultPlatform(
      0, // threadpoolsize
      v8::platform::IdleTaskSupport::kDisabled // idle task support
    ) );

    v8::V8::InitializePlatform( platform_.get() );

    v8::V8::Initialize();

    arrayAllocator_ = unique_ptr<v8::ArrayBuffer::Allocator>( move(
      v8::ArrayBuffer::Allocator::NewDefaultAllocator() ) );
    if ( !arrayAllocator_ )
      NEKO_EXCEPT( "Failed to create default V8 array allocator" );

    Isolate::CreateParams params;
    params.array_buffer_allocator = arrayAllocator_.get();
    isolate_ = Isolate::New( params );
    if ( !isolate_ )
      NEKO_EXCEPT( "V8 default isolation creation failed" );
  }

  void Scripting::initialize()
  {
    IsolScope isolateScope( isolate_ );
    HandleScope handleScope( isolate_ );

    Local<ObjectTemplate> global = ObjectTemplate::New( isolate_ );

    // init natives here for global template

    Local<Context> context = Context::New( isolate_, nullptr, global );
    if ( context.IsEmpty() )
      NEKO_EXCEPT( "V8 default context creation failed" );

    Context::Scope contextScope( context );

    context_.Reset( isolate_, context );

    // init natives here for global context
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