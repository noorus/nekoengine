#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_staticobject.h"

namespace neko {

  namespace js {

    class Console;
    using JSConsolePtr = unique_ptr<Console>;

    class Console: public StaticObject<Console> {
    private:
      ConsolePtr console_;
      void registerGlobals( Isolate* isolate, V8FunctionTemplate& tpl ) final;
    public:
      explicit Console( ConsolePtr console );
    public:
      //! JavaScript Console.print.
      void js_print( const V8CallbackArgs& args );
      //! JavaScript Console.dump.
      void js_dump( const V8CallbackArgs& args );
      //! JavaScript Console.getVariable.
      void js_getVariable( const V8CallbackArgs& args );
      //! JavaScript Console.setVariable.
      void js_setVariable( const V8CallbackArgs& args );
      //! JavaScript Console.execute.
      void js_execute( const V8CallbackArgs& args );
    public:
      static JSConsolePtr create( ConsolePtr console, Isolate* isolate, V8Object global );
    };

  }

}

#endif