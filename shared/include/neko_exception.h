#pragma once
#include "neko_types.h"

namespace neko {

  class Exception: public std::exception {
  public:
    enum Type {
      Type_Internal,
      Type_OpenGL,
      Type_WinAPI
    };
  protected:
    string description_;
    string source_;
    mutable string fullDescription_;
  public:
    Exception( const string& description );
    Exception( const string& description, const string& source, Type type );
#ifndef NEKO_SIDELIBRARY_BUILD
    Exception( const string& description, gl::GLenum gle, const string& source );
#endif
    virtual const string& getFullDescription() const;
    virtual const char* what() const noexcept override;
  };

#if defined(NEKO_EXCEPT)
# error NEKO_EXCEPT* macro already defined!
#else
# define NEKO_EXCEPT(description) {throw neko::Exception(description,__FUNCTION__,neko::Exception::Type_Internal);}
# ifndef NEKO_SIDELIBRARY_BUILD
#  define NEKO_OPENGL_EXCEPT(description,en) {throw neko::Exception(description,en,__FUNCTION__);}
# endif
# define NEKO_WINAPI_EXCEPT(description) {throw neko::Exception(description,__FUNCTION__,neko::Exception::Type_WinAPI);}
#endif

}