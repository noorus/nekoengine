#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"

namespace neko {

  namespace js {

    class Vector2: public DynamicObjectWrapper<Vector2> {
    private:
      vec2 inner_;
    public:
      Vector2( const vec2& source );
      ~Vector2();
      static void jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& info );
      static void registerExport( Isolate* isolate, v8::Local<v8::FunctionTemplate>& obj );
      void js_getX( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setX( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info );
      void js_getY( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setY( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info );
      void js_toString( const V8CallbackArgs& args );
    };

  }

}