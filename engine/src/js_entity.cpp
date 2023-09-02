#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_util.h"
# include "console.h"
# include "scripting.h"
# include "nekomath.h"
# include "js_math.h"
# include "js_mathutil.h"
# include "js_entity.h"
# include "locator.h"
# include "console.h"

namespace neko {

  namespace js {

    JS_MAPPEDDYNAMICOBJECT_DECLARE_STATICS( Entity, JSEntity )

    void Entity::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Entity, id, ID );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Entity, toString );
      JS_WRAPPER_SETMEMBER( tpl, Entity, has );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Entity, has, contains );
    }

    void Entity::jsConstructor( const V8CallbackArgs& args )
    {
      JS_DYNAMICOBJECT_CONSTRUCTBODY_BEGIN( Entity )

      if ( args.Length() != 1 || !args[0]->IsUint32() )
      {
        util::throwException( isolate, "Syntax error: Entity::constructor expected entity ID as argument 0" );
        return;
      }

      auto eid = util::uint32FromValue( context, args[0], 0, true );

      JSEntity loc( eid );
      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = scriptCtx->registry<Entity>( type<Entity> {} )->createFromJS( thisObj, static_cast<uint64_t>( eid ), loc );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = scriptCtx->registry<Entity>( type<Entity> {} )->createFrom( static_cast<uint64_t>( eid ), loc );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    void Entity::js_toString( const V8CallbackArgs& args )
    {
      const auto& name = scriptContext( args.GetIsolate() )->scene().nd( local_.eid ).name;
      args.GetReturnValue().Set(
        util::allocString( utils::ilprinf( R"(entity[%i,"%s"])", local_.eid, name.c_str() ), args.GetIsolate() ) );
    }

    void Entity::js_getID( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<uint32_t>( local_.eid ) );
    }

    void Entity::js_setID( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      auto eid = static_cast<c::entity>( util::uint32FromValue( context, value, 0, true ) );
      if ( eid != local_.eid )
        util::throwException( info.GetIsolate(), "Cannot assign to entity.id" );
    }

    JS_DYNAMICOBJECT_MEMBERFUNCTION_BEGIN( Entity, has )
    {
      const auto& cname = util::extractStringArg( context, args, 0, "" );

      ret = v8::False( isolate );
      if ( cname == "transform" && scriptCtx->scene().reg().any_of<c::transform>( local_.eid ) )
        ret = v8::True( isolate );
      else if ( cname == "renderable" && scriptCtx->scene().reg().any_of<c::renderable>( local_.eid ) )
        ret = v8::True( isolate );
      else if ( cname == "camera" && scriptCtx->scene().reg().any_of<c::camera>( local_.eid ) )
        ret = v8::True( isolate );
      else if ( cname == "primitive" && scriptCtx->scene().reg().any_of<c::primitive>( local_.eid ) )
        ret = v8::True( isolate );
      else if ( cname == "text" && scriptCtx->scene().reg().any_of<c::text>( local_.eid ) )
        ret = v8::True( isolate );
      else if ( cname == "sprite" && scriptCtx->scene().reg().any_of<c::sprite>( local_.eid ) )
        ret = v8::True( isolate );
    }
    JS_DYNAMICOBJECT_MEMBERFUNCTION_END()

    int32_t Entity::jsEstimateSize() const
    {
      int64_t result = sizeof( Text ) + sizeof( js::Vector3 ) + sizeof( js::Vector3 ) + sizeof( js::Quaternion );
      // add in the size of the mesh and whatever in bytes maybe
      return static_cast<int32_t>( std::min( static_cast<int64_t>( std::numeric_limits<int32_t>::max() ), result ) );
    }

    void Entity::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
    }

  }

}

#endif