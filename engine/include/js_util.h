#pragma once

#include "neko_types.h"
#include "neko_exception.h"
#include "neko_platform.h"

#include <cstdarg>
#include <v8.h>

#include "js_types.h"

namespace neko {

  namespace js {

    using v8::EscapableHandleScope;
    using v8::Eternal;
    using v8::FunctionCallbackInfo;
    using v8::FunctionTemplate;
    using v8::Function;
    using v8::ObjectTemplate;
    using v8::Global;
    using v8::HandleScope;
    using v8::Isolate;
    using v8::Local;
    using v8::ObjectTemplate;
    using v8::Persistent;
    using v8::PropertyCallbackInfo;
    using v8::Object;
    using v8::Handle;
    using v8::WeakCallbackInfo;
    using v8::WeakCallbackType;
    using v8::Promise;
    using v8::Value;
    using v8::ScriptOrModule;
    using v8::FixedArray;
    using v8::MaybeLocal;
    using v8::String;
    using v8::Context;
    using v8::Message;
    using v8::Module;
    using v8::TryCatch;
    using v8::ArrayBuffer;
    using v8::SealHandleScope;
    using v8::MicrotasksScope;

    using V8CallbackArgs = v8::FunctionCallbackInfo<v8::Value>;
    using V8FunctionTemplate = v8::Local<v8::FunctionTemplate>;
    using V8Object = v8::Local<v8::Object>;
    using V8Promise = v8::Local<v8::Promise>;
    using V8Value = v8::Local<v8::Value>;
    using V8Context = v8::Local<v8::Context>;
    using V8Function = v8::Local<v8::Function>;
    using V8String = v8::Local<v8::String>;

    namespace util {

      class AsyncTryCatchWrapper {
      private:
        Isolate* isolate_ = nullptr;
        TryCatch tryCatch_;
      public:
        AsyncTryCatchWrapper( Isolate* isolate ): isolate_( isolate ), tryCatch_( isolate )
        {
          assert( isolate_ );
          tryCatch_.SetVerbose( true );
        }
        bool check();
        inline V8Promise checkRetPromise( Local<Context> ctx, V8Promise actual )
        {
          if ( !check() )
            return actual;
          auto resolver = v8::Promise::Resolver::New( ctx ).ToLocalChecked();
          resolver->Reject( ctx, tryCatch_.Exception() ).ToChecked();
          return resolver->GetPromise();
        }
      };

      inline bool isWrappedType( const V8Context& ctx, const V8Object& object, WrappedType type )
      {
        if ( object->InternalFieldCount() != Max_WrapField )
          return false;
        auto val = object->GetInternalField( WrapField_Type );
        if ( val.IsEmpty() || !val->IsUint32() )
          return false;
        return ( val->Uint32Value( ctx ).FromMaybe( Max_WrappedType ) == type );
      }

      inline bool getWrappedType( const V8Context& ctx, const V8Value& value, WrappedType& type_out )
      {
        if ( value.IsEmpty() || !value->IsObject() )
          return false;
        auto object = value->ToObject( ctx ).ToLocalChecked();
        if ( object->InternalFieldCount() != Max_WrapField )
          return false;
        auto val = object->GetInternalField( WrapField_Type );
        if ( val.IsEmpty() || !val->IsUint32() )
          return false;
        auto retval = static_cast<WrappedType>( val->Uint32Value( ctx ).FromMaybe( Max_WrappedType ) );
        if ( retval >= Max_WrappedType )
          return false;
        type_out = retval;
        return true;
      }

      extern void debugDumpVariable( ConsolePtr console, Isolate* isolate, utf8String name, V8Value val );
      extern void debugDumpFunctionArguments(
        ConsolePtr console, Isolate* isolate, utf8String name, const V8CallbackArgs& args );

      inline V8String staticStr( Isolate* isolate, const char* data )
      {
        return v8::String::NewFromOneByte(
          isolate, reinterpret_cast<const uint8_t*>( data ), v8::NewStringType::kInternalized, -1 )
          .ToLocalChecked();
      }

      inline V8String staticStr( Isolate* isolate, const char* data, int length )
      {
        return v8::String::NewFromOneByte(
          isolate, reinterpret_cast<const uint8_t*>( data ), v8::NewStringType::kInternalized, length )
          .ToLocalChecked();
      }

