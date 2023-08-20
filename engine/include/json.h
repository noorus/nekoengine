#pragma once

#include "pch.h"
#include "neko_types.h"
#include "neko_exception.h"

namespace neko {

  namespace njson {

    inline vec2 readVec2( const json& obj )
    {
      vec2 ret = { 0.0f, 0.0f };
      if ( obj.is_object() )
      {
        if ( obj.contains( "x" ) )
          ret.x = obj["x"].get<float>();
        if ( obj.contains( "y" ) )
          ret.y = obj["y"].get<float>();
      }
      else if ( obj.is_array() && obj.size() == 2 )
      {
        ret.x = obj[0].get<float>();
        ret.y = obj[1].get<float>();
      }
      else
        NEKO_EXCEPT( "Expected JSON vec2 is not an object or 2-element array" );
      return ret;
    }

    inline vec2i readVec2i( const json& obj )
    {
      auto v = readVec2( obj );
      return vec2i( math::iround( v.x ), math::iround( v.y ) );
    }

    template <typename T>
    inline vector<T> readVector( const json& arr )
    {
      vector<T> ret;
      if ( !arr.is_array() )
        NEKO_EXCEPT( "Expected JSON array is not an array" );
      for ( const auto& entry : arr )
        ret.push_back( entry.get<T>() );
      return ret;
    }

  }

}