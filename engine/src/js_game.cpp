#include "stdafx.h"
#include "js_console.h"
#include "js_util.h"
#include "js_game.h"

namespace neko {

  namespace js {

    static const char* c_className = "Game";

    string Game::className( c_className );
    WrappedType Game::internalType = Wrapped_Game;

    Game::Game()
    {
    }

    JSGamePtr Game::create( Isolate* isolate, V8Object global )
    {
      auto instance = make_unique<Game>();
      instance->wrapperRegisterObject( isolate, global );
      return move( instance );
    }

    void Game::registerGlobals( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      JS_WRAPPER_SETOBJMEMBER( tpl, Game, registerScene );
    }

    Scene::Scene( const utf8String& name ): name_( name ), initialized_( false )
    {
      //
    }

    void Scene::validate()
    {
      assert( !fnInitialize_.IsEmpty() && !fnUpdate_.IsEmpty() );
    }

    void Scene::initialize( V8Context& ctx, GameTime time )
    {
      auto isolate = ctx->GetIsolate();
      auto call = V8Function::New( isolate, fnInitialize_ );
      const int argc = 1;
      V8Value argv[argc] = {
        v8::Number::New( isolate, time )
      };
      call->Call( ctx, ctx->Global(), argc, argv );
    }

    void Scene::update( V8Context& ctx, GameTime time, GameTime delta )
    {
      auto isolate = ctx->GetIsolate();
      auto call = V8Function::New( isolate, fnUpdate_ );
      const int argc = 2;
      V8Value argv[argc] = {
        v8::Number::New( isolate, time ),
        v8::Number::New( isolate, delta )
      };
      call->Call( ctx, ctx->Global(), argc, argv );
    }

    //! \verbatim
    //! bool Game.registerScene( scene )
    //! \endverbatim
    void Game::js_registerScene( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );
      auto context = isolate->GetCurrentContext();
      const utf8String funcName( "registerScene" );

      for ( int i = 0; i < args.Length(); i++ )
      {
        auto maybeObj = args[0]->ToObject( context );
        auto name = util::extractStringMember( isolate, funcName, maybeObj, "name" );
        auto initialize = util::extractFunctionMember( isolate, funcName, maybeObj, "initialize" );
        auto update = util::extractFunctionMember( isolate, funcName, maybeObj, "update" );
        Scene scene( name );
        scene.fnInitialize_.Reset( isolate, initialize );
        scene.fnUpdate_.Reset( isolate, update );
        scene.validate();
        scenes_[name] = move( scene );
      }

      args.GetReturnValue().Set( true );
    }

    void Game::update( V8Context& context, GameTime tick, GameTime time )
    {
      HandleScope handleScope( context->GetIsolate() );
      for ( auto& scene : scenes_ )
      {
        if ( !scene.second.initialized_ )
        {
          scene.second.initialize( context, time );
          scene.second.initialized_ = true;
        }
        scene.second.update( context, time, tick );
      }
    }

  }

}