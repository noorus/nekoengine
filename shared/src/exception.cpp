#include "stdafx.h"
#include "neko_exception.h"

namespace neko {

  Exception::Exception( const string& description ): description_( description )
  {
  }

  Exception::Exception( const string& description, const string& source ) :
    description_( description ), source_( source )
  {
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