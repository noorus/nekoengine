#pragma once
#ifndef NEKO_NO_SCRIPTING

# include "neko_types.h"
# include "neko_exception.h"
# include "console.h"
# include "mesh_primitives.h"
# include "js_dynamicobject.h"
# include "js_dynamicregistry.h"
# include "js_math.h"
# include "components.h"
# include "locator.h"

namespace neko {

  namespace js {

    template <class T, class B>
    class MappedComponentBase: public DynamicObject<T, B, MappedDynamicObjectRegistry<T, B>> {
    protected:
      B local_;
    public:
      MappedComponentBase( const B& source ): local_( source ) {}
      inline B& ent() { return local_; }
      inline operator B() const { return local_; }
    };

    struct JSTransformComponent
    {
    public:
      c::entity eid;
      js::Vector3Ptr scale;
      js::Vector3Ptr translate;
      js::QuaternionPtr rotate;
      JSTransformComponent( uint32_t id ): eid( static_cast<c::entity>( id ) ) {}
    };

    class TransformComponent: public MappedComponentBase<TransformComponent, JSTransformComponent> {
      using BaseType = MappedComponentBase<TransformComponent, JSTransformComponent>;
    protected:
      //! Properties
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Scale )
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Translate )
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Rotate )
      inline void js_afterGetScale( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetScale( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetTranslate( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetTranslate( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetRotate( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetRotate( const PropertyCallbackInfo<void>& info );
      //! Functions
      void js_toString( const V8CallbackArgs& args );
    public:
      static void jsConstructor( const V8CallbackArgs& info );
      int32_t jsEstimateSize() const override;
      void jsOnDestruct( Isolate* isolate ) override;
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
    public:
      TransformComponent( const JSTransformComponent& source ): BaseType( source ) {}
      inline void setFrom( const JSTransformComponent& other )
      {
        local_ = other;
      }
    };

    struct JSCameraComponent
    {
    public:
      c::entity eid = static_cast<c::entity>( 0xFFFFFFFF );
      bool isOrtho = false;
      Real perspectiveFovy = 30.0f;
      Real orthographicRadius = 16.0f;
      Real nearDist = 0.1f;
      Real farDist = 200.0f;
      Real exposure = 1.0f;
      JSCameraComponent( uint32_t id ): eid( static_cast<c::entity>( id ) ) {}
    };

    class CameraComponent: public MappedComponentBase<CameraComponent, JSCameraComponent> {
      using BaseType = MappedComponentBase<CameraComponent, JSCameraComponent>;
    protected:
      //! Properties
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Projection )
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Near )
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Far )
      JS_DYNAMICOBJECT_DECLAREACCESSORS( Exposure )
      inline void js_afterGetProjection( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetProjection( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetNear( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetNear( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetFar( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetFar( const PropertyCallbackInfo<void>& info );
      inline void js_afterGetExposure( const PropertyCallbackInfo<v8::Value>& info ) {}
      void js_afterSetExposure( const PropertyCallbackInfo<void>& info );
      //! Functions
      void js_toString( const V8CallbackArgs& args );
    public:
      static void jsConstructor( const V8CallbackArgs& info );
      int32_t jsEstimateSize() const override;
      void jsOnDestruct( Isolate* isolate ) override;
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
    public:
      CameraComponent( const JSCameraComponent& source ): BaseType( source ) {}
      inline void setFrom( const JSCameraComponent& other ) { local_ = other; }
    };

  }

}

#endif