#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"
#include "forwards.h"

namespace neko {

  namespace js {

    class Console {
    protected:
      //! My JavaScript-exported constructor function template.
      static Global<FunctionTemplate> constructor;
      //! My JavaScript-exported class name
      static string className;
      //! Internal v8 object handle
      Global<v8::Object> jsHandle_;
      int jsReferences_;
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
      void ref();
      void unref();
      inline Local<v8::Object> handle()
      {
        return handle( Isolate::GetCurrent() );
      }
      inline Local<v8::Object> handle( Isolate* isolate )
      {
        return Local<v8::Object>::New( isolate, persistent() );
      }
      inline Global<v8::Object>& persistent()
      {
        return jsHandle_;
      }
      void makeWeak();
      static void initialize( ConsolePtr console, Isolate* isolate, Local<v8::Context> context );
      static void shutdown();
    };

  }

}