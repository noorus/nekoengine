#pragma once
#include "neko_types.h"
#include "neko_platform.h"
#include "locator.h"

namespace neko {

  namespace functional {

    template <typename Container, typename Fn>
    Container map( const Container& in, Fn func )
    {
      Container out;
      out.reserve( in.size() );
      std::transform( in.begin(), in.end(), std::back_inserter( out ), func );
      return move( out );
    }

  }

  namespace utils {

    constexpr size_t alignToNextMultiple( size_t offset, size_t alignment )
    {
      return ( ( offset + alignment - 1 ) / alignment ) * alignment;
    }

    inline wstring asciiToWideNaive( string_view str )
    {
      wstring out;
      out.reserve( str.length() );
      for ( size_t i = 0; i < str.length(); ++i )
        out[i] = (wchar_t)str[i];
      return move( out );
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

    inline uint32_t utf8_to_utf32( const char * character )
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
  class ScopedRWLock: noncopyable {
  protected:
    platform::RWLock* lock_;
    bool exclusive_;
    bool locked_;

  public:
    //! Constructor.
    //! \param  lock      The lock to acquire.
    //! \param  exclusive (Optional) true to acquire in exclusive mode, false for shared.
    ScopedRWLock( platform::RWLock* lock, bool exclusive = true )
      : lock_( lock ), exclusive_( exclusive ), locked_( true )
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
    TextFileWriter( const string& filename ):
      impl_( filename, true )
    {
    }
    inline void write( const string& str )
    {
      impl_.writeBlob( (void*)str.c_str(), (uint32_t)str.size() );
    }
  };

  //! \class TextFileReader
  //! Simple text file reader that will read the full text file to memory and skip an utf-8 BOM.
  class TextFileReader {
  protected:
    platform::FileReader impl_;
  public:
    TextFileReader( const utf8String& filename ):
      impl_( filename )
    {
    }
    inline utf8String readFullAssumeUtf8()
    {
      auto str = impl_.readFullString();
      if ( str.length() < 3 )
        return str;

      const uint8_t utf8BOM[3] = { 0xEF, 0xBB, 0xBF };
      if ( memcmp( str.data(), utf8BOM, 3 ) == 0 )
        str.erase( 0, 3 );

      return str;
    }
  };

}