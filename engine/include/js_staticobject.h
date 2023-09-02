#pragma once

#include "neko_types.h"
#include "neko_exception.h"

#include "js_util.h"
#include "js_types.h"

namespace neko {

  namespace js {

    #define JS_STATICOBJECT_DECLARE_STATICS( T )  \
     utf8String StaticObject<T>::className( #T ); \
     WrappedType StaticObject<T>::internalType = Wrapped_##T##;

    template <class T>
    class StaticObject {
      friend class neko::Scripting;
    protected:
      //! My JavaScript-exported class name
      static utf8String className;
      //! My magic internal type
      static WrappedType internalType;
    protected:
      void wrapperRegisterObject( Isolate* isolate, Local<Object> global )
      {
        global
          ->Set( isolate->GetCurrentContext(), js::util::allocStringConserve( className, isolate ),
            wrapperImplCreateFunction( isolate ) )
          .FromJust();
      }
      inline Local<Object> wrapperImplCreateFunction( Isolate* isolate )
      {
        EscapableHandleScope scope( isolate );

        auto tpl = FunctionTemplate::New( isolate );
        tpl->SetClassName( util::allocString( className, isolate ) );
        tpl->InstanceTemplate()->SetInternalFieldCount( Max_WrapField );

        registerGlobals( isolate, tpl );

        auto classFn = tpl->GetFunction( isolate->GetCurrentContext() ).ToLocalChecked();
        auto classInst = classFn->NewInstance( isolate->GetCurrentContext() ).ToLocalChecked();
        classInst->SetInternalField( WrapField_Type, v8::Uint32::New( isolate, internalType ) );
        classInst->SetAlignedPointerInInternalField( WrapField_Pointer, this );

        return scope.Escape( classInst );
      }
    protected:
      virtual void registerGlobals( Isolate* isolate, Local<FunctionTemplate>& tpl ) = 0;
    };

  }

}