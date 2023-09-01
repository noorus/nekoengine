#pragma once

#include "neko_types.h"
#include "neko_exception.h"

#include "js_util.h"
#include "js_types.h"

namespace neko {

  namespace js {

    template <class T, class Y>
    class DynamicObjectRegistry {
    public:
      using PtrType = shared_ptr<T>; // ex. shared_ptr<js::Vector2>
    protected:
      vector<PtrType> pool_;
      //! My JS constructor store
      Eternal<FunctionTemplate> constructor;
    public:
      template <class... Args>
      inline PtrType createFromJS( Handle<Object>& handle, Args... args )
      {
        auto obj = new T( args... );
        auto ptr = PtrType( obj, [=]( T* baleet ) {
          // This is the shared_ptr deleter, but actually we just remove our
          // previously-added reference. The actual deletion is left to V8.
          baleet->unref();
          baleet->onPtrDelete();
        } );
        ptr->wrapperWrap( handle );
        ptr->ref();
        pool_.push_back( ptr );
        return ptr;
      }
      // createFrom doesn't actually magically create the instance based on a non-JS class.
      // rather, it runs the default JS constructor for an empty object, and then
      // uses setFrom to replace its internal contents with the passed [source].
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
      inline PtrType create()
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
        return ptr;
      }
      inline void destroy( PtrType obj )
      {
        //
      }
      void initialize( Isolate* isolate, Local<ObjectTemplate>& exports )
      {
        auto tpl = T::wrapperImplConstructor( isolate );
        exports->Set( util::allocStringConserve( T::className, isolate ), tpl );
        constructor.Set( isolate, tpl );
      }
      inline void clear() { pool_.clear(); }
    };

    template <class T, class Y>
    class MappedDynamicObjectRegistry {
    public:
      using IDType = uint64_t;
      using PtrType = shared_ptr<T>; // ex. shared_ptr<js::Vector2>
      using MapType = map<IDType, PtrType>;
    protected:
      MapType map_;
      //! My JS constructor store
      Eternal<FunctionTemplate> constructor;
    public:
      template <class... Args>
      inline PtrType createFromJS( Handle<Object>& handle, IDType id, Args... args )
      {
        if ( map_.contains( id ) )
          return map_[id];

        auto obj = new T( args... );
        auto ptr = PtrType( obj, [=]( T* baleet )
        {
          // This is the shared_ptr deleter, but actually we just remove our
          // previously-added reference. The actual deletion is left to V8.
          baleet->unref();
          baleet->onPtrDelete();
        } );
        ptr->wrapperWrap( handle );
        ptr->ref();
        map_[id] = ptr;
        return ptr;
      }
      // createFrom doesn't actually magically create the instance based on a non-JS class.
      // rather, it runs the default JS constructor for an empty object, and then
      // uses setFrom to replace its internal contents with the passed [source].
      inline PtrType createFrom( IDType id, const Y& source )
      {
        if ( map_.contains( id ) )
          return map_[id];

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
      inline PtrType create( IDType id )
      {
        if ( map_.contains( id ) )
          return map_[id];

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
        return ptr;
      }
      inline void destroy( PtrType obj )
      {
        //
      }
      void initialize( Isolate* isolate, Local<ObjectTemplate>& exports )
      {
        auto tpl = T::wrapperImplConstructor( isolate );
        exports->Set( util::allocStringConserve( T::className, isolate ), tpl );
        constructor.Set( isolate, tpl );
      }
      inline void clear() { map_.clear(); }
      inline MapType& items() noexcept { return map_; }
    };

  }

}