#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "mesh_primitives.h"
#include "js_wrapper.h"
#include "js_math.h"
#include "js_mesh.h"
#include "locator.h"
#include "camera.h"
/*
namespace neko {

  namespace js {

    class Camera : public DynamicObjectWrapper<Camera, neko::Camera> {
    private:
      JSModel local_; //!< Internal model.
    protected:
      //! Properties
      void js_getScale( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setScale( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getTranslate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setTranslate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getRotate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setRotate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      //! Functions
      void js_toString( const V8CallbackArgs& args );

    public:
      static void jsConstructor( const V8CallbackArgs& info );
      virtual int32_t jsEstimateSize() const;
      virtual void jsOnDestruct( Isolate* isolate );
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );

    public:
      Model( const JSModel& source )
          : local_( source ) {}
      inline void setFrom( const JSModel& other )
      {
        Locator::console().printf( neko::srcScripting, "js::Model(0x%I64X) setFrom JSModel(0x%I64X)", this, other );
        // TODO better; this is destructive
        //local_.vbo_->copy( *other.vbo_ );
        //local_.ebo_->copy( *other.ebo_ );
        //local_.vao_->reset();
      }
      inline neko::Camera& camera() { return local_; } //!< Get the internal JSModel.
      inline operator neko::Camera() const { return local_; }
    };

    using CameraPtr = shared_ptr<Camera>;
    using CameraVector = vector<Camera*>;
    using CameraPtrVector = vector<CameraPtr>;

  }

}*/

#endif