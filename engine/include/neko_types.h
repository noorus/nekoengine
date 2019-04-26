#pragma once

namespace neko {

  using std::uint8_t;  //!< 8-bit unsigned integer type.
  using std::uint16_t; //!< 16-bit unsigned integer type.
  using std::uint32_t; //!< 32-bit unsigned integer type.
  using std::uint64_t; //!< 64-bit unsigned integer type.

  using std::vector; //!< Local vector type.
  using std::list; //!< Local list type.

  typedef std::string string; //!< Local string type.
  typedef std::wstring wstring; //!< Local wide-string type.
  typedef std::stringstream stringstream; //!< Local string stream type.
  typedef std::wstringstream wstringstream; //!< Locale wide-string stream type.

  typedef double GameTime; //!< Our generic type for representing time.
  typedef float Real;

  typedef boost::char_separator<wchar_t> CharSeparator;
  typedef boost::tokenizer<CharSeparator, string::const_iterator, wstring> StringTokenizer;
  typedef std::vector<uint8_t> ByteVector;
  typedef std::vector<wstring> StringVector;
  typedef std::vector<void*> VoidVector;
  typedef std::list<wstring> StringList;
  typedef std::queue<wstring> StringQueue;

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