      //! Create a local JavaScript String instance from given source string.
      inline V8String allocString( const utf8String& str, Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret =
          v8::String::NewFromUtf8( isolate ? isolate : v8::Isolate::GetCurrent(), str.c_str(), v8::NewStringType::kNormal );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      //! Create a local JavaScript String instance from given source string.
      inline V8String allocString( const wstring& str, Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret = v8::String::NewFromTwoByte(
          isolate ? isolate : v8::Isolate::GetCurrent(), (uint16_t*)str.c_str(), v8::NewStringType::kNormal );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      template <int N>
      inline V8String utf8Literal( Isolate* isolate, const char ( &literal )[N] )
      {
        return String::NewFromUtf8Literal( isolate, literal, v8::NewStringType::kInternalized );
      }

      //! Create a local V8 String that will likely be duplicated (class names etc.)
      inline V8String allocStringConserve( string_view str, Isolate* isolate = nullptr )
      {
        auto ret =
          v8::String::NewFromUtf8( isolate ? isolate : Isolate::GetCurrent(), str.data(), v8::NewStringType::kInternalized );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      inline utf8String utf8StringFrom( Isolate* isolate, Local<String> v8_str )
      {
        String::Utf8Value utf8( isolate, v8_str );
        assert( *utf8 );
        return *utf8;
      }

      inline wstring wideStringFrom( Isolate* isolate, Local<String> v8_str )
      {
        return platform::utf8ToWide( utf8StringFrom( isolate, v8_str ) );
      }

      inline void throwException( Isolate* isolate, const char* message )
      {
        isolate->ThrowException( staticStr( isolate, message ) );
      }

      inline void throwExceptionFmt( Isolate* isolate, const char* fmt, ... )
      {
        char buffer[1024];

        va_list va_alist;
        va_start( va_alist, fmt );
        _vsnprintf_s( buffer, 1024, fmt, va_alist );
        va_end( va_alist );

        isolate->ThrowException( allocString( buffer, isolate ) );
      }

      inline void throwException( Isolate* isolate, string_view message )
      {
        isolate->ThrowException( staticStr( isolate, message.data() ) );
      }

      inline void throwException( Isolate* isolate, const wchar_t* message, ... )
      {
        wchar_t buffer[1024];

        va_list va_alist;
        va_start( va_alist, message );
        _vsnwprintf_s( buffer, 1024, message, va_alist );
        va_end( va_alist );

        isolate->ThrowException( allocString( buffer, isolate ) );
      }

      inline Real realFromArray( V8Context& context, v8::Local<v8::Array> arrayValue, uint32_t index )
      {
        return static_cast<Real>(
          arrayValue->Get( context, index ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
      }

      inline int intFromArray( V8Context& context, v8::Local<v8::Array> arrayValue, uint32_t index )
      {
        return static_cast<int>(
          arrayValue->Get( context, index ).ToLocalChecked()->Int32Value( context ).FromJust() );
      }

      inline int64_t int64FromValue( V8Context& context, V8Value value )
      {
        if ( !value->IsNumber() )
        {
          util::throwException( context->GetIsolate(), "Expected a GameValue (int64)" );
          return 0;
        }
        return static_cast<int64_t>( value->NumberValue( context ).FromMaybe( 0 ) );
      }

      inline int64_t int64FromArray( V8Context& context, v8::Local<v8::Array> arrayValue, uint32_t index )
      {
        return static_cast<int64_t>(
          arrayValue->Get( context, index ).ToLocalChecked()->IntegerValue( context ).FromJust() );
      }

      template <typename T>
      shared_ptr<T> extractWrappedArg( V8Context& context, const V8CallbackArgs& args, int index )
      {
        auto isolate = context->GetIsolate();
        if ( index >= args.Length() )
        {
          isolate->ThrowException( util::staticStr( isolate, "Bad argument count" ) );
          return {};
        }
        WrappedType argWrapType = Max_WrappedType;
        if ( !util::getWrappedType( context, args[index], argWrapType ) || argWrapType != T::internalType )
        {
          util::throwExceptionFmt( isolate, "Argument %i is not of expected class %s", index, T::className.c_str() );
          return {};
        }
        V8Object object;
        if ( !args[index]->ToObject( context ).ToLocal( &object ) )
        {
          util::throwException( isolate, "Object is somehow empty" );
          return {};
        }
        return T::unwrap( object )->shared_from_this();
      }

      inline int64_t extractInt64Arg(
        V8Context& context, const V8CallbackArgs& args, int index, int64_t defaultValue = 0 )
      {
        auto isolate = context->GetIsolate();
        if ( index >= args.Length() )
        {
          isolate->ThrowException( util::staticStr( isolate, "Bad argument count" ) );
          return defaultValue;
        }
        if ( !args[index]->IsNumber() )
        {
          util::throwExceptionFmt( isolate, "Argument %i is not a number", index );
          return defaultValue;
        }
        return static_cast<int64_t>( args[index]->IntegerValue( context ).FromMaybe( defaultValue ) );
      }

      inline Real extractNumberComponent( Isolate* isolate, v8::MaybeLocal<v8::Object>& obj, string_view name, Real defval )
      {
        if ( obj.IsEmpty() )
          return defval;
        auto val = obj.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( val.IsEmpty() || !val.ToLocalChecked()->IsNumber() )
          return defval;
        return static_cast<Real>(
          val.ToLocalChecked()->NumberValue( isolate->GetCurrentContext() ).FromMaybe( static_cast<double>( defval ) ) );
      }

      inline utf8String extractStringArg(
        V8Context& context, const V8CallbackArgs& args, int index, const utf8String& defaultValue )
      {
        auto isolate = context->GetIsolate();
        if ( index >= args.Length() )
        {
          isolate->ThrowException( util::staticStr( isolate, "Bad argument count" ) );
          return defaultValue;
        }
        if ( !args[index]->IsString() )
        {
          util::throwExceptionFmt( isolate, "Argument %i is not a string", index );
          return defaultValue;
        }

        v8::String::Utf8Value value( isolate, args[index] );
        return ( *value ? *value : defaultValue );
      }

      inline utf8String extractStringMember( Isolate* isolate, const utf8String& func,
        v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow = true )
      {
        const utf8String emptyString;
        if ( maybeObject.IsEmpty() )
        {
          util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return emptyString;
        }
        auto object =
          maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsString() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( func + ": passed object has no string member \"" + name + "\"" ).c_str() );
          return emptyString;
        }
        v8::String::Utf8Value value( isolate, object.ToLocalChecked() );
        return ( *value ? *value : emptyString );
      }

      inline optional<int> extractIntMember(
        Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name )
      {
        if ( maybeObject.IsEmpty() )
        {
          util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return {};
        }
        auto object =
          maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsNumber() )
        {
          util::throwException( isolate, ( func + ": passed object has no number member \"" + name + "\"" ).c_str() );
          return {};
        }
        return static_cast<int>( object.ToLocalChecked()->Int32Value( isolate->GetCurrentContext() ).FromJust() );
      }

      inline optional<int64_t> extractInt64Member(
        Isolate* isolate, const utf8String& func, V8Object lobject, const utf8String& name )
      {
        auto object = lobject->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsNumber() )
        {
          util::throwException( isolate, ( func + ": passed object has no number member \"" + name + "\"" ).c_str() );
          return {};
        }
        return static_cast<int64_t>( object.ToLocalChecked()->IntegerValue( isolate->GetCurrentContext() ).FromJust() );
      }

      inline optional<Local<v8::Array>> extractArrayMember(
        Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name )
      {
        if ( maybeObject.IsEmpty() )
        {
          util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return {};
        }
        auto member =
          maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( member.IsEmpty() || !member.ToLocalChecked()->IsArray() )
        {
          util::throwException( isolate, ( func + ": passed object has no array member \"" + name + "\"" ).c_str() );
          return {};
        }
        return v8::Local<v8::Array>::Cast( member.ToLocalChecked() );
      }

      inline unicodeString extractStringMemberUnicode( Isolate* isolate, const utf8String& func,
        v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow = true )
      {
        const unicodeString emptyString;
        if ( maybeObject.IsEmpty() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return emptyString;
        }
        auto object =
          maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsString() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( func + ": passed object has no string member \"" + name + "\"" ).c_str() );
          return emptyString;
        }
        v8::String::Value value( isolate, object.ToLocalChecked() );
        return ( *value ? unicodeString( *value ) : emptyString );
      }

