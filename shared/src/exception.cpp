#include "stdafx.h"
#include "neko_exception.h"
#ifdef NEKO_PLATFORM_WINDOWS
# include "windows.h"
#endif

#ifndef NEKO_SIDELIBRARY_BUILD
// FreeType stuff
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
  int          code;
  const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H
#endif

namespace neko {

  Exception::Exception( const string& description ): description_( description )
  {
  }

  Exception::Exception( const string& description, const string& source, Exception::Type type ) :
    description_( description ), source_( source )
  {
#ifdef NEKO_PLATFORM_WINDOWS
    if ( type == Exception::Type_WinAPI )
    {
      LPSTR message = nullptr;
      auto code = GetLastError();
      FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (LPSTR)&message, 0, NULL );
      description_.append( message );
      LocalFree( message );
    }
#endif
  }

#ifndef NEKO_SIDELIBRARY_BUILD
  Exception::Exception( const string& description, FT_Error error, const string& source ):
    description_( description ), source_( source )
  {
    description_.append( " (" + string( FT_Errors[error].message ) + ")" );
  }

  Exception::Exception( const string& description, gl::GLenum gle, const string& source ):
    description_( description ), source_( source )
  {
#ifndef RELEASE
    description_.append( " (" + glbinding::aux::Meta::getString( gle ) + ")" );
#else
    description_.append( " (" + std::to_string( (unsigned int)gle ) + ")" );
#endif
  }
#endif

  const string& Exception::getFullDescription() const
  {
    if ( fullDescription_.empty() )
    {
      stringstream stream;
      stream << description_;
      if ( !source_.empty() )
        stream << "\r\nIn function " << source_;
      fullDescription_ = stream.str();
    }

    return fullDescription_;
  }

  const char* Exception::what() const throw()
  {
    return getFullDescription().c_str();
  }

}