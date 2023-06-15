#pragma once
#include "neko_types.h"
#include "neko_platform.h"
#include "locator.h"
#include "filesystem.h"
#include "neatlywrappedsteamapi.h"

namespace neko {

  namespace utils {

    template <typename Container, typename Fn>
    Container map( const Container& in, Fn func )
    {
      Container out {};
      out.reserve( in.size() );
      std::transform( in.begin(), in.end(), std::back_inserter( out ), func );
      return out;
    }

    template <typename T>
    constexpr bool contains( const vector<T>& vec, T value )
    {
      return ( std::find( vec.begin(), vec.end(), value ) != vec.end() );
    }

    constexpr size_t alignToNextMultiple( size_t offset, size_t alignment )
    {
      return ( ( offset + alignment - 1 ) / alignment ) * alignment;
    }

    inline unicodeString uniFrom( const utf8String& u8str )
    {
      return unicodeString::fromUTF8( icu::StringPiece( u8str.c_str(), static_cast<int32_t>( u8str.length() ) ) );
    }

    inline utf8String uniToUtf8( const unicodeString& ustr ) noexcept
    {
      utf8String out;
      ustr.toUTF8String( out );
      return out;
    }

    //! Inline printf - buffer max is 16384 characters total
    utf8String ilprinf( const char* fmt, ... );

    inline wstring asciiToWideNaive( string_view str )
    {
      wstring out;
      out.reserve( str.length() );
      for ( size_t i = 0; i < str.length(); ++i )
        out[i] = (wchar_t)str[i];
      return move( out );
    }

    inline uint32_t parseUint32( const char* str )
    {
      return static_cast<uint32_t>( _atoi64( str ) );
    }

    inline uint64_t parseUint64( const utf8String& str )
    {
      return std::stoull( str );
    }

    inline void toLowercase( string& str )
    {
      std::transform( str.begin(), str.end(), str.begin(), ::tolower );
    }

    inline void toLowercase( wstring& str )
    {
      std::transform( str.begin(), str.end(), str.begin(), ::towlower );
    }

    inline utf8String render( steam::EResult steamResult )
    {
      static thread_local char buf[256];
      if ( steam::c_steamEResultDescriptionMap.find( steamResult ) != steam::c_steamEResultDescriptionMap.end() )
        sprintf_s<256>( buf, "Result %u - %s", steamResult, steam::c_steamEResultDescriptionMap.at( steamResult ).c_str() );
      else
        sprintf_s<256>( buf, "Result %u", steamResult );
      return buf;
    }

    inline utf8String render(
      steam::EAccountType type, steam::EUniverse universe, steam::AccountID_t id, unsigned int instance = 0 )
    {
      static thread_local char buf[37];
      switch ( type )
      {
        case steam::k_EAccountTypeAnonGameServer:
          sprintf_s<37>( buf, "[A:%u:%u:%u]", universe, id, instance );
          break;
        case steam::k_EAccountTypeGameServer:
          sprintf_s<37>( buf, "[G:%u:%u]", universe, id );
          break;
        case steam::k_EAccountTypeMultiseat:
          sprintf_s<37>( buf, "[M:%u:%u:%u]", universe, id, instance );
          break;
        case steam::k_EAccountTypePending:
          sprintf_s<37>( buf, "[P:%u:%u]", universe, id );
          break;
        case steam::k_EAccountTypeContentServer:
          sprintf_s<37>( buf, "[C:%u:%u]", universe, id );
          break;
        case steam::k_EAccountTypeClan:
          sprintf_s<37>( buf, "[g:%u:%u]", universe, id );
          break;
        case steam::k_EAccountTypeChat:
          switch ( instance & ~steam::k_EChatAccountInstanceMask )
          {
            case steam::k_EChatInstanceFlagClan:
              sprintf_s<37>( buf, "[c:%u:%u]", universe, id );
              break;
            case steam::k_EChatInstanceFlagLobby:
              sprintf_s<37>( buf, "[L:%u:%u]", universe, id );
              break;
            default:
              sprintf_s<37>( buf, "[T:%u:%u]", universe, id );
              break;
          }
          break;
        case steam::k_EAccountTypeInvalid:
          sprintf_s<37>( buf, "[I:%u:%u]", universe, id );
          break;
        case steam::k_EAccountTypeIndividual:
          sprintf_s<37>( buf, "[U:%u:%u]", universe, id );
          break;
        default:
          sprintf_s<37>( buf, "[i:%u:%u]", universe, id );
          break;
      }
      return utf8String( buf );
    }

