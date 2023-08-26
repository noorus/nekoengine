#pragma once

#include "neko_config.h"

#include <exception>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <regex>
#include <stack>
#include <cstdint>
#include <algorithm>
#include <random>
#include <filesystem>
#include <queue>
#include <unordered_map>
#include <utility>
#include <set>
#include <variant>
#include <any>
#include <optional>
#include <atomic>
#include <numbers>
#include <span>
#include <nlohmann/json.hpp>

#undef min
#undef max

#include <entt.hpp>

#pragma warning( push )
#pragma warning( disable : 28251 )
#include <moodycamel/readerwriterqueue.h>
#pragma warning( pop )

// GLM
#define GLM_FORCE_XYZW_ONLY // Hide rgba, stpq unions from vector types (avoiding bloat when examined in the debugger)
#define GLM_FORCE_SIZE_T_LENGTH // Make size() on GLM types return size_t instead of int (for STL compatibility)
#define GLM_FORCE_UNRESTRICTED_GENTYPE // Allow operation input types that strict GLSL wouldn't (such as int)
#define GLM_ENABLE_EXPERIMENTAL // Enable experimental new features
#if !defined(NEKO_VERBOSE_COMPILE) && !defined(_DEBUG)
# define GLM_FORCE_SILENT_WARNINGS
#endif
#ifndef _DEBUG
# define GLM_FORCE_INLINE // Force inlining in release build
#endif

// clang-format off
#define GLM_VEC2_EXTRA_STUFF \
  static constexpr vec<2, T, Q> unit_x() { return { static_cast<T>( 1.0 ), static_cast<T>( 0.0 ) }; } \
  static constexpr vec<2, T, Q> unit_y() { return { static_cast<T>( 0.0 ), static_cast<T>( 1.0 ) }; }
#define GLM_VEC3_EXTRA_STUFF \
  static constexpr vec<3, T, Q> unit_x() { return { static_cast<T>( 1.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ) }; } \
  static constexpr vec<3, T, Q> unit_y() { return { static_cast<T>( 0.0 ), static_cast<T>( 1.0 ), static_cast<T>( 0.0 ) }; } \
  static constexpr vec<3, T, Q> unit_z() { return { static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 1.0 ) }; }
#define GLM_VEC4_EXTRA_STUFF \
  static constexpr vec<4, T, Q> unit_x() { return { static_cast<T>( 1.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ) }; } \
  static constexpr vec<4, T, Q> unit_y() { return { static_cast<T>( 0.0 ), static_cast<T>( 1.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ) }; } \
  static constexpr vec<4, T, Q> unit_z() { return { static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 1.0 ), static_cast<T>( 0.0 ) }; } \
  static constexpr vec<4, T, Q> unit_w() { return { static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 1.0 ) }; }
#define GLM_MAT3_EXTRA_STUFF \
  static constexpr mat<3, 3, T, Q> zero() { return { static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), \
                                                     static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), \
                                                     static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ) }; }
#define GLM_MAT4_EXTRA_STUFF \
  static constexpr mat<4, 4, T, Q> zero() { return { static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), \
                                                     static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), \
                                                     static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), \
                                                     static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ) }; }
#define GLM_QUAT_EXTRA_STUFF \
  static constexpr qua<T, Q> identity() { return qua<T, Q>( static_cast<T>( 1.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ), static_cast<T>( 0.0 ) ); }

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#ifndef NEKO_NO_ICU
# include <unicode/ustring.h>
# include <unicode/unistr.h>
#endif

namespace neko {

  using std::uint8_t;
  using std::uint16_t;
  using std::uint32_t;
  using std::uint64_t;

  using std::string;
  using std::wstring;
  using std::stringstream;
  using std::wstringstream;
  using std::string_view;

  using std::array;
  using std::vector;
  using std::list;
  using std::set;
  using std::map;
  using std::priority_queue;
  using std::unordered_map;
  using std::make_shared;
  using std::shared_ptr;
  using std::make_unique;
  using std::unique_ptr;

  using std::atomic;
  using moodycamel::ReaderWriterQueue;
  using std::span;

  namespace chrono {
    using namespace std::chrono;
  }

  using std::move;
  using std::pair;
  using std::make_pair;
  using std::tuple;
  using std::make_tuple;
  using std::optional;
  using std::make_optional;
  using std::initializer_list;

  using utf8String = std::string; //!< Be careful. This is just a normal string but aliased to call to attention that it contains utf-8.

#ifndef NEKO_NO_ICU
  using unicodeString = icu::UnicodeString;
  using unicodePiece = icu::StringPiece;
#endif

  using std::make_shared;
  using std::make_unique;
  using std::enable_shared_from_this;
  using std::move;
  using std::string_view;

  using std::numeric_limits;

  using std::ignore;

  enum LogSource : unsigned long
  {
    srcError = 0,
    srcEngine,
    srcGfx,
    srcSound,
    srcLoader,
    srcScripting,
    srcInput,
    srcGame,
    srcGUI,
    srcMessaging,
    srcSteam
  };

  using Entity = entt::entity;

#ifdef NEKO_MATH_DOUBLE
  using Real = double;
  using vec2 = glm::dvec2;
  using vec3 = glm::dvec3;
  using vec4 = glm::dvec4;
  using mat2 = glm::dmat2x2;
  using mat3 = glm::dmat3x3;
  using mat4 = glm::dmat4x4;
  using mat34 = glm::dmat3x4;
  using quaternion = glm::dquat;
#else
  using Real = float;
  using vec2 = glm::fvec2;
  using vec3 = glm::fvec3;
  using vec4 = glm::fvec4;
  using mat2 = glm::fmat2x2;
  using mat3 = glm::fmat3x3;
  using mat4 = glm::fmat4x4;
  using mat34 = glm::fmat3x4;
  using quaternion = glm::fquat;
#endif

