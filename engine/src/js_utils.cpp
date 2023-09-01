#pragma once
#include "pch.h"
#include "console.h"
#include "utilities.h"
#include "engine.h"
#include "locator.h"
#include "scripting.h"
#include "console.h"
#include "js_util.h"
#include "js_types.h"
#include "js_game.h"

namespace neko {

  namespace js {

    namespace util {

      /* bool AsyncTryCatchWrapper::check()
      {
        if ( !tryCatch_.HasCaught() )
          return false;
        scriptContext( isolate_ )->onException( tryCatch_ );
        return true;
      }*/

      void debugDumpVariable( ConsolePtr console, Isolate* isolate, utf8String name, V8Value val )
      {
        HandleScope handleScope( isolate );
        auto context = isolate->GetCurrentContext();

        char buf[512];
        vector<utf8String> parts;
        sprintf_s( buf, 512, "Dump: %s =", name.c_str() );
        parts.emplace_back( buf );

        if ( val.IsEmpty() )
        {
          parts.emplace_back( "empty" );
          console->print( srcScripting, utils::arrayJoin( parts, ' ' ) );
        }
        else
        {
          if ( val->IsPrivate() )
            parts.emplace_back("private" );
          if ( val->IsExternal() )
            parts.emplace_back( "external" );
          if ( val->IsProxy() )
            parts.emplace_back( "proxy" );
          if ( val->IsGeneratorFunction() || val->IsGeneratorObject() )
            parts.emplace_back( "generator" );
          if ( val->IsNull() )
            parts.emplace_back( "null" );
          if ( val->IsUndefined() )
            parts.emplace_back( "undefined" );
          if ( val->IsAsyncFunction() )
            parts.emplace_back( "async function" );
          if ( val->IsFunction() )
            parts.emplace_back( "function" );
          if ( val->IsPromise() )
            parts.emplace_back( "promise" );
          if ( val->IsBoolean() )
            parts.emplace_back( "boolean" );
          if ( val->IsModule() )
            parts.emplace_back( "module" );
          if ( val->IsModuleNamespaceObject() )
            parts.emplace_back( "mno" );
          if ( val->IsFunctionTemplate() )
            parts.emplace_back( "function template" );
          if ( val->IsObjectTemplate() )
            parts.emplace_back( "object template" );
          if ( val->IsObject() )
            parts.emplace_back( "object" );
          auto type = util::utf8StringFrom( isolate, val->TypeOf( isolate ) );
          sprintf_s( buf, 512, "[tof: %s]", type.c_str() );
          parts.emplace_back( buf );
          v8::MaybeLocal<v8::Array> maybeProps;
          Local<Object> obj;
          if ( val->IsObject() )
          {
            val->ToObject( context ).ToLocal( &obj );
            auto constructor = util::utf8StringFrom( isolate, obj->GetConstructorName() );
            sprintf_s( buf, 512, "[cnstr: %s]", constructor.c_str() );
            parts.emplace_back( buf );
            maybeProps = obj->GetOwnPropertyNames( context );
          }
          auto vdet = val->ToString( context ); // val->ToDetailString( context );
          if ( !vdet.IsEmpty() && !val->IsFunction() )
          {
            parts.emplace_back( "=" );
            parts.push_back( util::utf8StringFrom( isolate, vdet.ToLocalChecked() ) );
          }
          console->print( srcScripting, utils::arrayJoin( parts, ' ' ) );
          if ( !maybeProps.IsEmpty() && !obj.IsEmpty() )
          {
            auto props = maybeProps.ToLocalChecked();
            for ( uint32_t i = 0; i < props->Length(); ++i )
            {
              auto maybeKey = props->Get( context, i );
              if ( !maybeKey.IsEmpty() )
              {
                auto key = maybeKey.ToLocalChecked();
                if ( key->IsString() )
                {
                  auto skey = util::utf8StringFrom( isolate, key->ToString( context ).ToLocalChecked() );
                  auto maybeValue = obj->Get( context, key );
                  if ( !maybeValue.IsEmpty() )
                    debugDumpVariable( console, isolate, utils::arrayJoin( vector<utf8String>( { name, skey } ), '.' ),
                      maybeValue.ToLocalChecked() );
                }
              }
            }
          }
        }
      }

      void debugDumpFunctionArguments( ConsolePtr console, Isolate* isolate, utf8String name, const V8CallbackArgs& args )
      {
        HandleScope handleScope( isolate );
        auto context = isolate->GetCurrentContext();

        for ( int i = 0; i < args.Length(); i++ )
        {
          auto arg = args[i];
          char asd[128];
          sprintf_s( asd, 128, "%s->Arg%i", name.c_str(), i );
          debugDumpVariable( console, isolate, asd, arg );
        }
      }

    }

  }

}