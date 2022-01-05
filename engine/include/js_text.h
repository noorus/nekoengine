#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "mesh_primitives.h"
#include "js_wrapper.h"
#include "locator.h"

namespace neko {

  // Lightweight, purposefully copyable
  class JSText {
  public:
    size_t id_;
    unicodeString content_;
    JSText();
  };

  namespace js {

    class Text: public DynamicObjectWrapper<Text, JSText> {
    private:
      JSText local_;
    protected:
      void js_toString( const V8CallbackArgs& args );
    public:
      static void jsConstructor( const V8CallbackArgs& info );
      virtual int32_t jsEstimateSize() const;
      virtual void jsOnDestruct( Isolate* isolate );
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
    public:
      Text( const JSText& source ): local_( source ) {}
      inline void setFrom( const JSText& other )
      {
        Locator::console().printf( neko::Console::srcScripting,
          "js::Text(0x%I64X) setFrom JSText(0x%I64X)", this, other );
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