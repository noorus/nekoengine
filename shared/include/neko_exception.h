#pragma once
#include "neko_types.h"

namespace neko {

  class Exception: public std::exception {
  protected:
    string description_;
    string source_;
    mutable string fullDescription_;
  public:
    Exception( const string& description );
    Exception( const string& description, const string& source );
    Exception( const string& description, FT_Error error, const string& source );
    Exception( const string& description, gl::GLenum gle, const string& source );
    virtual const string& getFullDescription() const;
    virtual const char* what() const throw() override;
  };

#if defined(NEKO_EXCEPT)
# error NEKO_EXCEPT* macro already defined!
#else
# define NEKO_EXCEPT(description) {throw neko::Exception(description,__FUNCTION__);}
# define NEKO_FREETYPE_EXCEPT(description,ret) {throw neko::Exception(description,ret,__FUNCTION__);}
# define NEKO_OPENGL_EXCEPT(description,en) {throw neko::Exception(description,en,__FUNCTION__);}
#endif

}