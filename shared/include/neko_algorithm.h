#pragma once
#include "neko_types.h"
#include "neko_platform.h"

namespace neko {

  namespace algorithm {

    const char c_defaultTrimCharacters[] = "\t\n\v\f\r ";

    template <typename T>
    T& ltrim( T& str, const T& chars = c_defaultTrimCharacters )
    {
      str.erase( 0, str.find_first_not_of( chars ) );
      return str;
    }

    template <typename T>
    T& rtrim( T& str, const T& chars = c_defaultTrimCharacters )
    {
      str.erase( str.find_last_not_of( chars ) + 1 );
      return str;
    }

    template <typename T>
    T& trim( T& str, const T& chars = c_defaultTrimCharacters )
    {
      return ltrim( rtrim( str, chars ), chars );
    }

    template <typename T>
    T trim_copy( T str, const T& chars = c_defaultTrimCharacters )
    {
      return ltrim( rtrim( str, chars ), chars );
    }

    template <typename T>
    bool iequals( const T& a, const T& b )
    {
      auto caseicomp = []( const char& x, const char& y )
      {
        return ( x == y || std::toupper( x ) == std::toupper( y ) );
      };
      return ( ( a.size() == b.size() ) && ( std::equal( a.begin(), a.end(), b.begin(), caseicomp ) ) );
    }

  }

}