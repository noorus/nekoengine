#pragma once
#include "neko_types.h"

namespace neko {

#ifdef _DEBUG
# define NEKO_RELEASE_EXCEPTIONS_TRY
# define NEKO_RELEASE_EXCEPTION_CATCHERS
#else
# define NEKO_RELEASE_EXCEPTIONS_TRY try
# define NEKO_RELEASE_EXCEPTION_CATCHERS                                                      \
  catch ( Exception & e )                                                                    \
  {                                                                                          \
   stringstream stream;                                                                      \
   stream << "A C++ exception occurred in " << platform::g_moduleName << ".\n";              \
   stream << platform::windowsExceptionMessage( GetCurrentThread(), e );                     \
   if ( !e.stack().empty() )                                                                 \
    stream << "\nCall stack:\n" << e.stack();                                                \
   platform::showFormattedError( stream, "C++ exception" );                                  \
   FatalAppExitA( -1, stream.str().c_str() );                                                \
  }                                                                                          \
  catch ( std::exception & e )                                                               \
  {                                                                                          \
   stringstream stream;                                                                      \
   stream << "A C++ exception occurred in " << platform::g_moduleName << ".\n";              \
   stream << platform::windowsExceptionMessage( GetCurrentThread(), e );                     \
   platform::showFormattedError( stream, "C++ exception" );                                  \
   FatalAppExitA( -1, stream.str().c_str() );                                                \
  }                                                                                          \
  catch ( ... )                                                                              \
  {                                                                                          \
   stringstream stream;                                                                      \
   stream << "An unsupported C++ exception occurred in " << platform::g_moduleName << ".\n"; \
   stream << platform::getThreadDescriptor( GetCurrentThread() );                            \
   platform::showFormattedError( stream, "Unknown exception" );                              \
   FatalAppExitA( -1, stream.str().c_str() );                                                \
  }
#endif

  class Exception: public std::exception {
  private:
    CONTEXT context_;
    utf8String message_;
    utf8String source_;
    utf8String stack_;
    mutable utf8String fullDescription_;
    DWORD code_ = 0xFFFFFFFF;
  protected:
    inline void setMessage( string_view msg ) { message_ = msg; }
    inline void setSource( string_view src ) { source_ = src; }
    inline void setCode( DWORD code ) { code_ = code; }
  public:
    Exception();
    Exception( const utf8String& description );
    Exception( const utf8String& description, const utf8String& source );
    Exception( const utf8String& description, const utf8String& source, DWORD code );
    Exception( const utf8String& description, gl::GLenum gle, const utf8String& source );
    inline const utf8String& message() const { return message_; }
    inline const utf8String& source() const { return source_; }
    inline const utf8String& stack() const { return stack_; }
    inline DWORD code() const { return code_; }
    virtual const utf8String& getFullDescription() const;
    virtual const char* what() const noexcept override;
  };

  class WinapiException: public Exception {
  public:
    WinapiException( DWORD code, string_view message )
    {
      setCode( code );
      setMessage( message );
    }
    WinapiException( DWORD code, string_view message, string_view caller )
    {
      setCode( code );
      setMessage( message );
      setSource( caller );
    }
    static WinapiException createFromLastError( string_view func, string_view caller );
  };

#if defined( NEKO_EXCEPT )
# error NEKO_EXCEPT* macro already defined!
#else
# define NEKO_EXCEPT( description )                    \
  {                                                    \
   throw neko::Exception( description, __FUNCTION__ ); \
  }
# define NEKO_OPENGL_EXCEPT( description, en )             \
  {                                                        \
   throw neko::Exception( description, en, __FUNCTION__ ); \
  }
# define NEKO_WINAPI_EXCEPT( winapifunc )                                        \
  {                                                                              \
   throw neko::WinapiException::createFromLastError( winapifunc, __FUNCTION__ ); \
  }
#endif

}