#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "mesh_primitives.h"
#include "js_wrapper.h"
#include "js_math.h"
#include "js_mesh.h"
#include "locator.h"

namespace neko {

  namespace js {

    class JSModel {
    public:
      size_t id_;
      MeshPtr mesh_;
      Vector3Ptr scale_;
      Vector3Ptr translate_;
      JSModel();
    };

    class Model : public DynamicObjectWrapper<Model, JSModel>
    {
    private:
      JSModel local_; //!< Internal model.

    protected:
      //! Properties
      void js_getScale( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setScale( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getTranslate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setTranslate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      //! Functions
      void js_toString( const V8CallbackArgs& args );

    public:
      static void jsConstructor( const V8CallbackArgs& info );
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );

    public:
      Model( const JSModel& source ): local_( source ) {}
      inline void setFrom( const JSModel& other )
      {
        Locator::console().printf( neko::Console::srcScripting, "js::Model(0x%I64X) setFrom JSModel(0x%I64X)", this, other );
        // TODO better; this is destructive
        //local_.vbo_->copy( *other.vbo_ );
        //local_.ebo_->copy( *other.ebo_ );
        //local_.vao_->reset();
      }
      /*inline void setFromDestructive( JSMesh& other )
      {
        local_.localVBO_ = other.localVBO_;
        local_.localEBO_ = other.localEBO_;
        local_.localVAO_ = other.localVAO_;
      }*/
      inline JSModel& model() { return local_; } //!< Get the internal JSModel.
      inline operator JSModel() const { return local_; }
    };

    using ModelPtr = shared_ptr<Model>;
    using ModelVector = vector<ModelPtr>;

  }

}