      inline V8Function extractFunctionMember( Isolate* isolate, const utf8String& func,
        v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow = true )
      {
        const V8Function emptyFunc;
        if ( maybeObject.IsEmpty() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return emptyFunc;
        }
        auto object =
          maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsFunction() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( func + ": passed object has no function member \"" + name + "\"" ).c_str() );
          return emptyFunc;
        }
        return V8Function::New( isolate, V8Function::Cast( object.ToLocalChecked() ) );
      }

    }

  #define JS_TEMPLATE_ACCESSOR( tpl, x, y, z ) tpl->PrototypeTemplate()->SetAccessor( util::allocString( x ), y, z )

  #define JS_SET_GLOBAL_FUNCTION( x )                                                       \
    global->Set( js::util::allocStringConserve( #x, isolate_ ),                             \
      FunctionTemplate::New(                                                                \
      isolate_,                                                                             \
      []( const v8::FunctionCallbackInfo<Value>& args ) {                                   \
        auto self = static_cast<ScriptContext*>( args.Data().As<v8::External>()->Value() ); \
        self->js_##x( args );                                                               \
      },                                                                                    \
      v8::External::New( isolate_, static_cast<void*>( this ) ) ) )

  //! Use this to create member functions for variables in dynamic-wrapped objects' templates.
  #define JS_WRAPPER_SETMEMBER( obj, cls, x )                                                          \
    obj->PrototypeTemplate()->Set(                                                                     \
    util::utf8Literal( isolate, #x ), FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
    auto self = args.This();                                                                           \
    if ( !util::isWrappedType( args.GetIsolate()->GetCurrentContext(), self, internalType ) )          \
      return;                                                                                          \
    auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) );     \
    obj->js_##x( args );                                                                               \
  } ) )

  //! Use this to create member functions for variables in dynamic-wrapped objects' templates.
  #define JS_WRAPPER_SETMEMBERNAMED( obj, cls, x, y )                                                   \
   obj->PrototypeTemplate()->Set(                                                                       \
     util::utf8Literal( isolate, #y ), FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
     auto self = args.This();                                                                           \
     if ( !util::isWrappedType( args.GetIsolate()->GetCurrentContext(), self, internalType ) )          \
      return;                                                                                           \
     auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) );     \
     obj->js_##x( args );                                                                               \
     } ) )

  //! Use this to create member functions for static-wrapped objects (instances).
  #define JS_WRAPPER_SETOBJMEMBER(tpl,cls,x) tpl->PrototypeTemplate()->Set( \
    util::utf8Literal( isolate, #x ), \
    FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
      auto self = static_cast<cls*>( args.Data().As<v8::External>()->Value() ); \
      self->js_##x( args.GetIsolate(), args ); \
    }, v8::External::New( isolate, (void*)this ) ) )

  //! Use this to create member functions for static-wrapped objects (instances) with another name.
  #define JS_WRAPPER_SETOBJMEMBERNAMED(tpl,cls,x,y) tpl->PrototypeTemplate()->Set( \
    util::utf8Literal( isolate, #y ), \
    FunctionTemplate::New( isolate, []( const V8CallbackArgs& args ) { \
      auto self = static_cast<cls*>( args.Data().As<v8::External>()->Value() ); \
      self->js_##x( args.GetIsolate(), args ); \
    }, v8::External::New( isolate, (void*)this ) ) )

  //! Use this to create accessors for variables in dynamic-wrapped objects' templates.
  #define JS_WRAPPER_SETACCESSOR(obj,cls,x,valInternal) obj->PrototypeTemplate()->SetAccessor( \
    util::utf8Literal( isolate, #x ), []( V8String prop, const PropertyCallbackInfo<v8::Value>& info ) { \
      auto self = info.This(); \
      if ( !util::isWrappedType( info.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
        return; \
      auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
      obj->js_get##valInternal( prop, info ); \
    }, []( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info ) { \
      auto self = info.This(); \
      if ( !util::isWrappedType( info.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
        return; \
      auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
      obj->js_set##valInternal( prop, value, info ); \
    } )

  // clang-format off

  #define JS_WRAPPER_SETREADONLYACCESSOR( obj, cls, x, valInternal ) \
    obj->PrototypeTemplate()->SetAccessor( \
      util::utf8Literal( isolate, #x ), \
      []( V8String prop, const PropertyCallbackInfo<v8::Value>& info ) { \
        auto self = info.This(); \
        if ( !util::isWrappedType( info.GetIsolate()->GetCurrentContext(), self, internalType ) ) \
          return; \
        auto obj = static_cast<cls*>( self->GetAlignedPointerFromInternalField( WrapField_Pointer ) ); \
        obj->js_get##valInternal( prop, info ); \
      }, \
      nullptr, \
      V8Value(), \
      v8::AccessControl::ALL_CAN_READ, \
      (v8::PropertyAttribute)(v8::PropertyAttribute::ReadOnly | v8::PropertyAttribute::DontDelete) \
    )

  }

}