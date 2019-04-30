#pragma once
#include "neko_types.h"
#include "neko_platform.h"

namespace neko {

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
    void write( const string& str )
    {
      impl_.writeBlob( (void*)str.c_str(), (uint32_t)str.size() );
    }
  };

}