 #include "pch.h"
#include "neko_exception.h"
#include "neko_platform.h"
#ifdef NEKO_PLATFORM_WINDOWS
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# include "windows.h"
#endif

namespace neko {

  __forceinline utf8String stackTrace( const PCONTEXT ctx )
  {
    auto thread = GetCurrentThread();
    platform::SinkedStackWalker sw;
    sw.ShowCallstack( thread, ctx );
    return sw.output();
  }

  Exception::Exception()
  {
#ifndef DEBUG
    RtlCaptureContext( &context_ );
    stack_ = stackTrace( &context_ );
#endif
  }

  Exception::Exception( const utf8String& description ): message_( description )
  {
#ifndef DEBUG
    RtlCaptureContext( &context_ );
    stack_ = stackTrace( &context_ );
#endif
  }

  Exception::Exception( const utf8String& description, const utf8String& source ): message_( description ), source_( source )
  {
#ifndef DEBUG
    RtlCaptureContext( &context_ );
    stack_ = stackTrace( &context_ );
#endif
  }

  Exception::Exception( const utf8String& description, const utf8String& source, DWORD code ):
    message_( description ), source_( source ), code_( code )
  {
#ifndef DEBUG
    RtlCaptureContext( &context_ );
    stack_ = stackTrace( &context_ );
#endif
  }

  Exception::Exception( const string& description, gl::GLenum gle, const string& source ):
    message_( description ), source_( source )
  {
#ifndef RELEASE
    message_.append( " (" + glbinding::aux::Meta::getString( gle ) + ")" );
#else
    message_.append( " (" + std::to_string( (unsigned int)gle ) + ")" );
#endif
  }

  WinapiException WinapiException::createFromLastError( string_view func, string_view caller )
  {
    auto code = GetLastError();
    string msg = platform::formatWinapiError( func, code );
    return { code, msg, caller };
  }

  const string& Exception::getFullDescription() const
  {
    if ( fullDescription_.empty() )
    {
      stringstream stream;
      stream << message_;
      if ( !source_.empty() )
        stream << "\nIn function " << source_;
      fullDescription_ = stream.str();
    }

    return fullDescription_;
  }

  const char* Exception::what() const noexcept
  {
    return getFullDescription().c_str();
  }

}