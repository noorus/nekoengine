#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include <v8.h>

namespace neko {

  namespace js {

    using v8::Isolate;
    using v8::HandleScope;
    using v8::Local;
    using v8::Persistent;
    using v8::Eternal;
    using v8::PropertyCallbackInfo;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;

    enum WrappedType {
      Wrapped_Console = 0
    };

    template <class T>
    class Wrapper {
    protected:
    protected:
      //! My JavaScript-exported constructor function template.
      static Eternal<FunctionTemplate> constructor;
      //! My JavaScript-exported class name
      static string className;
    private:
      Persistent<v8::Object> jsHandle_;
      WrappedType wrappedType_;
    protected:
      int jsReferences_;
      Isolate* isolate_;
      inline void wrap( Local<v8::Object> handle )
      {
        assert( persistent().IsEmpty() );
        assert( handle->InternalFieldCount() > 0 );
        handle->SetAlignedPointerInInternalField( 0, this );
        persistent().Reset( Isolate::GetCurrent(), handle );
        makeWeak();
      }
      inline void makeWeak()
      {
        persistent().SetWeak( this,
          []( const v8::WeakCallbackInfo<Wrapper>& data )
        {
          Wrapper* wrapper = data.GetParameter();
          assert( wrapper->jsReferences_ == 0 );
          wrapper->jsHandle_.Reset();
          delete wrapper;
        }, v8::WeakCallbackType::kParameter );
      }
      inline void clearWeak()
      {
        jsHandle_.ClearWeak();
      }
    public:
      explicit Wrapper( const WrappedType type ):
        jsReferences_( 0 ), wrappedType_( type )
      {
        isolate_ = Isolate::GetCurrent();
      }
      virtual ~Wrapper()
      {
        if ( !persistent().IsEmpty() )
        {
          persistent().ClearWeak();
          persistent().Reset();
        }
      }
      inline Local<v8::Object> handle()
      {
        return handle( Isolate::GetCurrent() );
      }
      inline Local<v8::Object> handle( Isolate* isolate )
      {
        return Local<v8::Object>::New( isolate, persistent() );
      }
      inline Persistent<v8::Object>& persistent()
      {
        return jsHandle_;
      }
      virtual void ref()
      {
        assert( !persistent().IsEmpty() );
        persistent().ClearWeak();
        jsReferences_++;
      }
      virtual void unref()
      {
        assert( !persistent().IsEmpty() );
        assert( !persistent().IsWeak() );
        assert( jsReferences_ > 0 );
        if ( --jsReferences_ == 0 )
          makeWeak();
      }
      inline const WrappedType getWrappedType()
      {
        return wrappedType_;
      }
      static inline bool isWrappedType( Local<v8::Object> handle )
      {
        assert( !handle.IsEmpty() );
        assert( handle->InternalFieldCount() > 0 );
        auto ptr = handle->GetAlignedPointerFromInternalField( 0 );
        auto wrapper = static_cast<Wrapper*>( ptr );
        return static_cast<T*>( wrapper );
      }
      static inline T* unwrap( Local<v8::Object> handle )
      {
        assert( !handle.IsEmpty() );
        assert( handle->InternalFieldCount() > 0 );
        auto ptr = handle->GetAlignedPointerFromInternalField( 0 );
        auto wrapper = static_cast<Wrapper*>( ptr );
        return static_cast<T*>( wrapper );
      }
    };

    template <class T> Eternal<v8::FunctionTemplate> Wrapper<T>::constructor;

  }

}