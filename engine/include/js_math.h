#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"

namespace neko {

  namespace js {

    class Math;
    using JSMathPtr = unique_ptr<Math>;

    class Math: public StaticObjectWrapper<Math> {
    private:
      void registerGlobals( Isolate* isolate, V8FunctionTemplate& tpl ) final;
    public:
      explicit Math();
    public:
      //! JavaScript Math.equals
      void js_equals( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Math.greater
      void js_greater( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Math.lesser
      void js_lesser( Isolate* isolate, const V8CallbackArgs& args );
    public:
      static JSMathPtr create( Isolate* isolate, V8Object global );
    };

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
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
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

    template <class T>
    inline shared_ptr<T> extractWrappedDynamic( V8Context& context, V8Value& value )
    {
      auto object = value->ToObject( context ).ToLocalChecked();
      return T::unwrap( object )->shared_from_this();
    }

  }

}