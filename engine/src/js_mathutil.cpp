#include "stdafx.h"
#include "js_math.h"
#include "js_util.h"
#include "console.h"
#include "scripting.h"

namespace neko {

  namespace js {

    Vector2Ptr extractVector2( int arg, const V8CallbackArgs& args )
    {
      if ( args.Length() >= ( arg + 1 ) && args[arg]->IsObject() )
      {
        auto context = args.GetIsolate()->GetCurrentContext();
        auto object = args[arg]->ToObject( context ).ToLocalChecked();
        if ( util::isWrappedType( context, object, Wrapped_Vector2 ) )
        {
          return Vector2::unwrap( object )->shared_from_this();
        }
      }
      util::throwException( args.GetIsolate(), L"Expected object vec2 as argument %d", arg );
      return Vector2Ptr();
    }

    Vector3Ptr extractVector3( int arg, const V8CallbackArgs& args )
    {
      if ( args.Length() >= ( arg + 1 ) && args[arg]->IsObject() )
      {
        auto context = args.GetIsolate()->GetCurrentContext();
        auto object = args[arg]->ToObject( context ).ToLocalChecked();
        if ( util::isWrappedType( context, object, Wrapped_Vector3 ) )
        {
          return Vector3::unwrap( object )->shared_from_this();
        }
      }
      util::throwException( args.GetIsolate(), L"Expected object vec3 as argument %d", arg );
      return Vector3Ptr();
    }

    Vector3Ptr extractVector3Member( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow )
    {
      const Vector3Ptr emptyPtr;
      if ( maybeObject.IsEmpty() )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no vec3 member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      auto object = maybeObject.ToLocalChecked()->Get( util::allocStringConserve( name, isolate ) );
      if ( object.IsEmpty() || !object->IsObject() )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no vec3 member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      auto local = v8::Local<v8::Object>::Cast( object );
      if ( !util::isWrappedType( isolate->GetCurrentContext(), local, Wrapped_Vector3 ) )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no vec3 member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      return move( Vector3::unwrap( local )->shared_from_this() );
    }

    QuaternionPtr extractQuaternion( int arg, const V8CallbackArgs& args )
    {
      if ( args.Length() >= ( arg + 1 ) && args[arg]->IsObject() )
      {
        auto context = args.GetIsolate()->GetCurrentContext();
        auto object = args[arg]->ToObject( context ).ToLocalChecked();
        if ( util::isWrappedType( context, object, Wrapped_Quaternion ) )
        {
          return Quaternion::unwrap( object )->shared_from_this();
        }
      }
      util::throwException( args.GetIsolate(), L"Expected object quaternion as argument %d", arg );
      return QuaternionPtr();
    }

    QuaternionPtr extractQuaternionMember( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow )
    {
      const QuaternionPtr emptyPtr;
      if ( maybeObject.IsEmpty() )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no quaternion member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      auto object = maybeObject.ToLocalChecked()->Get( util::allocStringConserve( name, isolate ) );
      if ( object.IsEmpty() || !object->IsObject() )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no quaternion member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      auto local = v8::Local<v8::Object>::Cast( object );
      if ( !util::isWrappedType( isolate->GetCurrentContext(), local, Wrapped_Quaternion ) )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no quaternion member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      return move( Quaternion::unwrap( local )->shared_from_this() );
    }

  }

}