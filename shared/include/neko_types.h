#pragma once

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

  using std::make_shared;
  using std::make_unique;

  using boost::noncopyable;

  using Real = float;
  using StringVector = vector<string>;

  using glm::vec2;
  using glm::vec3;
  // typedef b2Mat22 mat2;
  // typedef b2Mat33 mat3;
  using glm::mat4;
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

}