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
#include <boost/math/constants/constants.hpp>
#include <glm/glm.hpp>

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
  using std::vector;
  using std::list;
  using std::set;
  using std::map;
  using std::pair;
  using std::priority_queue;
  using std::unordered_map;
  using std::shared_ptr;
  using std::unique_ptr;
  using std::move;

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

  using boost::noncopyable;

#ifdef NEKO_MATH_DOUBLE
  using Real = double;
# define NEKO_ZERO 0.0
# define NEKO_HALF 0.5
# define NEKO_ONE 1.0
  namespace math {
    using namespace boost::math::double_constants;
  }
  using vec2 = glm::dvec2;
  using vec3 = glm::dvec3;
  using vec4 = glm::dvec4;
  using mat2 = glm::dmat2x2;
  using mat3 = glm::dmat3x3;
  using mat4 = glm::dmat4x4;
  using quaternion = glm::dquat;
#else
  using Real = float;
# define NEKO_ZERO 0.0f
# define NEKO_HALF 0.5f
# define NEKO_ONE 1.0f
  namespace math {
    using namespace boost::math::float_constants;
  }
  using vec2 = glm::fvec2;
  using vec3 = glm::fvec3;
  using vec4 = glm::fvec4;
  using mat2 = glm::fmat2x2;
  using mat3 = glm::fmat3x3;
  using mat4 = glm::fmat4x4;
  using quaternion = glm::fquat;
#endif

  using GameTime = double;

  using StringVector = vector<utf8String>;

  using vec2i = glm::i64vec2;
  using vec3i = glm::i64vec3;
  using vec4i = glm::i64vec4;

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
    uint16_t year;    //!< Year
    uint16_t month;   //!< Month
    uint16_t day;     //!< Day
    uint16_t hour;    //!< Hour
    uint16_t minute;  //!< Minute
    uint16_t second;  //!< Second
  };

}