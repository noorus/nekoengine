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

  }

}