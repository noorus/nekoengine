#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "forwards.h"
#include "js_util.h"

namespace neko {

  namespace js {

    enum WrappedType {
      Wrapped_Console, //!< Static: JSConsole
      Wrapped_Math, //!< Static: JSMath
      Wrapped_Game,
      Wrapped_Vector2, //!< Dynamic: Vector2
      Wrapped_Vector3, //!< Dynamic: Vector3
      Wrapped_Quaternion, //!< Dynamic: Quaternion
      Wrapped_Mesh, //!< Dynamic: DynamicMesh
      Wrapped_Model, //!< Dynamic: Model
      Max_WrappedType
    };

    enum WrappedFieldIndex {
      WrapField_Type,
      WrapField_Pointer,
      Max_WrapField
    };

    namespace util {

      inline bool isWrappedType( const V8Context& ctx, const V8Object& object, WrappedType type )
      {
        if ( object->InternalFieldCount() != Max_WrapField )
          return false;
        auto val = object->GetInternalField( WrapField_Type );
        if ( val.IsEmpty() || !val->IsUint32() )
          return false;
        return ( val->Uint32Value( ctx ).FromMaybe( Max_WrappedType ) == type );
      }

      inline bool getWrappedType( const V8Context& ctx, const V8Value& value, WrappedType& type_out )
      {
        if ( value.IsEmpty() || !value->IsObject() )
          return false;
        auto object = value->ToObject( ctx ).ToLocalChecked();
        if ( object->InternalFieldCount() != Max_WrapField )
          return false;
        auto val = object->GetInternalField( WrapField_Type );
        if ( val.IsEmpty() || !val->IsUint32() )
          return false;
        auto retval = static_cast<WrappedType>( val->Uint32Value( ctx ).FromMaybe( Max_WrappedType ) );
        if ( retval >= Max_WrappedType )
          return false;
        type_out = retval;
        return true;
      }

    }

    //! Use this to create member functions for static-wrapped objects (instances).
#   define JS_WRAPPER_SETOBJMEMBER(tpl,cls,x) tpl->PrototypeTemplate()->Set( \
      util::staticStr( isolate, #x ), \
      FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = static_cast<cls*>( args.Data().As<v8::External>()->Value() ); \
        self->js_##x( args.GetIsolate(), args ); \
      }, v8::External::New( isolate, (void*)this ) ) )

    //! Use this to create member functions for static-wrapped objects (instances).
#   define JS_WRAPPER_SETOBJMEMBERNAMED(tpl,cls,x,y) tpl->PrototypeTemplate()->Set( \
      util::staticStr( isolate, #y ), \
      FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = static_cast<cls*>( args.Data().As<v8::External>()->Value() ); \
        self->js_##x( args.GetIsolate(), args ); \
      }, v8::External::New( isolate, (void*)this ) ) )

    //! Use this to create accessors for variables in dynamic-wrapped objects' templates.
