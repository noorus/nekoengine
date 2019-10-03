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
      char error[64];
      sprintf_s<64>( error, "Expected object vec2 as argument %d", arg );
      args.GetIsolate()->ThrowException( util::allocString( error, args.GetIsolate() ) );
      return Vector2Ptr();
    }

  }

}