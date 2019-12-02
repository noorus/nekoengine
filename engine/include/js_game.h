#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"

namespace neko {

  namespace js {

    class Game;
    using JSGamePtr = unique_ptr<Game>;

    class Scene {
    public:
      v8::Global<v8::Function> fnInitialize_;
      v8::Global<v8::Function> fnUpdate_;
      bool initialized_;
    private:
      utf8String name_;
    public:
      Scene(): initialized_( false ) {}
      Scene( const utf8String& name );
      void validate();
      void initialize( V8Context& ctx, GameTime time );
      void update( V8Context& ctx, GameTime time, GameTime delta );
    };

    using SceneMap = map<string, Scene>;

    class Game: public StaticObjectWrapper<Game> {
    private:
      void registerGlobals( Isolate* isolate, V8FunctionTemplate& tpl ) final;
    protected:
      SceneMap scenes_;
    public:
      explicit Game();
    public:
      //! JavaScript Game.registerScene
      void js_registerScene( Isolate* isolate, const V8CallbackArgs& args );
    public:
      static JSGamePtr create( Isolate* isolate, V8Object global );
      void update( V8Context& context, GameTime tick, GameTime time );
    };

  }

}