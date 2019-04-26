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
    virtual const string& getFullDescription() const;
    virtual const char* what() const throw() override;
  };

#if defined(NEKO_EXCEPT)
# error NEKO_EXCEPT* macro already defined!
#else
# define NEKO_EXCEPT(description) {throw neko::Exception(description,__FUNCTION__);}
#endif

}