#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_util.h"
# include "console.h"
# include "scripting.h"
# include "nekomath.h"
# include "js_math.h"
# include "js_mathutil.h"
# include "js_text.h"
# include "locator.h"
# include "console.h"

namespace neko {

  JSText::JSText()
  {
    static size_t indexCounter = 0;
    id_ = indexCounter++;
  }

  namespace js {

    JS_DYNAMICOBJECT_DECLARE_STATICS( Text, neko::JSText )

    void Text::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Text, scale, Scale );
      JS_WRAPPER_SETACCESSOR( tpl, Text, translate, Translate );
      JS_WRAPPER_SETACCESSOR( tpl, Text, rotate, Rotate );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Text, toString );
    }

    void Text::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      const utf8String funcName( "text::Constructor" );

      vec3 defaultScale( 1.0f, 1.0f, 1.0f );
      vec3 defaultTranslate( 0.0f, 0.0f, 0.0f );
      quaternion defaultRotate( glm::quat_identity<Real, glm::defaultp>() );

      JSText text;

      WrappedType wrappedType = Max_WrappedType;
      if ( args.Length() == 1 )
      {
        util::getWrappedType( context, args[0], wrappedType );
        if ( args[0]->IsObject() && wrappedType == Max_WrappedType )
        {
          auto maybeObj = args[0]->ToObject( context );
          text.scale_ = extractVector3Member( isolate, funcName, maybeObj, "scale", false );
          text.translate_ = extractVector3Member( isolate, funcName, maybeObj, "translate", false );
          text.rotate_ = extractQuaternionMember( isolate, funcName, maybeObj, "rotate", false );
          text.content_ = util::extractStringMemberUnicode( isolate, funcName, maybeObj, "str", false );
        }
        else if ( args[0]->IsString() )
        {
          v8::String::Value str( isolate, args[0] );
          if ( *str )
            text.content_ = unicodeString( *str );
        }
      }

      auto ctx = getScriptContext( isolate );

      if ( !text.scale_ )
        text.scale_ = ctx->vec3reg()->createFrom( defaultScale );
      if ( !text.translate_ )
        text.translate_ = ctx->vec3reg()->createFrom( defaultTranslate );
      if ( !text.rotate_ )
        text.rotate_ = ctx->quatreg()->createFrom( defaultRotate );

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->textreg()->createFromJS( thisObj, text );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->textreg()->createFrom( text );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( Text, Scale, local_.scale_ )
    JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( Text, Translate, local_.translate_ )
    JS_DYNAMICOBJECT_QUATERNION_PROPERTY_GETSET_IMPLEMENTATIONS( Text, Rotate, local_.rotate_ )

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
      int64_t result = sizeof( Text ) + sizeof( js::Vector3 ) + sizeof( js::Vector3 ) + sizeof( js::Quaternion );
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