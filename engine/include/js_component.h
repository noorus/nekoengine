#pragma once
#ifndef NEKO_NO_SCRIPTING

# include "neko_types.h"
# include "neko_exception.h"
# include "console.h"
# include "mesh_primitives.h"
# include "js_dynamicobject.h"
# include "js_dynamicregistry.h"
# include "js_math.h"
# include "js_mesh.h"
# include "components.h"
# include "locator.h"

namespace neko {

  namespace js {

    enum ComponentType
    {
      Component_Transform = 0,
      Component_Camera
    };

    struct JSComponent
    {
    public:
      neko::c::entity eid;
      ComponentType ctype;
      js::Vector3Ptr scale;
      js::Vector3Ptr translate;
      js::QuaternionPtr rotate;
      JSComponent( uint32_t id ): eid( static_cast<neko::c::entity>( id ) ) {}
    };

    class TransformComponent: public DynamicObject<TransformComponent, JSComponent, MappedDynamicObjectRegistry<TransformComponent, JSComponent>> {
    private:
      JSComponent local_;
    protected:
      void js_getScale( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setScale( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getTranslate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setTranslate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getRotate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setRotate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      inline void js_afterGetScale( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetScale( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetTranslate( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetTranslate( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetRotate( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetRotate( const PropertyCallbackInfo<void>& info );
    public:
      static void jsConstructor( const V8CallbackArgs& info );
      virtual int32_t jsEstimateSize() const;
      virtual void jsOnDestruct( Isolate* isolate );
      void js_toString( const V8CallbackArgs& args );
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
    public:
      TransformComponent( const JSComponent& source ): local_( source ) {}
      inline void setFrom( const JSComponent& other )
      {
        Locator::console().printf(
          neko::srcScripting, "js::Component(0x%I64X) setFrom JSComponent(0x%I64X)", this, reinterpret_cast<const void*>( &other ) );
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
      inline JSComponent& ent() { return local_; }
      inline operator JSComponent() const { return local_; }
    };

    using ComponentPtr = shared_ptr<TransformComponent>;
    using ComponentVector = vector<TransformComponent*>;
    using ComponentPtrVector = vector<ComponentPtr>;

  }

}

#endif