  namespace numbers {
    constexpr Real zero = static_cast<Real>( 0.0 );
    constexpr Real half = static_cast<Real>( 0.5 );
    constexpr Real one = static_cast<Real>( 1.0 );
    constexpr Real two = static_cast<Real>( 2.0 );
    constexpr Real three = static_cast<Real>( 3.0 );
    constexpr Real six = static_cast<Real>( 6.0 );
    constexpr Real ten = static_cast<Real>( 10.0 );
    constexpr Real fifteen = static_cast<Real>( 15.0 );
    constexpr Real minus_one = static_cast<Real>( -1.0 );
    constexpr Real pi = std::numbers::pi_v<Real>;
    constexpr Real e = std::numbers::e_v<Real>;
    constexpr Real ln2 = std::numbers::ln2_v<Real>;
    constexpr Real ln10 = std::numbers::ln10_v<Real>;
    constexpr Real sqrt2 = std::numbers::sqrt2_v<Real>;
    constexpr Real g = static_cast<Real>( 9.80665 ); // standard gravity m/s
  }

  using quat = quaternion;

  using Radians = Real;
  using Degrees = Real;

  constexpr vec4 rgba( int r, int g, int b, double a )
  {
    return {
      static_cast<Real>( r ) / 255.0f,
      static_cast<Real>( g ) / 255.0f,
      static_cast<Real>( b ) / 255.0f,
      static_cast<Real>( a ) };
  }

  using GameTime = double;

  using StringVector = vector<utf8String>;

  using vec2i = glm::i32vec2;
  using vec3i = glm::i32vec3;
  using vec4i = glm::i32vec4;

  using vec2u = glm::u32vec2;
  using vec3u = glm::u32vec3;
  using vec4u = glm::u32vec4;

  using vec2f = glm::fvec2;
  using vec3f = glm::fvec3;
  using vec4f = glm::fvec4;

  using vec2d = glm::dvec2;
  using vec3d = glm::dvec3;
  using vec4d = glm::dvec4;

  using json = nlohmann::json;

  struct Ray
  {
    vec3 origin { 0.0f };
    vec3 direction { 0.0f };
  };

  class nocopy {
  private:
    nocopy( const nocopy& ) = delete;
    nocopy& operator=( const nocopy& ) = delete;
  protected:
    constexpr nocopy() = default;
    ~nocopy() = default;
  };

  struct size2i {
    int64_t w;
    int64_t h;
    explicit size2i(): w( 0 ), h( 0 ) {}
    inline size2i( const size2i& other ): w( other.w ), h( other.h ) {}
    inline size2i( const int64_t width, const int64_t height ): w( width ), h( height ) {}
    inline size2i( const vec2i& v ): w( v.x ), h( v.y ) {}
    inline size2i& operator *= ( const int64_t scalar )
    {
      w *= scalar;
      h *= scalar;
      return *this;
    }
    inline size2i& operator /= ( const int64_t scalar )
    {
      w /= scalar;
      h /= scalar;
      return *this;
    }
    inline size2i& operator += ( const int64_t scalar )
    {
      w += scalar;
      h += scalar;
      return *this;
    }
    inline size2i& operator -= ( const int64_t scalar )
    {
      w -= scalar;
      h -= scalar;
      return *this;
    }
    inline size2i& operator += ( const vec2i& rhs )
    {
      w += rhs.x;
      h += rhs.y;
      return *this;
    }
    inline size2i& operator -= ( const vec2i& rhs )
    {
      w -= rhs.x;
      h -= rhs.y;
      return *this;
    }
    inline bool operator == ( const size2i& rhs ) const
    {
      return ( w == rhs.w && h == rhs.h );
    }
    inline bool operator != ( const size2i& rhs ) const
    {
      return !( *this == rhs );
    }
    inline bool operator < ( const size2i& rhs ) const
    {
      return ( w < rhs.w && h < rhs.h );
    }
    inline bool operator > ( const size2i& rhs ) const
    {
      return ( w > rhs.w && h > rhs.h );
    }
    inline bool operator <= ( const size2i& rhs ) const
    {
      return ( w <= rhs.w && h <= rhs.h );
    }
    inline bool operator >= ( const size2i& rhs ) const
    {
      return ( w >= rhs.w && h >= rhs.h );
    }
    inline size2i& operator = ( const int64_t scalar )
    {
      w = scalar;
      h = scalar;
      return *this;
    }
#ifdef NEKO_PLATFORM_WINDOWS
    inline size2i( const SIZE& sz ): w( sz.cx ), h( sz.cy ) {}
    inline size2i( const RECT& rect ): w( rect.right - rect.left ), h( rect.bottom - rect.top ) {}
#endif
  };

  struct Image {
    size2i size_;
    vector<uint8_t> buffer_;
    Image(): size_() {}
  };

  enum CharacterConstants {
    TAB = 9,
    LF = 10,
    CR = 13,
    SPACE = 32,
    QUOTE = 34,
    COLON = 58,
    SEMICOLON = 59,
    BACKSLASH = 92
  };

  struct DateTime {
    uint16_t year = 0;    //!< Year
    uint16_t month = 0;   //!< Month
    uint16_t day = 0;     //!< Day
    uint16_t hour = 0;    //!< Hour
    uint16_t minute = 0;  //!< Minute
    uint16_t second = 0;  //!< Second
  };

}