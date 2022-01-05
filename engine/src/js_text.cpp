#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"
#include "js_math.h"
#include "js_mathutil.h"
#include "js_text.h"
#include "locator.h"

namespace neko {

  JSText::JSText()
  {
    static size_t indexCounter = 0;
    id_ = indexCounter++;
  }

  namespace js {

    static const char* c_className = "text";

    string Text::className( c_className );
    WrappedType Text::internalType = Wrapped_Text;

    void Text::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      //JS_WRAPPER_SETACCESSOR( tpl, Mesh, x, X );
      //JS_WRAPPER_SETACCESSOR( tpl, Mesh, y, Y );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Text, toString );
    }

    void Text::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      JSText text;

      WrappedType wrappedType = Max_WrappedType;
      if ( args.Length() > 0 )
      {
        if ( args[0]->IsString() )
        {
          v8::String::Value str( isolate, args[0] );
          if ( *str )
            text.content_ = unicodeString( *str );
        }
      }

      auto ctx = getScriptContext( isolate );

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->textreg().createFromJS( thisObj, text );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->textreg().createFrom( text );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    void Text::js_toString( const V8CallbackArgs& args )
    {
      utf8String content;
      local_.content_.toUTF8String( content );
      char result[1024];
      sprintf_s<1024>( result, "text[%zi,\"%s\"]", local_.id_, content.c_str() );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

    int32_t Text::jsEstimateSize() const
    {
      int64_t result = sizeof( Text );
      result += local_.content_.length() * sizeof( uint32_t );
      // add in the size of the mesh and whatever in bytes maybe
      return static_cast<int32_t>( std::min( static_cast<int64_t>( std::numeric_limits<int32_t>::max() ), result ) );
    }

    void Text::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
      if ( ctx )
        ctx->renderSync().destructed( this );
    }

  }

}

#endif