#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"
#include "forwards.h"

namespace neko {

  namespace js {

    class Console: public Wrapper<Console> {
    protected:
      static Console* instance;
      ConsolePtr mConsole;
      explicit Console( ConsolePtr console );
      //! JavaScript Console.print.
      static void jsPrint( const FunctionCallbackInfo<v8::Value>& args );
      //! JavaScript Console.getVariable.
      static void jsGetVariable( const FunctionCallbackInfo<v8::Value>& args );
      //! JavaScript Console.setVariable.
      static void jsSetVariable( const FunctionCallbackInfo<v8::Value>& args );
      //! JavaScript Console.execute.
      static void jsExecute( const FunctionCallbackInfo<v8::Value>& args );
    public:
      inline ConsolePtr getConsole() { return mConsole; }
      static void initialize( ConsolePtr console, Local<v8::Context> context );
      static void shutdown();
    };

  }

}