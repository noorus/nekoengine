#include "pch.h"
#include "utilities.h"

namespace neko {

  namespace utils {

    thread_local char tls_ilprintfBuffer[16384];

    utf8String ilprintf( const char* fmt, ... )
    {
      va_list va_alist;
      va_start( va_alist, fmt );
      _vsnprintf_s( tls_ilprintfBuffer, 16384, fmt, va_alist );
      va_end( va_alist );
      return tls_ilprintfBuffer;
    }

  }

}