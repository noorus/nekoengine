#pragma once
#include "neko_types.h"
#include "neko_platform.h"

namespace neko {

  namespace utils {

    constexpr size_t alignToNextMultiple( size_t offset, size_t alignment )
    {
      return ( ( offset + alignment - 1 ) / alignment ) * alignment;
    }

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