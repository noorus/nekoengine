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
      //! JavaScript Math.greaterOrEqual
      void js_greaterOrEqual( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Math.lesser
      void js_lesser( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Math.lesserOrEqual
      void js_lesserOrEqual( Isolate* isolate, const V8CallbackArgs& args );
    public:
      static JSMathPtr create( Isolate* isolate, V8Object global );
    };

    //! \class Vector2
    //! \brief Implementation of JavaScript vec2().
    class Vector2 : public DynamicObjectWrapper<Vector2, neko::vec2>
    {
    private:
      vec2 v_; //!< Internal vec2.
    protected:
      //! Properties
      void js_getX( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setX( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getY( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setY( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      //! Comparisons
      void js_equals( const V8CallbackArgs& args );
      void js_greater( const V8CallbackArgs& args );
      void js_greaterOrEqual( const V8CallbackArgs& args );
      void js_lesser( const V8CallbackArgs& args );
      void js_lesserOrEqual( const V8CallbackArgs& args );
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
      static void jsConstructor( const V8CallbackArgs& info );
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
      inline operator vec2() const { return v_; }
    };

    using Vector2Ptr = shared_ptr<Vector2>;

    //! \class Vector3
    //! \brief Implementation of JavaScript vec3().
    class Vector3 : public DynamicObjectWrapper<Vector3, neko::vec3>
    {
    private:
      vec3 v_; //!< Internal vec3.
    protected:
      //! Properties
      void js_getX( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setX( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getY( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setY( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getZ( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setZ( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      //! Comparisons
      void js_equals( const V8CallbackArgs& args );
      void js_greater( const V8CallbackArgs& args );
      void js_greaterOrEqual( const V8CallbackArgs& args );
      void js_lesser( const V8CallbackArgs& args );
      void js_lesserOrEqual( const V8CallbackArgs& args );
      //! Functions
      void js_toString( const V8CallbackArgs& args );
      void js_length( const V8CallbackArgs& args );
      void js_distance( const V8CallbackArgs& args );
      void js_normalise( const V8CallbackArgs& args );
      void js_normalisedCopy( const V8CallbackArgs& args );
      void js_midPoint( const V8CallbackArgs& args );
      void js_angleBetween( const V8CallbackArgs& args );
      void js_makeFloor( const V8CallbackArgs& args );
      void js_makeCeil( const V8CallbackArgs& args );
      void js_add( const V8CallbackArgs& args );

    public:
      static void jsConstructor( const V8CallbackArgs& info );
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );

    public:
      Vector3( const vec3& source ): v_( source ) {}
      Vector3(): v_( 0.0f, 0.0f, 0.0f ) {}
      inline void setFrom( const vec3& other )
      {
        v_.x = other.x;
        v_.y = other.y;
        v_.z = other.z;
      }
      inline vec3& v() { return v_; } //!< Get the internal vec3.
      inline operator vec3() const { return v_; }
    };

    using Vector3Ptr = shared_ptr<Vector3>;

    //! Expect and extract a Vector2 object as args[arg],
    //! throw JS exception and return null on failure.
    Vector2Ptr extractVector2( int arg, const V8CallbackArgs& args );

    //! Expect and extract a Vector3 object as args[arg],
    //! throw JS exception and return null on failure.
    Vector3Ptr extractVector3( int arg, const V8CallbackArgs& args );

    Vector3Ptr extractVector3Member( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow );

    template <class T>
    inline shared_ptr<T> extractWrappedDynamic( V8Context& context, const V8Value& value )
    {
      auto object = value->ToObject( context ).ToLocalChecked();
      return T::unwrap( object )->shared_from_this();
    }

  }

}