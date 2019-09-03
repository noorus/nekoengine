#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"
#include "forwards.h"
#include <type_traits>

namespace neko {

  namespace js {

    class Scripting;

    template <class T>
    constexpr inline v8::Local<v8::External> v8extwrap( v8::Isolate* isolate, T val )
    {
      return v8::External::New( isolate, (void*)val );
    }

    template <class T>
    constexpr inline T* v8extunwrap( v8::Local<v8::Value>& val )
    {
      auto self = v8::Local<v8::External>::Cast( val );
      return static_cast<T*>( self->Value() );
    }

    using V8CallbackArgs = v8::FunctionCallbackInfo<v8::Value>;

#   define JS_TEMPLATE_SETNEW(tpl,cls,x) tpl->PrototypeTemplate()->Set( \
      util::allocStringConserve( #x, Isolate::GetCurrent() ), \
      FunctionTemplate::New( Isolate::GetCurrent(), []( const V8CallbackArgs& args ) { \
        auto self = v8extunwrap<cls>( args.Data() ); \
        self->js_##x( args.GetIsolate(), args ); \
      }, v8extwrap( Isolate::GetCurrent(), this ) ) )

    template <class T>
    class WrapThatShit {
      friend class Scripting;
    protected:
      //! My JavaScript-exported class name
      static string className;
    public:
      void wrappedRegisterObject( Isolate* isolate, v8::Local<v8::Object>& global )
      {
        global->Set( isolate->GetCurrentContext(),
          js::util::allocStringConserve( className, isolate ),
          wrappedImplCreateFunction( isolate )
        ).FromJust();
      }
    protected:
      inline v8::Local<v8::Object> wrappedImplCreateFunction( Isolate* isolate )
      {
        v8::EscapableHandleScope scope( isolate );

        auto tpl = FunctionTemplate::New( isolate );
        tpl->SetClassName( util::allocString( className, isolate ) );
        tpl->InstanceTemplate()->SetInternalFieldCount( 1 );

        registerGlobals( tpl );

        auto classFn = tpl->GetFunction( isolate->GetCurrentContext() ).ToLocalChecked();
        auto classInst = classFn->NewInstance( isolate->GetCurrentContext() ).ToLocalChecked();
        //auto ext = v8::External::New( isolate, instance );
        //fninst->SetInternalField( 0, ext );

        return scope.Escape( classInst );
      }
    public:
      virtual void registerGlobals( v8::Local<v8::FunctionTemplate>& tpl ) = 0;
    };

#   define JS_TEMPLATE_SET(tpl,cls,x) JS_TEMPLATE_SETNEW(tpl,cls,x)

    class Console: public WrapThatShit<Console> {
      friend class Scripting;
    public:
      ConsolePtr mConsole;
      explicit Console( ConsolePtr console );
    public:
      void registerGlobals( v8::Local<v8::FunctionTemplate>& tpl ) override;
    public:
      //! JavaScript Console.print.
      void js_print( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Console.getVariable.
      void js_getVariable( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Console.setVariable.
      void js_setVariable( Isolate* isolate, const V8CallbackArgs& args );
      //! JavaScript Console.execute.
      void js_execute( Isolate* isolate, const V8CallbackArgs& args );
    };

  }

}