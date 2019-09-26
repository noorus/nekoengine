#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "forwards.h"
#include "js_util.h"

namespace neko {

  namespace js {

#   define JS_WRAPPER_SETMEMBER(tpl,cls,x) tpl->PrototypeTemplate()->Set( \
      util::allocStringConserve( #x, isolate ), \
      FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = static_cast<cls*>( args.Data().As<v8::External>()->Value() ); \
        self->js_##x( args.GetIsolate(), args ); \
      }, v8::External::New( isolate, (void*)this ) ) )

#   define JS_WRAPPER_SETACCESSOR(obj,cls,x,valInternal) obj->PrototypeTemplate()->SetAccessor( \
      util::allocStringConserve( #x, isolate ), []( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info ) { \
        auto self = info.This(); \
        assert( self->InternalFieldCount() >= Max_WrapField ); \
        assert( self->GetInternalField( WrapField_Type )->Uint32Value( info.GetIsolate()->GetCurrentContext() ).FromMaybe( 9999 ) == internalType ); \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_get##valInternal( prop, info ); \
      }, []( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info ) { \
        auto self = info.This(); \
        assert( self->InternalFieldCount() >= Max_WrapField ); \
        assert( self->GetInternalField( WrapField_Type )->Uint32Value( info.GetIsolate()->GetCurrentContext() ).FromMaybe( 9999 ) == internalType ); \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_set##valInternal( prop, value, info ); \
      } )

#   define JS_WRAPPER_SETINTMEMBER(obj,cls,x) obj->PrototypeTemplate()->Set( \
      util::allocStringConserve( #x, isolate ), FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = args.This(); \
        assert( self->InternalFieldCount() >= Max_WrapField ); \
        assert( self->GetInternalField( WrapField_Type )->Uint32Value( args.GetIsolate()->GetCurrentContext() ).FromMaybe( 9999 ) == internalType ); \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_##x( args ); \
    } ) )

    enum WrappedType {
      Wrapped_Console,
      Wrapped_Vector2
    };

    enum WrappedFieldIndex {
      WrapField_Type,
      WrapField_Pointer,
      Max_WrapField
    };

    template <class T>
    class StaticObjectWrapper {
      friend class neko::Scripting;
    protected:
      //! My JavaScript-exported class name
      static string className;
      //! My magic internal type
      static WrappedType internalType;
    protected:
      void wrapperRegisterObject( Isolate* isolate, V8Object global )
      {
        global->Set( isolate->GetCurrentContext(),
          js::util::allocStringConserve( className, isolate ),
          wrapperImplCreateFunction( isolate )
        ).FromJust();
      }
      inline v8::Local<v8::Object> wrapperImplCreateFunction( Isolate* isolate )
      {
        v8::EscapableHandleScope scope( isolate );

        auto tpl = FunctionTemplate::New( isolate );
        tpl->SetClassName( util::allocString( className, isolate ) );
        tpl->InstanceTemplate()->SetInternalFieldCount( Max_WrapField );

        registerGlobals( isolate, tpl );

        auto classFn = tpl->GetFunction( isolate->GetCurrentContext() ).ToLocalChecked();
        auto classInst = classFn->NewInstance( isolate->GetCurrentContext() ).ToLocalChecked();
        classInst->SetInternalField( WrapField_Type, v8::Uint32::New( isolate, internalType ) );
        classInst->SetAlignedPointerInInternalField( WrapField_Pointer, this );

        return scope.Escape( classInst );
      }
    protected:
      virtual void registerGlobals( Isolate* isolate, v8::Local<v8::FunctionTemplate>& tpl ) = 0;
    };

    template <class T>
    class DynamicObjectWrapper {
      friend class neko::Scripting;
      template <class T>
      friend class DynamicObjectConstructor;
    protected:
      //! My JavaScript-exported class name
      static string className;
      //! My magic internal type
      static WrappedType internalType;
      //! Myself
      v8::Persistent<v8::Object> handle_;
      int refs_;
    private:
      static void WeakCallback( const v8::WeakCallbackInfo<DynamicObjectWrapper>& data )
      {
        auto wrap = (T*)data.GetParameter();
        assert( wrap->refs_ == 0 );
        wrap->handle_.Reset();
        delete wrap;
      }
    protected:
      inline void MakeWeak()
      {
        persistent().SetWeak( this, WeakCallback, v8::WeakCallbackType::kParameter );
      }
      virtual void Ref()
      {
        assert( !persistent().IsEmpty() );
        persistent().ClearWeak();
        refs_++;
      }
      virtual void Unref()
      {
        assert( !persistent().IsEmpty() );
        assert( !persistent().IsWeak() );
        assert( refs_ > 0 );
        if ( --refs_ == 0 )
          MakeWeak();
      }
    protected:
      inline void wrapperWrap( v8::Handle<v8::Object>& handle )
      {
        assert( persistent().IsEmpty() );
        assert( handle->InternalFieldCount() == Max_WrapField );
        handle->SetInternalField( WrapField_Type, v8::Uint32::New( handle->GetIsolate(), internalType ) );
        handle->SetAlignedPointerInInternalField( WrapField_Pointer, this );
        persistent().Reset( v8::Isolate::GetCurrent(), handle );
        MakeWeak();
        handle->GetIsolate()->AdjustAmountOfExternalAllocatedMemory( sizeof( T ) );
      }
      static inline V8FunctionTemplate wrapperImplConstructor( Isolate* isolate )
      {
        v8::EscapableHandleScope scope( isolate );

        auto tpl = FunctionTemplate::New( isolate, T::jsConstructor );
        tpl->SetClassName( util::allocString( className, isolate ) );
        tpl->InstanceTemplate()->SetInternalFieldCount( Max_WrapField );

        T::registerExport( isolate, tpl );

        return scope.Escape( tpl );
      }
    protected:
      template <class T>
      static inline T* Unwrap( v8::Local<v8::Object> handle )
      {
        assert( !handle.IsEmpty() );
        assert( handle->InternalFieldCount() > 0 );
        // Cast to ObjectWrap before casting to T.  A direct cast from void
        // to T won't work right when T has more than one base class.
        void* ptr = handle->GetAlignedPointerFromInternalField( 0 );
        ObjectWrap* wrap = static_cast<ObjectWrap*>( ptr );
        return static_cast<T*>( wrap );
      }
    public:
      DynamicObjectWrapper(): refs_( 0 )
      {
      }
      virtual ~DynamicObjectWrapper()
      {
        if ( persistent().IsEmpty() )
          return;
        persistent().ClearWeak();
        persistent().Reset();
      }
      inline v8::Local<v8::Object> handle()
      {
        return handle( v8::Isolate::GetCurrent() );
      }
      inline v8::Local<v8::Object> handle( v8::Isolate* isolate )
      {
        return v8::Local<v8::Object>::New( isolate, persistent() );
      }
      inline v8::Persistent<v8::Object>& persistent()
      {
        return handle_;
      }
    };

    template <class T>
    class DynamicObjectConstructor {
    protected:
      //! My JS constructor store
      v8::Eternal<v8::FunctionTemplate> constructor;
    public:
      void initialize( Isolate* isolate, v8::Local<v8::ObjectTemplate>& exports )
      {
        auto tpl = T::wrapperImplConstructor( isolate );
        exports->Set( util::allocStringConserve( T::className, isolate ), tpl );
        constructor.Set( isolate, tpl );
      }
    };

  }

}