#   define JS_WRAPPER_SETACCESSOR(obj,cls,x,valInternal) obj->PrototypeTemplate()->SetAccessor( \
      util::staticStr( isolate, #x ), []( V8String prop, const PropertyCallbackInfo<v8::Value>& info ) { \
        auto self = info.This(); \
        if ( !util::isWrappedType( info.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
          return; \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_get##valInternal( prop, info ); \
      }, []( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info ) { \
        auto self = info.This(); \
        if ( !util::isWrappedType( info.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
          return; \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_set##valInternal( prop, value, info ); \
      } )

    //! Use this to create member functions for variables in dynamic-wrapped objects' templates.
#   define JS_WRAPPER_SETMEMBER(obj,cls,x) obj->PrototypeTemplate()->Set( \
      util::staticStr( isolate, #x ), \
      FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = args.This(); \
        if ( !util::isWrappedType( args.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
          return; \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_##x( args ); \
    } ) )

    //! Use this to create member functions for variables in dynamic-wrapped objects' templates.
#   define JS_WRAPPER_SETMEMBERNAMED(obj,cls,x,y) obj->PrototypeTemplate()->Set( \
      util::staticStr( isolate, #y ), \
      FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = args.This(); \
        if ( !util::isWrappedType( args.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
          return; \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_##x( args ); \
    } ) )

    // ex. js::DynamicObjectsRegistry<js::Vector2, vec2> vec2Registry_;

    template <class T, class Y>
    class DynamicObjectsRegistry {
    public:
      using PtrType = shared_ptr<T>; // ex. shared_ptr<js::Vector2>
    protected:
      vector<PtrType> pool_;
      //! My JS constructor store
      v8::Eternal<v8::FunctionTemplate> constructor;
    public:
      template <class... Args>
      inline PtrType createFromJS( v8::Handle<v8::Object>& handle, Args... args )
      {
        auto obj = new T( args... );
        auto ptr = PtrType( obj, [=]( T* baleet )
        {
          // This is the shared_ptr deleter, but actually we just remove our
          // previously-added reference. The actual deletion is left to V8.
          baleet->unref();
        });
        ptr->wrapperWrap( handle );
        pool_.push_back( ptr );
        return ptr;
      }
      inline PtrType createFrom( const Y& source )
      {
        auto isolate = Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();
        auto constFunc = constructor.Get( isolate )->GetFunction( context ).ToLocalChecked();
        assert( !constFunc.IsEmpty() );
        auto inst = constFunc->NewInstance( context );
        assert( !inst.IsEmpty() );
        V8Object object;
        inst.ToLocal( &object );
        auto unwr = T::unwrap( object );
        assert( unwr );
        // Don't worry, even unwr here has been created through createFromJS above.
        auto ptr = unwr->shared_from_this();
        assert( ptr );
        ptr->setFrom( source );
        return ptr;
      }
      void initialize( Isolate* isolate, v8::Local<v8::ObjectTemplate>& exports )
      {
        auto tpl = T::wrapperImplConstructor( isolate );
        exports->Set( util::allocStringConserve( T::className, isolate ), tpl );
        constructor.Set( isolate, tpl );
      }
      inline void clear()
      {
        pool_.clear();
      }
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

    template <class T, class Y>
    class DynamicObjectWrapper: public std::enable_shared_from_this<T> {
      friend class neko::Scripting;
      template <class T>
      friend class DynamicObjectConstructor;
      friend class DynamicObjectsRegistry<T, Y>;
      using RegistryPtrType = shared_ptr<DynamicObjectsRegistry<T, Y>>;
    protected:
      //! My JavaScript-exported class name
      static string className;
      //! My magic internal type
      static WrappedType internalType;
      //! Myself
      v8::Persistent<v8::Object> handle_;
      int refs_;
    private:
      static void weakCallback( const v8::WeakCallbackInfo<DynamicObjectWrapper>& data )
      {
        auto wrap = (T*)data.GetParameter();
        if ( !wrap )
          return;
        assert( wrap->refs_ == 0 );
        wrap->handle_.Reset();
        delete wrap;
      }
    protected:
      inline void makeWeak()
      {
        persistent().SetWeak( this, weakCallback, v8::WeakCallbackType::kParameter );
      }
    protected:
      inline void wrapperWrap( v8::Handle<v8::Object>& handle )
      {
        assert( persistent().IsEmpty() );
        assert( handle->InternalFieldCount() == Max_WrapField );
        handle->SetInternalField( WrapField_Type, v8::Uint32::New( handle->GetIsolate(), internalType ) );
        handle->SetAlignedPointerInInternalField( WrapField_Pointer, this );
        persistent().Reset( v8::Isolate::GetCurrent(), handle );
        makeWeak();
        handle->GetIsolate()->AdjustAmountOfExternalAllocatedMemory( sizeof( T ) );
        // this is a clutch, but as long as wrapperWrap is called ONLY ONCE
        // when creating a new object, we can safely add that one refcount
        // here, which will be later subtracted by the shared_ptr deleter.
        ref();
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
    public:
      DynamicObjectWrapper(): refs_( 0 )
      {
      }
      virtual void ref()
      {
        assert( !persistent().IsEmpty() );
        persistent().ClearWeak();
        refs_++;
      }
      virtual void unref()
      {
        assert( !persistent().IsEmpty() );
        assert( !persistent().IsWeak() );
        assert( refs_ > 0 );
        if ( --refs_ == 0 )
          makeWeak();
      }
      void reset()
      {
        if ( persistent().IsEmpty() )
          return;
        persistent().ClearWeak();
        persistent().Reset();
      }
      virtual ~DynamicObjectWrapper()
      {
        reset();
      }
      inline V8Object handle()
      {
        return handle( v8::Isolate::GetCurrent() );
      }
      inline V8Object handle( v8::Isolate* isolate )
      {
        return V8Object::New( isolate, persistent() );
      }
      inline v8::Persistent<v8::Object>& persistent()
      {
        return handle_;
      }
      //! Unwraps the given handle.
      static inline T* unwrap( V8Object handle )
      {
        assert( !handle.IsEmpty() );
        assert( handle->InternalFieldCount() >= Max_WrapField );
        auto ptr = handle->GetAlignedPointerFromInternalField( WrapField_Pointer );
        auto wrapper = static_cast<DynamicObjectWrapper*>( ptr );
        return static_cast<T*>( wrapper );
      }
    };

  }

}