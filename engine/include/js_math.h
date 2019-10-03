#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"

namespace neko {

  namespace js {

    class Vector2: public DynamicObjectWrapper<Vector2, vec2> {
    private:
      vec2 v_; //!< Internal vec2.
    protected:
      //! Properties
      void js_getX( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setX( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info );
      void js_getY( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setY( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info );
      //! Functions
      void js_toString( const V8CallbackArgs& args );
      void js_length( const V8CallbackArgs& args );
      void js_distance( const V8CallbackArgs& args );
      void js_dotProduct( const V8CallbackArgs& args );
      void js_normalise( const V8CallbackArgs& args );
      void js_normalisedCopy( const V8CallbackArgs& args );
      void js_midPoint( const V8CallbackArgs& args );
      void js_crossProduct( const V8CallbackArgs& args );
      void js_angleBetween( const V8CallbackArgs& args );
      void js_angleTo( const V8CallbackArgs& args );
      void js_perpendicular( const V8CallbackArgs& args );
      void js_reflect( const V8CallbackArgs& args );
      void js_makeFloor( const V8CallbackArgs& args );
      void js_makeCeil( const V8CallbackArgs& args );
    public:
      static void jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& info );
      static void registerExport( Isolate* isolate, v8::Local<v8::FunctionTemplate>& obj );
    public:
      Vector2( const vec2& source ): v_( source ) {}
      Vector2(): v_( 0.0f, 0.0f ) {}
      inline void setFrom( const vec2& other )
      {
        v_.x = other.x;
        v_.y = other.y;
      }
      inline vec2& v() { return v_; } //!< Get the internal vec2.
    };

    using Vector2Ptr = shared_ptr<Vector2>;

    //! Expect and extract a Vector2 object as args[arg],
    //! throw JS exception and return null on failure.
    Vector2Ptr extractVector2( int arg, const V8CallbackArgs& args );

  }

}