    inline utf8String render( const steam::CSteamID& id )
    {
      return render( id.GetEAccountType(), id.GetEUniverse(), id.GetAccountID(), id.GetUnAccountInstance() );
    }

    inline utf8String beautifyDuration( chrono::seconds input_seconds, bool verbose = false )
    {
      using days = chrono::duration<int, std::ratio<86400>>;
      auto d = duration_cast<days>( input_seconds );
      input_seconds -= d;
      auto h = duration_cast<chrono::hours>( input_seconds );
      input_seconds -= h;
      auto m = duration_cast<chrono::minutes>( input_seconds );
      input_seconds -= m;
      auto s = duration_cast<chrono::seconds>( input_seconds );

      auto dc = d.count();
      auto hc = h.count();
      auto mc = m.count();
      auto sc = s.count();

      std::stringstream ss;
      if ( dc )
        ss << dc << ( verbose ? " days" : "d" );
      if ( dc || hc )
        ss << ' ' << hc << ( verbose ? " hours" : "h" );
      if ( dc || hc || mc )
        ss << ' ' << mc << ( verbose ? " minutes" : "m" );
      if ( dc || hc || mc || sc )
        ss << ' ' << sc << ( verbose ? " seconds" : "s" );

      return ss.str();
    }

    inline size_t utf8_surrogate_len( const char* character )
    {
      size_t result = 0;
      char test_char;

      if ( !character )
        return 0;

      test_char = character[0];

      if ( ( test_char & 0x80 ) == 0 )
        return 1;

      while ( test_char & 0x80 )
      {
        test_char <<= 1;
        result++;
      }

      return result;
    }

    inline size_t utf8_strlen( const char* string )
    {
      const char* ptr = string;
      size_t result = 0;

      while ( *ptr )
      {
        ptr += utf8_surrogate_len( ptr );
        result++;
      }

      return result;
    }

    inline uint32_t utf8_to_utf32( const char* character )
    {
      uint32_t result = -1;

      if ( !character )
      {
        return result;
      }

      if ( ( character[0] & 0x80 ) == 0x0 )
      {
        result = character[0];
      }

      if ( ( character[0] & 0xC0 ) == 0xC0 )
      {
        result = ( ( character[0] & 0x3F ) << 6 ) | ( character[1] & 0x3F );
      }

      if ( ( character[0] & 0xE0 ) == 0xE0 )
      {
        result = ( ( character[0] & 0x1F ) << ( 6 + 6 ) ) | ( ( character[1] & 0x3F ) << 6 ) | ( character[2] & 0x3F );
      }

      if ( ( character[0] & 0xF0 ) == 0xF0 )
      {
        result = ( ( character[0] & 0x0F ) << ( 6 + 6 + 6 ) ) | ( ( character[1] & 0x3F ) << ( 6 + 6 ) ) | ( ( character[2] & 0x3F ) << 6 ) | ( character[3] & 0x3F );
      }

      if ( ( character[0] & 0xF8 ) == 0xF8 )
      {
        result = ( ( character[0] & 0x07 ) << ( 6 + 6 + 6 + 6 ) ) | ( ( character[1] & 0x3F ) << ( 6 + 6 + 6 ) ) | ( ( character[2] & 0x3F ) << ( 6 + 6 ) ) | ( ( character[3] & 0x3F ) << 6 ) | ( character[4] & 0x3F );
      }

      return result;
    }

