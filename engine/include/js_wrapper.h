#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "forwards.h"
#include "js_util.h"

namespace neko {

  namespace js {

#   define JS_WRAPPER_SETMEMBER(tpl,cls,x) tpl->PrototypeTemplate()->Set( \
      util::allocStringConserve( #x, isolate ), \
      FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
        auto self = dirty::externalUnwrap<cls>( args.Data() ); \
        self->js_##x( args.GetIsolate(), args ); \
      }, dirty::externalWrap( isolate, this ) ) )

    template <class T>
    class StaticObjectWrapper {
      friend class neko::Scripting;
    protected:
      //! My JavaScript-exported class name
      static string className;
    protected:
      void wrapperRegisterObject( Isolate* isolate, V8Object& global )
      {
        global->Set( isolate->GetCurrentContext(),
          js::util::allocStringConserve( className, isolate ),
          wrapperImplCreateFunction( isolate )
        ).FromJust();
      }
      inline v8::Local<v8::Object> wrapperImplCreateFunction( Isolate* isolate )
      {
        v8::EscapableHandleScope scope( isolate );

        auto tpl = FunctionTemplate::New( isolate );
        tpl->SetClassName( util::allocString( className, isolate ) );
        tpl->InstanceTemplate()->SetInternalFieldCount( 1 );

        registerGlobals( isolate, tpl );

        auto classFn = tpl->GetFunction( isolate->GetCurrentContext() ).ToLocalChecked();
        auto classInst = classFn->NewInstance( isolate->GetCurrentContext() ).ToLocalChecked();
        //auto ext = v8::External::New( isolate, instance );
        //fninst->SetInternalField( 0, ext );

        return scope.Escape( classInst );
      }
    protected:
      virtual void registerGlobals( Isolate* isolate, v8::Local<v8::FunctionTemplate>& tpl ) = 0;
    };

  }

}