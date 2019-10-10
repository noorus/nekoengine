#include "stdafx.h"
#include "neko_exception.h"

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
  int          code;
  const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

namespace neko {

  Exception::Exception( const string& description ): description_( description )
  {
  }

  Exception::Exception( const string& description, const string& source ) :
    description_( description ), source_( source )
  {
  }


  Exception::Exception( const string& description, FT_Error error, const string& source ):
    description_( description ), source_( source )
  {
    description_.append( " (" + string( FT_Errors[error].message ) + ")" );
  }

  Exception::Exception( const string& description, gl::GLenum gle, const string& source ):
    description_( description ), source_( source )
  {
    description_.append( " (" + glbinding::aux::Meta::getString( gle ) + ")" );
  }

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