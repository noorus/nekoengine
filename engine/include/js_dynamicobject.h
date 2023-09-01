#pragma once

#include "neko_types.h"
#include "neko_exception.h"

#include "js_util.h"
#include "js_types.h"
#include "js_dynamicregistry.h"

namespace neko {

  namespace js {

    class DirtyableObject {
    protected:
      bool dirty_ = true;
    public:
      inline bool dirty() const noexcept { return dirty_; }
      inline void markDirty() { dirty_ = true; }
      inline void markClean() { dirty_ = false; }
    };

    // clang-format off

    #define JS_DYNAMICOBJECT_DECLARE_STATICS( T, Base ) \
      utf8String DynamicObject<T, Base>::className( #T ); \
      WrappedType DynamicObject<T, Base>::internalType = Wrapped_ ## T ## ;

    #define JS_DYNAMICOBJECT_DECLARE_STATICS_NAMED( T, Base, CName ) \
      utf8String DynamicObject<T, Base>::className( #CName ); \
      WrappedType DynamicObject<T, Base>::internalType = Wrapped_ ## T ## ;

    #define JS_MAPPEDDYNAMICOBJECT_DECLARE_STATICS( T, Base ) \
      utf8String DynamicObject<T, Base, MappedDynamicObjectRegistry<T, Base>>::className( #T ); \
      WrappedType DynamicObject<T, Base, MappedDynamicObjectRegistry<T, Base>>::internalType = Wrapped_ ## T ## ;

    #define JS_MAPPEDDYNAMICOBJECT_DECLARE_STATICS_NAMED( T, Base, CName ) \
      utf8String DynamicObject<T, Base, MappedDynamicObjectRegistry<T, Base>>::className( #CName ); \
      WrappedType DynamicObject<T, Base, MappedDynamicObjectRegistry<T, Base>>::internalType = Wrapped_ ## T ## ;

    #define JS_DYNAMICOBJECT_DECLARE_PROPERTY( Name ) \
      void js_get ## Name ## ( V8String prop, const PropertyCallbackInfo<v8::Value>& info ); \
      void js_set ## Name ## ( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info )

    #define JS_DYNAMICOBJECT_DECLARE_READONLYPROPERTY( Name ) \
      void js_get ## Name ## ( V8String prop, const PropertyCallbackInfo<v8::Value>& info );

    #define JS_DYNAMICOBJECT_PROPERTYGETTER_BEGIN( T, Property ) \
      void T##::js_get ## Property ## ( V8String prop, const PropertyCallbackInfo<v8::Value>& info ) { \
        if ( prop->IsSymbol() ) \
          return; \
        const utf8String funcName( #T "::js_get" #Property ); \
        auto isolate = info.GetIsolate(); \
        HandleScope handleScope( isolate ); \
        auto context = isolate->GetCurrentContext(); \
        auto scriptCtx = scriptContext( isolate ); \
        v8::Local<v8::Value> ret;
    
    #define JS_DYNAMICOBJECT_PROPERTYGETTER_END() \
        info.GetReturnValue().Set( ret ); \
      }
    
    #define JS_DYNAMICOBJECT_PROPERTYSETTER_BEGIN( T, Property ) \
      void T##::js_set ## Property ## ( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info ) { \
        if ( prop->IsSymbol() ) \
          return; \
        const utf8String funcName( #T "::js_set" #Property ); \
        auto isolate = info.GetIsolate(); \
        HandleScope handleScope( isolate ); \
        auto context = isolate->GetCurrentContext(); \
        auto scriptCtx = scriptContext( isolate );
    
    #define JS_DYNAMICOBJECT_PROPERTYSETTER_END() \
      }
    
    #define JS_DYNAMICOBJECT_MEMBERFUNCTION_BEGIN( T, Method ) \
      void T##::js_##Method( const V8CallbackArgs& args ) { \
        const utf8String funcName( #T "::" #Method ); \
        auto isolate = args.GetIsolate(); \
        HandleScope handleScope( isolate ); \
        auto context = isolate->GetCurrentContext(); \
        auto scriptCtx = scriptContext( isolate ); \
        v8::Local<v8::Value> ret;
    
    #define JS_DYNAMICOBJECT_MEMBERFUNCTION_END() \
        args.GetReturnValue().Set( ret ); \
      }

    #define JS_DYNAMICOBJECT_CONSTRUCTBODY_BEGIN( T )       \
      const utf8String funcName( #T "::Constructor" );       \
      auto isolate = args.GetIsolate();                      \
      HandleScope handleScope( isolate );                    \
      auto context = args.GetIsolate()->GetCurrentContext(); \
      auto scriptCtx = scriptContext( isolate );

    #define JS_DYNAMICOBJECT_CONSTRUCTBODY_DO( T, Source )                          \
      if ( args.IsConstructCall() )                                                 \
      {                                                                             \
        auto thisObj = args.This();                                                 \
        auto ptr = scriptCtx->registry<T>( type<T> {} )->createFromJS( thisObj, Source ); \
        args.GetReturnValue().Set( ptr->handle( isolate ) );                        \
      }                                                                             \
      else                                                                          \
      {                                                                             \
        auto ptr = scriptCtx->registry<T>( type<T> {} )->createFrom( Source );            \
        args.GetReturnValue().Set( ptr->handle( isolate ) );                        \
      }
    
    #define JS_DYNAMICOBJECT_DESTRUCTBODY_DO( T, LocalPtr ) \
      if ( isolate && !deleted_ ) \
      { \
        auto ctx = scriptContext( isolate ); \
        if ( ctx ) \
          ctx->registry<T>( type<T>{} )->destroy( LocalPtr ); \
      } \
      DynamicObject<T, BaseType>::jsOnDestruct( isolate );

    #define JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( cls, propname, varname )                  \
      void cls::js_get##propname( V8String prop, const PropertyCallbackInfo<v8::Value>& info )               \
      {                                                                                                      \
       info.GetReturnValue().Set( varname->handle( info.GetIsolate() ) );                                    \
      }                                                                                                      \
      void cls::js_set##propname( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info ) \
      {                                                                                                      \
       auto isolate = info.GetIsolate();                                                                     \
       auto context = isolate->GetCurrentContext();                                                          \
       WrappedType argWrapType = Max_WrappedType;                                                            \
       if ( !util::getWrappedType( context, value, argWrapType ) || argWrapType != Wrapped_Vector3 )         \
       {                                                                                                     \
        isolate->ThrowException( util::staticStr( isolate, "Passed argument is not a vec3" ) );              \
        return;                                                                                              \
       }                                                                                                     \
       auto object = value->ToObject( context ).ToLocalChecked();                                            \
       varname = Vector3::unwrap( object )->shared_from_this();                                              \
      }

    #define JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS_WITH_CALLBACKS( cls, propname, varname )   \
      void cls::js_get##propname( V8String prop, const PropertyCallbackInfo<v8::Value>& info )               \
      {                                                                                                      \
       info.GetReturnValue().Set( varname->handle( info.GetIsolate() ) );                                    \
       js_afterGet##propname( info );                                                                        \
      }                                                                                                      \
      void cls::js_set##propname( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info ) \
      {                                                                                                      \
       auto isolate = info.GetIsolate();                                                                     \
       auto context = isolate->GetCurrentContext();                                                          \
       WrappedType argWrapType = Max_WrappedType;                                                            \
       if ( !util::getWrappedType( context, value, argWrapType ) || argWrapType != Wrapped_Vector3 )         \
       {                                                                                                     \
        isolate->ThrowException( util::staticStr( isolate, "Passed argument is not a vec3" ) );              \
        return;                                                                                              \
       }                                                                                                     \
       auto object = value->ToObject( context ).ToLocalChecked();                                            \
       varname = Vector3::unwrap( object )->shared_from_this();                                              \
       js_afterSet##propname( info );                                                                        \
      }

    #define JS_DYNAMICOBJECT_QUATERNION_PROPERTY_GETSET_IMPLEMENTATIONS( cls, propname, varname )              \
      void cls::js_get##propname( V8String prop, const PropertyCallbackInfo<v8::Value>& info )                  \
      {                                                                                                         \
       info.GetReturnValue().Set( varname->handle( info.GetIsolate() ) );                                       \
      }                                                                                                         \
      void cls::js_set##propname( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info ) \
      {                                                                                                         \
       auto isolate = info.GetIsolate();                                                                        \
       auto context = isolate->GetCurrentContext();                                                             \
       WrappedType argWrapType = Max_WrappedType;                                                               \
       if ( !util::getWrappedType( context, value, argWrapType ) || argWrapType != Wrapped_Quaternion )         \
       {                                                                                                        \
        isolate->ThrowException( util::staticStr( isolate, "Passed argument is not a quaternion" ) );           \
        return;                                                                                                 \
       }                                                                                                        \
       auto object = value->ToObject( context ).ToLocalChecked();                                               \
       varname = Quaternion::unwrap( object )->shared_from_this();                                       \
      }

    template <typename T>
    T::PtrType unwrapDynamic( V8Context& context, V8Value value, bool shouldThrow = true )
    {
      if ( value.IsEmpty() )
      {
        if ( shouldThrow )
          util::utf8Literal( args.GetIsolate(), "Argument is empty" );
        return {};
      }
      auto object = value->ToObject( context ).ToLocalChecked();
      if ( !util::isWrappedType( context, object, T::internalType ) )
      {
        if ( shouldThrow )
          util::utf8Literal( args.GetIsolate(), "Wrong internal type" );
        return {};
      }
      return T::unwrap( object )->ptr();
    }

    template <class T, class Y, class R = DynamicObjectRegistry<T, Y>>
    class DynamicObject: public ShareableBase<T>, public DirtyableObject {
      friend class neko::Scripting;
      template <class T>
      friend class DynamicObjectConstructor;
    public:
      using BaseType = Y;
      using PtrType = shared_ptr<T>;
      using RegistryType = R;
      using RegistryPtrType = shared_ptr<RegistryType>;
      friend RegistryType;
    public:
      static utf8String className;
      static WrappedType internalType;
    protected:
      Persistent<Object> persistent_; // Handle to myself
      int refs_; // Ref counter
      bool deleted_ = false; // This is true if the shared_ptr that we give out via ptr() no longer exists (i.e. DO NOT call ptr())
      inline void onPtrDelete() { deleted_ = true; }
      virtual int32_t jsEstimateSize() const = 0;
      virtual void jsOnDestruct( Isolate* isolate ) { delete this; }
    private:
      static void weakCallback( const WeakCallbackInfo<DynamicObject>& data )
      {
        auto wrap = dynamic_cast<T*>( data.GetParameter() );
        if ( !wrap )
          return;
        assert( wrap->refs_ == 0 );
        wrap->persistent_.Reset();
        data.GetIsolate()->AdjustAmountOfExternalAllocatedMemory( -( wrap->jsEstimateSize() ) );
        wrap->jsOnDestruct( data.GetIsolate() );
      }
    protected:
      inline void makeWeak() { persistent_.SetWeak( this, weakCallback, WeakCallbackType::kParameter ); }
    protected:
      inline void wrapperWrap( Handle<Object>& handle )
      {
        assert( persistent_.IsEmpty() );
        assert( handle->InternalFieldCount() == Max_WrapField );
        handle->SetInternalField( WrapField_Type, v8::Uint32::New( handle->GetIsolate(), internalType ) );
        handle->SetAlignedPointerInInternalField( WrapField_Pointer, this );
        persistent_.Reset( Isolate::GetCurrent(), handle );
        handle->GetIsolate()->AdjustAmountOfExternalAllocatedMemory( jsEstimateSize() );
        makeWeak();
      }
      static inline V8FunctionTemplate wrapperImplConstructor( Isolate* isolate )
      {
        EscapableHandleScope scope( isolate );

        auto tpl = FunctionTemplate::New( isolate, T::jsConstructor );
        tpl->SetClassName( util::allocString( className, isolate ) );
        tpl->InstanceTemplate()->SetInternalFieldCount( Max_WrapField );

        T::registerExport( isolate, tpl );

        return scope.Escape( tpl );
      }
    public:
      DynamicObject(): refs_( 0 )
      {
      }

      virtual void ref()
      {
        assert( !persistent_.IsEmpty() );
        persistent_.ClearWeak();
        refs_++;
      }

      virtual void unref()
      {
        assert( !persistent_.IsEmpty() );
        assert( !persistent_.IsWeak() );
        assert( refs_ > 0 );
        if ( --refs_ == 0 )
          makeWeak();
      }

      void reset()
      {
        if ( persistent_.IsEmpty() )
          return;
        OutputDebugStringA( __FUNCTION__ "\r\n" );
        persistent_.ClearWeak();
        persistent_.Reset();
      }

      virtual ~DynamicObject()
      {
        // OutputDebugStringA( __FUNCTION__ "\r\n" );
        // reset();
      }

      inline V8Object handle() { return handle( Isolate::GetCurrent() ); }
      inline V8Object handle( Isolate* isolate ) { return V8Object::New( isolate, persistent_ ); }
      inline Persistent<Object>& persistent() { return persistent_; }

      //! Unwraps the given handle.
      static inline T* unwrap( V8Object handle )
      {
        assert( !handle.IsEmpty() );
        assert( handle->InternalFieldCount() >= Max_WrapField );
        auto ptr = handle->GetAlignedPointerFromInternalField( WrapField_Pointer );
        return static_cast<T*>( ptr );
      }
    };

  }

}