#pragma once
#ifndef NEKO_NO_SCRIPTING

# include "neko_types.h"
# include "neko_exception.h"
# include "console.h"
# include "mesh_primitives.h"
# include "js_wrapper.h"
# include "js_math.h"
# include "js_mesh.h"
# include "components.h"
# include "locator.h"

namespace neko {

  namespace js {

    struct JSEntity {
    public:
      neko::c::entity id;
      js::Vector3Ptr scale;
      js::Vector3Ptr translate;
      js::QuaternionPtr rotate;
    };

    class Entity: public DynamicObjectWrapper<Entity, JSEntity> {
    private:
      JSEntity local_;
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
      Entity( const JSEntity& source ): local_( source ) {}
      inline void setFrom( const JSEntity& other )
      {
        Locator::console().printf(
          neko::srcScripting, "js::Model(0x%I64X) setFrom JSModel(0x%I64X)", this, reinterpret_cast<const void*>( &other ) );
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
      inline JSEntity& ent() { return local_; }
      inline operator JSEntity() const { return local_; }
    };

    using EntityPtr = shared_ptr<Entity>;
    using EntityVector = vector<Entity*>;
    using EntityPtrVector = vector<EntityPtr>;

  }

}

#endif