#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include <v8.h>

namespace neko {

  namespace js {

    using v8::Isolate;
    using v8::HandleScope;
    using v8::Local;
    using v8::Persistent;
    using v8::Eternal;
    using v8::Global;
    using v8::PropertyCallbackInfo;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;
    using v8::EscapableHandleScope;

  }

}