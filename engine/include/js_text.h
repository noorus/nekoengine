#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "mesh_primitives.h"
#include "locator.h"
#include "js_dynamicobject.h"
#include "js_math.h"

namespace neko {

  // Lightweight, purposefully copyable
  class JSText {
  public:
    size_t id_;
    unicodeString content_;
    TextPtr impl_;
    js::Vector3Ptr scale_;
    js::Vector3Ptr translate_;
    js::QuaternionPtr rotate_;
    JSText();
  };

  namespace js {

    class Text: public DynamicObject<Text, JSText> {
    private:
      JSText local_; //!< Internal model.
    protected:
      //! Properties
      void js_getScale( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setScale( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getTranslate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setTranslate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      void js_getRotate( V8String prop, const PropertyCallbackInfo<v8::Value>& info );
      void js_setRotate( V8String prop, V8Value value, const PropertyCallbackInfo<void>& info );
      //! Functions
      void js_toString( const V8CallbackArgs& args );
    public:
      static void jsConstructor( const V8CallbackArgs& info );
      int32_t jsEstimateSize() const override;
      void jsOnDestruct( Isolate* isolate ) override;
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
    public:
      Text( const JSText& source ): local_( source ) {}
      inline void setFrom( const JSText& other )
      {
        Locator::console().printf( neko::srcScripting,
          "js::Text(0x%I64X) setFrom JSText(0x%I64X)", this, reinterpret_cast<const void*>( &other ) );
        // local_.txt_ = other.txt_;
      }
      inline JSText& text() { return local_; } //!< Get the internal JSText.
    };

    using TextPtr = shared_ptr<js::Text>;
    using TextVector = vector<js::Text*>;
    using TextPtrVector = vector<js::TextPtr>;

  }

}

#endif