    inline wstring extractExtension( const utf8String& path )
    {
      auto wpath = platform::utf8ToWide( path );
      auto dot = wpath.find_last_of( L'.' );
      if ( dot == wpath.npos )
        return L"";
      auto ext = wpath.substr( dot + 1 );
      std::transform( ext.begin(), ext.end(), ext.begin(), ::towlower );
      return ext;
    }

    inline wstring extractFilename( const utf8String& path )
    {
      auto wpath = platform::utf8ToWide( path );
      auto slash = wpath.find_last_of( LR"(\/)" );
      if ( slash == wpath.npos )
        return L"";
      auto fname = wpath.substr( slash + 1 );
      return fname;
    }

    inline wstring extractFilepath( const utf8String& path )
    {
      auto wpath = platform::utf8ToWide( path );
      auto slash = wpath.find_last_of( LR"(\/)" );
      if ( slash == wpath.npos )
        return L"";
      auto fpath = wpath.substr( 0, slash );
      return fpath;
    }

    class DumbBuffer {
    private:
      Memory::Sector sector_;
      uint8_t* buffer_;
      size_t length_;
    public:
      DumbBuffer( Memory::Sector sector, const vector<uint8_t>& source ):
        sector_( sector ), buffer_( nullptr ), length_( 0 )
      {
        length_ = source.size();
        buffer_ = static_cast<uint8_t*>( Locator::memory().alloc( sector_, length_ ) );
        memcpy( buffer_, source.data(), length_ );
      }
      ~DumbBuffer()
      {
        if ( buffer_ )
          Locator::memory().free( sector_, buffer_ );
      }
      inline const Memory::Sector sector() const { return sector_; }
      inline const size_t length() const { return length_; }
      inline uint8_t* data() { return buffer_; }
    };

  }

  //! \class ScopedRWLock
  //! Automation for scoped acquisition and release of an RWLock.
  class ScopedRWLock {
  protected:
    platform::RWLock* lock_;
    bool exclusive_;
    bool locked_;
  public:
    //! Constructor.
    //! \param  lock      The lock to acquire.
    //! \param  exclusive (Optional) true to acquire in exclusive mode, false for shared.
    ScopedRWLock( platform::RWLock* lock, bool exclusive = true ):
      lock_( lock ), exclusive_( exclusive ), locked_( true )
    {
      exclusive_ ? lock_->lock() : lock_->lockShared();
    }
    //! Call directly if you want to unlock before object leaves scope.
    void unlock()
    {
      if ( locked_ )
        exclusive_ ? lock_->unlock() : lock_->unlockShared();
      locked_ = false;
    }
    ~ScopedRWLock()
    {
      unlock();
    }
  };

  //! \class TextFileWriter
  //! Simple text file writer that will append to existing files.
  class TextFileWriter {
  protected:
    platform::FileWriter impl_;
  public:
    TextFileWriter( const utf8String& filename ): TextFileWriter( platform::utf8ToWide( filename ) ) {}
    TextFileWriter( const wstring& filename ): impl_( filename, true )
    {
    }
    inline void write( const utf8String& str )
    {
      impl_.writeBlob( (void*)str.c_str(), (uint32_t)str.size() );
    }
  };

  //! \class TextFileReader
  //! Simple text file reader that will read the full text file to memory and skip an utf-8 BOM.
  class TextFileReader {
  protected:
    FileReaderPtr impl_;
  public:
    TextFileReader( FileReaderPtr reader ): impl_( reader )
    {
    }
    inline utf8String readFullAssumeUtf8()
    {
      auto str = impl_->readFullString();
      if ( str.length() < 3 )
        return str;

      const uint8_t utf8BOM[3] = { 0xEF, 0xBB, 0xBF };
      if ( memcmp( str.data(), utf8BOM, 3 ) == 0 )
        str.erase( 0, 3 );

      return str;
    }
  };

}